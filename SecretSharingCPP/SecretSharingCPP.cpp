#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <execution>
#include "Protocols.h"
#include <random>

using namespace std;

void StartMeasurement(string dataset, int k, int D, int q, int h)
{
#pragma region Init

	cout << "Dataset - " << dataset << ", k=" << k << ", D=" << D << " Started" << endl;

	string directoryName = "k-" + to_string(k) + ", D-" + to_string(D) + ", Dataset-" + dataset + "/";
	string fileName = directoryName + "MeasureOfflinePart1.txt";
	filesystem::create_directory(directoryName);
	filesystem::path path(fileName);
	ofstream dataFile(path, ios_base::app);
	dataFile << "Database - " + dataset + ", k=" + to_string(k) + ", D=" + to_string(D) + "\n";
	dataFile.close();

#pragma endregion


#pragma region Settings

	vector<vector<int8_t>> userItemMatrix = Protocols::ReadUserItemMatrix(dataset + ".dat");

	int N = userItemMatrix.size(); // users
	int M = userItemMatrix[0].size(); // items

#pragma endregion


#pragma region Computing the similarity matrix and the shares(Protocol 1 + 2)

	cout << "MeasureOfflinePart1" << endl;

	Protocols::SimulateSingleVendorWorkInComputingSimilarityMatrix(userItemMatrix, D, fileName);

	userItemMatrix.clear();
	userItemMatrix.shrink_to_fit();

	vector<vector<uint16_t>> someRShare;
	vector<vector<uint16_t>> someXiRShare;
	vector<vector<uint16_t>> someSqRShare;

	someRShare = Protocols::CreateRandomMatrixShare(10000, 1000);
	someXiRShare = Protocols::CreateRandomMatrixShare(10000, 1000);
	someSqRShare = Protocols::CreateRandomMatrixShare(10000, 1000);

	Protocols::SimulateSingleMediatorWorkInComputingSimilarityMatrix(N, M, someRShare, someXiRShare, someSqRShare, D, fileName);

	someRShare.clear();
	someRShare.shrink_to_fit();
	someXiRShare.clear();
	someXiRShare.shrink_to_fit();
	someSqRShare.clear();
	someSqRShare.shrink_to_fit();

#pragma endregion


#pragma region MeasureOfflinePart2

	cout << "MeasureOfflinePart2" << endl;

	fileName = directoryName + "MeasureOfflinePart2.txt";

	Protocols::SimulateSingleMediatorWorkInComputingOfflinePart2(D, N, M, q, fileName);

#pragma endregion


#pragma region MeasureOnlinePredictRating

	cout << "MeasureOnlinePredictRating" << endl;

	fileName = directoryName + "MeasureOnlinePredictRating.txt";

	Protocols::SimulateSingleMediatorWorkInOnlinePredictRating(M, D, fileName);

#pragma endregion


#pragma region MeasureOnlinePredictRanking

	cout << "MeasureOnlinePredictRanking" << endl;

	fileName = directoryName + "MeasureOnlinePredictRanking.txt";

	Protocols::SimulateSingleMediatorWorkInOnlinePredictRanking(M, q, fileName);

	Protocols::SimulateSingleVendorWorkInOnlinePredictRanking(M, D, h, fileName);

#pragma endregion
}

void main(void)
{
	srand((unsigned)time(NULL));
	int q = 80; // num of similar items
	int h = 10; // num of most recomended items to take
	const char* datasets[] = { "100K", "1M", "10M", "20M", "WM0","WM1", "WM2", "WM3", "WM4", "WN0", "WN1", "WN2", "WN3", "WN4" };

	for (string dataset : datasets)
	{
		StartMeasurement(dataset, 1, 3, q, h);
		StartMeasurement(dataset, 1, 5, q, h);
		StartMeasurement(dataset, 1, 7, q, h);
		StartMeasurement(dataset, 1, 9, q, h);
		cout << "Done: " + dataset << endl;
	}
}