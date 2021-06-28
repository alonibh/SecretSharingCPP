#include <string>
#include <chrono>
#include <stdint.h>
#include <math.h>

#include "Protocols.h"

void CalcSq(std::vector<std::vector<int8_t>>& Rk)
{
	for (int i = 0; i < Rk.size(); ++i)
	{
		for (int j = 0; j < Rk[0].size(); ++j)
		{
			if (-1 != Rk[i][j])
			{
				Rk[i][j] = pow(Rk[i][j], 2);
			}
		}
	}
}

void Protocols::SimulateSingleVendorWorkInComputingSimilarityMatrix(std::vector<std::vector<int8_t>>& Rk, uint32_t numOfShares, std::string fileName)
{
	auto now = std::chrono::system_clock::now();

}