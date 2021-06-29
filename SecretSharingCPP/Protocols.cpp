#include <string>
#include <chrono>
#include <stdint.h>
#include <math.h>
#include <fstream>
#include <algorithm>
#include "Protocols.h"
#include <iostream>
#include <filesystem>

using namespace std;
using namespace std::chrono;

const uint64_t PRIME = 2147483647;

struct UserRating { int UserId; int ItemId; int Rating; };

uint64_t ReconstructShamirSecret(vector<uint64_t>& coordinates)
{
	uint64_t secret = 0;
	if (coordinates.size() == 3)
	{
		secret = ((3 * (coordinates[0] - coordinates[1]) + coordinates[2]) % PRIME);
	}
	else if (coordinates.size() == 5)
	{
		secret = ((5 * (coordinates[0] - coordinates[3])) - (10 * (coordinates[1] - coordinates[2])) + coordinates[4]) % PRIME;
	}
	else if (coordinates.size() == 7)
	{
		secret = ((7 * (coordinates[0] - coordinates[5])) + (21 * (coordinates[4] - coordinates[1])) + (35 * (coordinates[2] - coordinates[3])) + coordinates[6]) % PRIME;
	}
	else if (coordinates.size() == 9)
	{
		secret = ((9 * (coordinates[0] - coordinates[7])) + (36 * (coordinates[6] - coordinates[1])) + (84 * (coordinates[2] - coordinates[5])) + (126 * (coordinates[4] - coordinates[3])) + coordinates[8]) % PRIME;
	}

	if (secret < 0)
	{
		secret += PRIME;
	}

	return secret;
}

uint64_t ScalarProductVectors(vector<uint16_t>& vector1, vector<uint16_t>& vector2)
{
	int length = vector1.size();
	uint64_t sum = 0;
	for (int i = 0; i < length; i++)
	{
		sum += vector1[i] * vector2[i];
	}

	return sum % PRIME;
}

vector<vector<int8_t>> CalcSq(vector<vector<int8_t>>& matrix)
{
	int N = matrix.size();
	int M = matrix[0].size();

	vector<vector<int8_t> > sq(N, vector<int8_t>(M));

	for (int i = 0; i < matrix.size(); ++i)
	{
		for (int j = 0; j < matrix[0].size(); ++j)
		{
			if (-1 != matrix[i][j])
			{
				sq[i][j] = pow(matrix[i][j], 2);
			}
		}
	}
	return sq;
}

vector<vector<int8_t>> CalcXi(vector<vector<int8_t>>& matrix)
{
	int N = matrix.size();
	int M = matrix[0].size();

	vector<vector<int8_t> > xi(N, vector<int8_t>(M));

	for (int i = 0; i < matrix.size(); ++i)
	{
		for (int j = 0; j < matrix[0].size(); ++j)
		{
			if (-1 != matrix[i][j])
			{
				xi[i][j] = (matrix[i][j] == 0 ? 0 : 1);
			}
		}
	}
	return xi;
}

