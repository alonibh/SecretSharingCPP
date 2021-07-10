#include <string>
#include <chrono>
#include <stdint.h>
#include <math.h>
#include <fstream>
#include <algorithm>
#include "Protocols.h"
#include <iostream>
#include <filesystem>
#include<numeric>
#include <random>
#include <ppl.h>

using namespace Concurrency;
using namespace std;
using namespace std::chrono;

const uint64_t PRIME = 2147483647;
const int Q = 100;

struct UserRating { int UserId; int ItemId; int Rating; };
struct ScoreAndIndex { uint32_t score; int index; };
struct score_comparer
{
	inline bool operator() (const ScoreAndIndex& struct1, const ScoreAndIndex& struct2)
	{
		return (struct1.score > struct2.score);
	}
};
struct uint64_comparer
{
	bool operator() (uint64_t i, uint64_t j) { return (i < j); }
};


uint64_t ModForNegative(long x)
{
	long prime = 2147483647;
	return ((x % prime + prime) % prime);
}

vector<int> CreatePermutation(int size)
{
	vector<int> vec(size);
	for (size_t i = 0; i < size; i++)
	{
		vec[i] = i;
	}

	auto rng = default_random_engine{};
	shuffle(begin(vec), end(vec), rng);

	return vec;
}

void GetSimilarityVectorForTopSimilarItemsToM(const vector<vector<uint8_t>>& similarityMatrix, int m, int q, bool isPositivesOnly, vector<uint8_t>& sm)
{
	size_t vectorLength = similarityMatrix.size();
	vector<ScoreAndIndex> similarityScoreAndIndex(vectorLength, { 0,0 });

	for (int i = 0; i < vectorLength; i++)
	{
		similarityScoreAndIndex[i] = { similarityMatrix[m][i], i };
	}

	sort(similarityScoreAndIndex.begin(), similarityScoreAndIndex.end(), score_comparer());
	vector<ScoreAndIndex> topSimilarityScoreAndIndex(similarityScoreAndIndex.begin(), similarityScoreAndIndex.begin() + q);

	for (const auto& item : similarityScoreAndIndex)
	{
		if (isPositivesOnly)
		{
			if (item.score > 0)
			{
				sm[item.index] = item.score;
			}
		}
		else
		{
			sm[item.index] = item.score;
		}
	}
}

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
	for (size_t i = 0; i < length; i++)
	{
		sum += vector1[i] * vector2[i];
	}
	return sum % PRIME;
}

uint64_t ScalarProductVectors(vector<int>& vector1, vector<int>& vector2)
{
	int length = vector1.size();
	uint64_t sum = 0;
	for (size_t i = 0; i < length; i++)
	{
		sum += vector1[i] * vector2[i];
	}
	return sum % PRIME;
}

vector<vector<int8_t>> CalcSq(const vector<vector<int8_t>>& matrix, vector<vector<int8_t>>& sq)
{
	int N = matrix.size();
	int M = matrix[0].size();

	for (size_t i = 0; i < N; ++i)
	{
		for (size_t j = 0; j < M; ++j)
		{
			if (-1 != matrix[i][j])
			{
				sq[i][j] = pow(matrix[i][j], 2);
			}
		}
	}
	return sq;
}

vector<vector<int8_t>> CalcXi(const vector<vector<int8_t>>& matrix, vector<vector<int8_t>>& xi)
{
	int N = matrix.size();
	int M = matrix[0].size();

	for (size_t i = 0; i < N; ++i)
	{
		for (size_t j = 0; j < M; ++j)
		{
			if (-1 != matrix[i][j])
			{
				xi[i][j] = (matrix[i][j] == 0 ? 0 : 1);
			}
		}
	}
	return xi;
}

