#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
using namespace std;

#include "Protocols.h"

void MeasureOfflinePart1(string dataset, int k, int D, int q, int h)
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

	vector<vector<int8_t>> userItemMatrix = Protocols::ReadUserItemMatrix("ratings-distict-" + dataset + ".dat");

	int N = userItemMatrix.size(); // users
	int M = userItemMatrix[0].size(); // items

#pragma endregion


#pragma region Computing the similarity matrix and the shares(Protocol 1 + 2)

	cout << "MeasureOfflinePart1" << endl;

	Protocols::SimulateSingleVendorWorkInComputingSimilarityMatrix(userItemMatrix, D, fileName);

	vector<vector<uint16_t>> someRShare;
	vector<vector<uint16_t>> someXiRShare;
	vector<vector<uint16_t>> someSqRShare;

	someRShare = Protocols::CreateRandomMatrixShare(N, M);
	someXiRShare = Protocols::CreateRandomMatrixShare(N, M);
	someSqRShare = Protocols::CreateRandomMatrixShare(N, M);

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
	int h = 20; // num of most recomended items to take
	string dataset = "1M";
	MeasureOfflinePart1(dataset, 1, 3, q, h);
}