void ShamirSecretSharingNoStoring(uint32_t scalar, int numOfShares)
{
	int maxRangeForRandom = 65535;
	int minRangeForRandom = 2;
	int range = maxRangeForRandom - minRangeForRandom + 1;

	if (numOfShares == 3)
	{
		uint32_t a = rand() % range + minRangeForRandom;
		uint32_t lastY = scalar;

		for (int i = 0; i < 3; i++)
		{
			auto y = lastY + a;
			lastY = y;
		}
	}

	else if (numOfShares == 5)
	{
		uint32_t a = rand() % range + minRangeForRandom;
		uint32_t b = rand() % range + minRangeForRandom;

		uint32_t B = a + b;
		uint32_t B2 = b + b;
		uint32_t B3 = B + B2;
		uint32_t B5 = B3 + B2;
		uint32_t B7 = B5 + B2;
		uint32_t B9 = B7 + B2;
		uint32_t lastY = 0;

		for (int i = 0; i < 5; i++)
		{
			uint32_t y = 0;
			switch (i)
			{
			case 0:
				y = scalar + B;
				break;
			case 1:
				y = lastY + B3;
				break;
			case 2:
				y = lastY + B5;
				break;
			case 3:
				y = lastY + B7;
				break;
			case 4:
				y = lastY + B9;
				break;
			}
			lastY = y;
		}
	}

	else if (numOfShares == 7)
	{
		uint32_t a = rand() % range + minRangeForRandom;
		uint32_t b = rand() % range + minRangeForRandom;
		uint32_t c = rand() % range + minRangeForRandom;

		uint32_t B = a + b + c;
		uint32_t B1 = 6 * c;
		uint32_t B2 = 2 * b;
		uint32_t B3 = B + B2 + B1;
		uint32_t B5 = B3 + B2 + 2 * B1;
		uint32_t B7 = B5 + B2 + 3 * B1;
		uint32_t B9 = B7 + B2 + 4 * B1;
		uint32_t B11 = B9 + B2 + 5 * B1;
		uint32_t B13 = B11 + B2 + 6 * B1;

		uint32_t lastY = 0;

		for (int i = 0; i < 7; i++)
		{
			uint32_t y = 0;
			switch (i)
			{
			case 0:
				y = scalar + B;
				break;
			case 1:
				y = lastY + B3;
				break;
			case 2:
				y = lastY + B5;
				break;
			case 3:
				y = lastY + B7;
				break;
			case 4:
				y = lastY + B9;
				break;
			case 5:
				y = lastY + B11;
				break;
			case 6:
				y = lastY + B13;
				break;
			}
			lastY = y;
		}
	}

	else if (numOfShares == 9)
	{
		uint32_t a = rand() % range + minRangeForRandom;
		uint32_t b = rand() % range + minRangeForRandom;
		uint32_t c = rand() % range + minRangeForRandom;
		uint32_t d = rand() % range + minRangeForRandom;

		uint32_t B = a + b + c + d;
		uint32_t B1 = 2 * d;
		uint32_t B2 = 6 * c;
		uint32_t B3 = 2 * b;
		uint32_t B5 = B + B3 + B2 + 7 * B1;
		uint32_t B7 = B5 + B3 + 2 * B2 + 25 * B1;
		uint32_t B9 = B7 + B3 + 3 * B2 + 55 * B1;
		uint32_t B11 = B9 + B3 + 4 * B2 + 97 * B1;
		uint32_t B13 = B11 + B3 + 5 * B2 + 151 * B1;
		uint32_t B15 = B13 + B3 + 6 * B2 + 217 * B1;
		uint32_t B17 = B15 + B3 + 7 * B2 + 295 * B1;
		uint32_t B19 = B17 + B3 + 8 * B2 + 385 * B1;

		uint32_t lastY = 0;

		for (int i = 0; i < 9; i++)
		{
			uint32_t y = 0;
			switch (i)
			{
			case 0:
				y = scalar + B;
				break;
			case 1:
				y = lastY + B5;
				break;
			case 2:
				y = lastY + B7;
				break;
			case 3:
				y = lastY + B9;
				break;
			case 4:
				y = lastY + B11;
				break;
			case 5:
				y = lastY + B13;
				break;
			case 6:
				y = lastY + B15;
				break;
			case 7:
				y = lastY + B17;
				break;
			case 8:
				y = lastY + B19;
				break;
			}
			lastY = y;
		}
	}
}

void AddShare(vector<vector<uint16_t>>& matrix, vector<vector<uint16_t>>& share)
{
	int N = matrix.size();
	int M = matrix[0].size();

	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < M; j++)
		{
			matrix[i][j] += share[i][j];
		}
	}
}

vector<vector<uint16_t>> CreateRandomShares(int n, int numOfShares)
{
	vector<vector<uint16_t>> shares;
	int maxRangeForRandom = 65535;
	int minRangeForRandom = 2;
	int range = maxRangeForRandom - minRangeForRandom + 1;

	for (int i = 0; i < numOfShares; i++)
	{
		vector<uint16_t> vector(n, 0);
		for (int j = 0; j < n; j++)
		{
			vector[j] = rand() % range + minRangeForRandom;
		}
		shares.push_back(vector);
	}
	return shares;
}

vector<vector<int8_t>> Protocols::ReadUserItemMatrix(string path)
{
	vector<UserRating> ratings;
	ifstream infile(path);

	int userId, itemId, rating;
	int N = 0, M = 0;
	while (infile >> userId >> itemId >> rating)
	{
		UserRating userRating = { userId, itemId, rating };
		ratings.push_back(userRating);
		if (userRating.UserId > N)
			N = userRating.UserId;
		if (userRating.ItemId > M)
			M = userRating.ItemId;
	}

	infile.close();

	vector<vector<int8_t> > userItemMatrix(N, vector<int8_t>(M));
	for (UserRating rating : ratings) {
		userItemMatrix[rating.UserId - 1][rating.ItemId - 1] = rating.Rating;
	}

	return userItemMatrix;
}