uint32_t ShamirSecretSharingNoStoring(uint32_t scalar, int numOfShares)
{
	int maxRangeForRandom = 65535;
	int minRangeForRandom = 2;
	int range = maxRangeForRandom - minRangeForRandom + 1;
	uint32_t lastY = 0;

	if (numOfShares == 3)
	{
		uint32_t a = rand() % range + minRangeForRandom;
		lastY = scalar;

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
	return lastY;
}

void AddShare(vector<vector<uint16_t>>& matrix, vector<vector<uint16_t>>& share)
{
	int N = matrix.size();
	int M = matrix[0].size();

	for (size_t i = 0; i < N; i++)
	{
		for (size_t j = 0; j < M; j++)
		{
			matrix[i][j] += share[i][j];
		}
	}
}

vector<vector<uint16_t>> CreateRandomShares(int n, int numOfShares)
{
	vector<vector<uint16_t>> shares(numOfShares, vector<uint16_t>(n));
	int maxRangeForRandom = 65535;
	int minRangeForRandom = 2;
	int range = maxRangeForRandom - minRangeForRandom + 1;

	for (size_t i = 0; i < numOfShares; i++)
	{
		vector<uint16_t> vector(n, 0);
		for (size_t j = 0; j < n; j++)
		{
			vector[j] = rand() % range + minRangeForRandom;
		}
		shares[i] = vector;
	}
	return shares;
}

vector<vector<uint8_t>> CreateRandomMatrix(int M)
{
	vector<vector<uint8_t>> similarityMatrix(M, vector<uint8_t>(M));

	for (size_t i = 0; i < M; i++)
	{
		for (size_t j = i + 1; j < M; j++)
		{
			uint8_t similarityScore = rand() % Q;
			////Convert to integer value
			similarityMatrix[i][j] = similarityScore;
			similarityMatrix[j][i] = similarityScore;
		}
	}
	return similarityMatrix;
}

vector<vector<int8_t>> CreateRandomUserItemMatrix(int N, int M)
{
	vector<vector<int8_t>> userItemMatrix(N, vector<int8_t>(M));

	for (size_t i = 0; i < N; i++)
	{
		for (size_t j = 0; j < M; j++)
		{
			userItemMatrix[i][j] = rand() % 6;
		}
	}
	return userItemMatrix;
}

vector<vector<int8_t>> Protocols::ReadUserItemMatrix(string path)
{
	cout << "Start ReadUserItemMatrix" << endl;

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

	vector<vector<int8_t>> userItemMatrix(N, vector<int8_t>(M));
	for (UserRating rating : ratings) {
		userItemMatrix[rating.UserId - 1][rating.ItemId - 1] = rating.Rating;
	}

	cout << "Done ReadUserItemMatrix" << endl;

	return userItemMatrix;
}

vector<vector<uint16_t>> Protocols::CreateRandomMatrixShare(int n, int m)
{
	vector<vector<uint16_t>> matrix(n, vector<uint16_t>(m));
	int maxRangeForRandom = 65535;

	for (size_t i = 0; i < n; i++)
	{
		for (size_t j = 0; j < m; j++)
		{
			matrix[i][j] = rand() % (maxRangeForRandom + 1);
		}
	}
	return matrix;
}

void Protocols::SimulateSingleVendorWorkInComputingSimilarityMatrix(vector<vector<int8_t>>& Rk, uint32_t numOfShares, const string& fileName)
{
	vector<vector<int8_t>> randomMatrix = CreateRandomUserItemMatrix(10000, 1000);

	int N = Rk.size();
	int M = Rk[0].size();
	int fakeN = 10000;
	int fakeM = 1000;

	auto start = high_resolution_clock::now();

	vector<vector<int8_t>> sq(fakeN, vector<int8_t>(fakeM));
	vector<vector<int8_t>> xi(fakeN, vector<int8_t>(fakeM));

	CalcSq(randomMatrix, sq);
	CalcXi(randomMatrix, xi);

	auto stop = high_resolution_clock::now();

	cout << "--ignore " << to_string(sq[0][0]) << " " << to_string(xi[0][0]) << endl;
	sq.clear();
	sq.shrink_to_fit();
	xi.clear();
	xi.shrink_to_fit();

	auto ATime = duration_cast<milliseconds>(stop - start);
	double ratio = ((double)N / fakeN) * ((double)M / fakeM);
	ATime *= ratio;

	filesystem::path path(fileName);
	ofstream dataFile(path, ios_base::app);
	dataFile << "A time is: " + to_string(ATime.count()) + " milliseconds\n";

	uint32_t scalar = rand() % 2147483648;
	start = high_resolution_clock::now();

	for (int i = 0; i < 10000000; i++)
	{
		scalar = ShamirSecretSharingNoStoring(scalar, numOfShares);
	}

	stop = high_resolution_clock::now();

	cout << "--ignore " << to_string(scalar) << endl;

	auto FDTime = duration_cast<milliseconds>(stop - start);

	FDTime = FDTime * 3 * Rk.size() * Rk[0].size() / 10000000;

	dataFile << "F(" + to_string(numOfShares) + ")*3*N*M: " + to_string(FDTime.count()) + " milliseconds\n";

	auto totalVendorTime = ATime + FDTime;
	dataFile << "Total time for a single vendor computing the similarity matrix: " + to_string(totalVendorTime.count()) + " milliseconds\n";

	dataFile.close();
}

void Protocols::SimulateSingleMediatorWorkInComputingSimilarityMatrix(int N, int M, vector<vector<uint16_t>>& someRShare, vector<vector<uint16_t>>& someXiRShare, vector<vector<uint16_t>>& someSqRShare, int numOfShares, string fileName)
{
	int fakeN = someRShare.size();
	int fakeM = someRShare[0].size();
	double ratio = ((double)N / fakeN) * ((double)M / fakeM);

	auto start = high_resolution_clock::now();

	vector<vector<uint16_t>> RShare(fakeN, vector<uint16_t>(fakeM));
	vector<vector<uint16_t>> SqRShare(fakeN, vector<uint16_t>(fakeM));
	vector<vector<uint16_t>> XiRShare(fakeN, vector<uint16_t>(fakeM));

	AddShare(RShare, someRShare);
	AddShare(SqRShare, someSqRShare);
	AddShare(XiRShare, someXiRShare);

	auto stop = high_resolution_clock::now();

	cout << "--ignore " << to_string(RShare[0][0]) << " " << to_string(SqRShare[0][0]) << to_string(XiRShare[0][0]) << endl;

	auto lines7To9Time = duration_cast<milliseconds>(stop - start);
	lines7To9Time *= ratio;

	filesystem::path path(fileName);
	ofstream dataFile(path, ios_base::app);
	dataFile << "M1: " + to_string(lines7To9Time.count()) + " milliseconds\n";

	RShare.clear();
	RShare.shrink_to_fit();
	SqRShare.clear();
	SqRShare.shrink_to_fit();
	XiRShare.clear();
	XiRShare.shrink_to_fit();

	uint64_t tmp;

	auto someCmShare = CreateRandomShares(N, numOfShares)[0];
	auto someClShare = CreateRandomShares(N, numOfShares)[0];

	start = high_resolution_clock::now();

	size_t size = 1000;

	parallel_for(size_t(0), size, [&](size_t i)
		{
			tmp = ScalarProductVectors(someCmShare, someClShare);
		});

	stop = high_resolution_clock::now();

	cout << "--ignore " << to_string(tmp) << endl;

	auto K2Time = duration_cast<milliseconds>(stop - start);
	K2Time = (K2Time * M * (M - 1) * 3) / (size * 2);
	dataFile << "K2 (parallel): " + to_string(K2Time.count()) + " milliseconds\n";

	start = high_resolution_clock::now();

	for (size_t i = 0; i < size; i++)
	{
		tmp = ScalarProductVectors(someCmShare, someClShare);
	}

	stop = high_resolution_clock::now();

	cout << "--ignore " << to_string(tmp) << endl;

	K2Time = duration_cast<milliseconds>(stop - start);
	K2Time = (K2Time * M * (M - 1) * 3) / (size * 2);
	dataFile << "K2: " + to_string(K2Time.count()) + " milliseconds\n";

	vector<uint64_t> coordinates(numOfShares);
	for (int i = 0; i < numOfShares; i++)
	{
		coordinates[i] = rand() % 2147483648;
	}

	start = high_resolution_clock::now();

	for (size_t i = 0; i < size; i++)
	{
		tmp = ReconstructShamirSecret(coordinates);
	}

	stop = high_resolution_clock::now();

	cout << "--ignore " << to_string(tmp) << endl;

	auto K3Time = duration_cast<milliseconds>(stop - start);
	K3Time = (K3Time * 3 * M * (M - 1)) / (2 * size * numOfShares);
	dataFile << "K3: " + to_string(K3Time.count()) + " milliseconds\n";

	dataFile.close();
}

void Protocols::SimulateSingleMediatorWorkInComputingOfflinePart2(int numOfShares, int N, int M, int q, string fileName)
{
	vector<vector<uint16_t>> ratingVectorsSharesArray[100];
	vector<vector<uint16_t>> XiRatingVectorsSharesArray[100];

	for (size_t i = 0; i < 100; i++)
	{
		auto ratingVectorShares = CreateRandomShares(N, numOfShares);
		ratingVectorsSharesArray[i] = ratingVectorShares;

		auto XiRatingVectorShares = CreateRandomShares(N, numOfShares);
		XiRatingVectorsSharesArray[i] = XiRatingVectorShares;
	}

	vector<uint64_t> Xds[100];
	vector<uint64_t> Yds[100];

	auto start = high_resolution_clock::now();

	for (size_t itemIndex = 0; itemIndex < 100U; itemIndex++)
	{
		for (const auto& share : ratingVectorsSharesArray[itemIndex])
		{
			Xds[itemIndex].push_back(accumulate(share.begin(), share.end(), 0));
		}

		for (const auto& share : XiRatingVectorsSharesArray[itemIndex])
		{
			Yds[itemIndex].push_back(accumulate(share.begin(), share.end(), 0));
		}
	}

	auto stop = high_resolution_clock::now();
	auto XTime = duration_cast<milliseconds>(stop - start);
	XTime = (XTime * M) / (100 * numOfShares);

	filesystem::path path(fileName);
	ofstream dataFile(path, ios_base::app);
	dataFile << "X time is: " + to_string(XTime.count()) + " milliseconds\n";

	start = high_resolution_clock::now();

	double averageRatings[100];
	for (size_t itemIndex = 0; itemIndex < 100U; itemIndex++)
	{
		vector<uint64_t> xCoordinates(numOfShares, 0);
		size_t i = 0;
		for (const auto& xd : Xds[itemIndex])
		{
			xCoordinates[i] = xd;
			i++;
		}
		auto x = ReconstructShamirSecret(xCoordinates);

		vector<uint64_t> yCoordinates(numOfShares, 0);
		i = 0;
		for (const auto& yd : Yds[itemIndex])
		{
			yCoordinates[i] = yd;
			i++;
		}
		auto y = ReconstructShamirSecret(yCoordinates);

		if (y != 0)
		{
			averageRatings[itemIndex] = x / y;
		}
	}

	stop = high_resolution_clock::now();
	auto YTime = duration_cast<milliseconds>(stop - start);
	YTime = (YTime * M) / (100 * numOfShares);
	dataFile << "Y time is: " + to_string(YTime.count()) + " milliseconds\n";

	auto similarityMatrix = CreateRandomMatrix(M);
	vector<double> randomAverageRatings(M, 0);

	for (size_t i = 0; i < M; i++)
	{
		auto randomScore = (float)rand() / RAND_MAX;

		randomAverageRatings[i] = randomScore * 5;
	}

	start = high_resolution_clock::now();

	int x = 0;
	int fakeM = 1000;
	double ratio = (double)M / fakeM;

	for (int m = 0; m < fakeM; m++)
	{
		size_t vectorLength = similarityMatrix.size();
		vector<uint8_t> sm(vectorLength, 0);
		GetSimilarityVectorForTopSimilarItemsToM(similarityMatrix, m, q, true, sm);
		vector<uint8_t> sTagM(vectorLength, 0);
		GetSimilarityVectorForTopSimilarItemsToM(similarityMatrix, m, q, false, sTagM);
		vector<uint64_t> cl(M);
		for (int i = 0; i < M; i++)
		{
			cl[i] = round(Q * sm[i] * randomAverageRatings[i]);
		}
		x += sm[0] + sTagM[0] + cl[0];
	}

	stop = high_resolution_clock::now();

	cout << "--ignore " + to_string(x) << endl;

	auto ZTime = duration_cast<milliseconds>(stop - start);
	ZTime = ZTime / numOfShares;
	ZTime *= ratio;

	dataFile << "Z time is: " + to_string(ZTime.count()) + " milliseconds\n";

	dataFile << "Total time for each mediator computing offline part 2: " + to_string(XTime.count() + YTime.count() + ZTime.count()) + " milliseconds\n";

	dataFile.close();

}

void Protocols::SimulateSingleMediatorWorkInOnlinePredictRating(int numberOfItems, int numOfShares, string fileName)
{
	vector<vector<uint16_t>> smVectors(100, vector<uint16_t>(numberOfItems));
	for (int i = 0; i < 100; i++)
	{
		vector<uint16_t> smVector(numberOfItems, 0);
		for (int j = 0; j < numberOfItems; j++)
		{
			smVector[j] = rand() % 6;
		}
		smVectors[i] = smVector;
	}

	vector<vector<uint16_t>> sharesArray[100];

	for (int i = 0; i < 100; i++)
	{
		auto smVector = smVectors[i];

		auto smShares = CreateRandomShares(numberOfItems, numOfShares);
		sharesArray[i] = smShares;
	}

	vector<double> ratingsVector(numberOfItems, 0);
	for (int i = 0; i < numberOfItems; i++)
	{
		ratingsVector[i] = rand() % 6;
	}

	auto XTime = milliseconds::zero();
	auto YTime = milliseconds::zero();

	for (const auto& shares : sharesArray)
	{
		vector<uint64_t> xds(shares.size());
		auto Xstart = high_resolution_clock::now();
		size_t i = 0;

		for (const auto& share : shares)
		{
			double xd = 0;
			for (size_t itemCounter = 0; itemCounter < numberOfItems; itemCounter++)
			{
				xd += share[itemCounter] * ratingsVector[itemCounter];
			}
			xd = round(xd);
			xds[i] = xd;
			i++;
		}

		auto Xstop = high_resolution_clock::now();
		XTime += duration_cast<milliseconds>(Xstop - Xstart);

		auto Ystart = high_resolution_clock::now();

		auto tmp = ReconstructShamirSecret(xds);

		auto Ystop = high_resolution_clock::now();

		cout << "--ignore " << to_string(tmp) << endl;

		YTime += duration_cast<milliseconds>(Ystop - Ystart);
	}
	XTime = XTime / 100;
	YTime = YTime / 100;

	filesystem::path path(fileName);
	ofstream dataFile(path, ios_base::app);
	dataFile << "X time is: " + to_string(XTime.count()) + " milliseconds\n";

	dataFile << "Y time is: " + to_string(YTime.count()) + " milliseconds\n";

	dataFile << "Total time for each mediator computing online predict rating: " + to_string((XTime.count() + YTime.count()) * 3) + " milliseconds\n";

	dataFile.close();

}

void Protocols::SimulateSingleMediatorWorkInOnlinePredictRanking(int M, int q, string fileName)
{
	auto start = high_resolution_clock::now();

	auto tmp = CreatePermutation(M);

	auto stop = high_resolution_clock::now();

	cout << "--ignore " << to_string(tmp[0]) << endl;

	auto XTime = duration_cast<milliseconds>(stop - start);

	filesystem::path path(fileName);
	ofstream dataFile(path, ios_base::app);
	dataFile << "X time is: " + to_string(XTime.count()) + " milliseconds\n";

	vector<int> topItemsSimilarityVector(M);
	for (size_t i = 0; i < q; i++)
	{
		topItemsSimilarityVector[i] = rand() % 100;
	}

	vector<int> xiRVector(M);
	for (size_t i = 0; i < M; i++)
	{
		xiRVector[i] = rand() % PRIME;
	}

	int x = 0;
	int fakeM = 5000;

	double ratio = (double)M / fakeM;

	start = high_resolution_clock::now();

	for (int i = 0; i < 100; i++)
	{
		vector<uint64_t> Xd(fakeM);
		vector<uint64_t> XdShares(fakeM);
		vector<uint64_t> oneMinusXiShares(fakeM);

		long addon = Q * q + 1;

		for (size_t itemIndex = 0; itemIndex < fakeM; itemIndex++)
		{
			long Xdm = addon;
			Xdm += ScalarProductVectors(topItemsSimilarityVector, xiRVector);
			XdShares[itemIndex % M] = Xdm;
			oneMinusXiShares[itemIndex % M] = (1 - xiRVector[itemIndex % M]);
		}

		for (size_t shareIndex = 0; shareIndex < fakeM; shareIndex++)
		{
			long mult = XdShares[shareIndex % M] * oneMinusXiShares[shareIndex % M];
			auto mod = ModForNegative(mult);
			Xd[shareIndex % M] = mod;
		}

		x += (Xd[0] + XdShares[0] + oneMinusXiShares[0]);
	}

	stop = high_resolution_clock::now();

	cout << "--ignore " << to_string(x) << endl;

	auto YTime = duration_cast<milliseconds>(stop - start);
	YTime /= 100;
	YTime *= ratio;

	dataFile << "Y time is: " + to_string(YTime.count()) + " milliseconds\n";

	dataFile << "Total time for each mediator computing online predict ranking: " + to_string(XTime.count() + YTime.count()) + " milliseconds\n";
}

void Protocols::SimulateSingleVendorWorkInOnlinePredictRanking(int M, int numberOfShares, int h, string fileName)
{
	auto vendorTime = milliseconds::zero();

	for (int i = 0; i < 100; i++)
	{
		vector<uint64_t> coordinates(M);
		for (size_t j = 0; j < M; j++)
		{
			coordinates[j] = rand() % PRIME;
		}

		auto start = high_resolution_clock::now();

		auto tmp = ReconstructShamirSecret(coordinates);
		sort(coordinates.begin(), coordinates.end(), uint64_comparer());
		vector<uint64_t> top(coordinates.begin(), coordinates.begin() + h);

		auto stop = high_resolution_clock::now();

		cout << "--ignore " << to_string(tmp) << " " << top[0] << endl;

		vendorTime += duration_cast<milliseconds>(stop - start);
	}

	vendorTime /= 100;

	filesystem::path path(fileName);
	ofstream dataFile(path, ios_base::app);
	dataFile << "Total time for a single vendor computing online predict ranking: " + to_string(vendorTime.count()) + " milliseconds\n";
}