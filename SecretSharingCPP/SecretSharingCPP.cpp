#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
using namespace std;

#include "Protocols.h"

void MeasureOfflinePart1(string dataset, int k, int D, int q, int h)
{
	cout << "Dataset - " << dataset << ", k=" << k << ", D=" << D << " Started" << endl;
	string directoryName = "k-" + to_string(k) + ", D-" + to_string(D) + ", Dataset-" + dataset + "/";
	string fileName = directoryName + "MeasureOfflinePart1.txt";


	filesystem::create_directory(directoryName);
	filesystem::path path(fileName);
	ofstream dataFile(path, ios_base::app);
	dataFile << "Database - " + dataset + ", k=" + to_string(k) + ", D=" + to_string(D) + "\n";
	dataFile.close();

	vector<vector<int8_t>> userItemMatrix = Protocols::ReadUserItemMatrix("ratings-distict-" + dataset + ".dat");

	int N = userItemMatrix.size(); // users
	int M = userItemMatrix[0].size(); // items

	cout << "MeasureOfflinePart1";


	Protocols::SimulateSingleVendorWorkInComputingSimilarityMatrix(userItemMatrix, D, fileName);

	vector<vector<uint16_t>> someRShare;
	vector<vector<uint16_t>> someXiRShare;
	vector<vector<uint16_t>> someSqRShare;

	someRShare = Protocols::CreateRandomMatrixShare(N, M);
	someXiRShare = Protocols::CreateRandomMatrixShare(N, M);
	someSqRShare = Protocols::CreateRandomMatrixShare(N, M);

	Protocols::SimulateSingleMediatorWorkInComputingSimilarityMatrix(N, M, someRShare, someXiRShare, someSqRShare, D, fileName);
}

void main(void)
{
	int q = 80; // num of similar items
	int h = 20; // num of most recomended items to take
	string dataset = "100K";
	MeasureOfflinePart1(dataset, 1, 3, q, h);
}