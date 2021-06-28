#include <vector>

class Protocols
{
public:
	Protocols() = delete;
	static void SimulateSingleVendorWorkInComputingSimilarityMatrix(std::vector<std::vector<int8_t>>& Rk, uint32_t numOfShares, std::string fileName);
};