vector<vector<uint16_t>> Protocols::CreateRandomMatrixShare(int n, int m)
{
	vector<vector<uint16_t>> matrix(n, vector<uint16_t>(m));
	int maxRangeForRandom = 65535;

	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < m; j++)
		{
			matrix[i][j] = rand() % (maxRangeForRandom + 1);
		}
	}
	return matrix;
}

void Protocols::SimulateSingleVendorWorkInComputingSimilarityMatrix(vector<vector<int8_t>>& Rk, uint32_t numOfShares, string fileName)
{
	auto start = high_resolution_clock::now();

	CalcSq(Rk);
	CalcXi(Rk);

	auto stop = high_resolution_clock::now();
	auto ATime = duration_cast<milliseconds>(stop - start);

	filesystem::path path(fileName);
	ofstream dataFile(path, ios_base::app);
	dataFile << "A time is: " + to_string(ATime.count()) + " milliseconds\n";

	auto FDTime = milliseconds::zero();
	for (int i = 0; i < 1000; i++)
	{
		uint32_t scalar = rand() % 2147483648;
		start = high_resolution_clock::now();
		ShamirSecretSharingNoStoring(scalar, numOfShares);
		stop = high_resolution_clock::now();
		FDTime += duration_cast<milliseconds>(stop - start);
	}

	FDTime = FDTime * 3 * Rk.size() * Rk[0].size() / 1000;
	dataFile << "F(" + to_string(numOfShares) + ")*3*N*M: " + to_string(FDTime.count()) + " milliseconds\n";

	auto totalVendorTime = ATime + FDTime;
	dataFile << "Total time for a single vendor computing the similarity matrix: " + to_string(totalVendorTime.count()) + " milliseconds\n";

	dataFile.close();
}

void Protocols::SimulateSingleMediatorWorkInComputingSimilarityMatrix(int N, int M, vector<vector<uint16_t>> someRShare, vector<vector<uint16_t>> someXiRShare, vector<vector<uint16_t>> someSqRShare, int numOfShares, string fileName)
{
	auto start = high_resolution_clock::now();

	vector<vector<uint16_t>> RShare(N, vector<uint16_t>(M));
	vector<vector<uint16_t>> SqRShare(N, vector<uint16_t>(M));
	vector<vector<uint16_t>> XiRShare(N, vector<uint16_t>(M));

	AddShare(RShare, someRShare);
	AddShare(SqRShare, someXiRShare);
	AddShare(XiRShare, someSqRShare);

	auto stop = high_resolution_clock::now();
	auto lines7To9Time = duration_cast<milliseconds>(stop - start);

	filesystem::path path(fileName);
	ofstream dataFile(path, ios_base::app);
	dataFile << "M1: " + to_string(lines7To9Time.count()) + " milliseconds\n";

	auto someCmShare = CreateRandomShares(N, numOfShares)[0];
	auto someClShare = CreateRandomShares(N, numOfShares)[0];


	start = high_resolution_clock::now();
	for (int i = 0; i < 1000; i++)
	{
		ScalarProductVectors(someCmShare, someClShare);
	}
	stop = high_resolution_clock::now();
	auto K2Time = duration_cast<milliseconds>(stop - start);

	K2Time = (K2Time * M * (M - 1) * 3) / (1000 * 2);
	dataFile << "K2: " + to_string(K2Time.count()) + " milliseconds\n";


	vector<uint64_t> coordinates;
	for (int i = 0; i < numOfShares; i++)
	{
		coordinates.push_back(rand() % 2147483648);
	}
	start = high_resolution_clock::now();
	for (int i = 0; i < 1000; i++)
	{
		auto a = ReconstructShamirSecret(coordinates);
	}
	stop = high_resolution_clock::now();
	auto K3Time = duration_cast<milliseconds>(stop - start);
	K3Time = (K3Time * M * (M - 1)) / (2 * 1000 * numOfShares);
	dataFile << "K3: " + to_string(K3Time.count()) + " milliseconds\n";

	dataFile.close();

}