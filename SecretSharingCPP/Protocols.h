#include <vector>

using namespace std;

class Protocols
{
public:
	Protocols() = delete;
	static void SimulateSingleVendorWorkInComputingSimilarityMatrix(vector<vector<int8_t>>& Rk, uint32_t numOfShares, const string& fileName);
	static vector<vector<int8_t>> ReadUserItemMatrix(string path);
	static vector<vector<uint16_t>> CreateRandomMatrixShare(int n, int m);
	static void SimulateSingleMediatorWorkInComputingSimilarityMatrix(int N, int M, vector<vector<uint16_t>>& someRShare, vector<vector<uint16_t>>& someXiRShare, vector<vector<uint16_t>>& someSqRShare, int numOfShares, string fileName);
	static void SimulateSingleMediatorWorkInComputingOfflinePart2(int numOfShares, int N, int M, int q, string fileName);
	static void SimulateSingleMediatorWorkInOnlinePredictRating(int numberOfItems, int numOfShares, string fileName);
	static void SimulateSingleMediatorWorkInOnlinePredictRanking(int M, int q, string fileName);
	static void SimulateSingleVendorWorkInOnlinePredictRanking(int M, int numberOfShares, int h, string fileName);
};