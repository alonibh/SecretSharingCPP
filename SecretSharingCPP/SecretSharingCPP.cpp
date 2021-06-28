#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "Protocols.h"

void MeasureOfflinePart1(std::string dataset, int k, int D, int q, int h)
{
	static const std::string DIRECTORY_NAME = "Test";

	std::cout << "Dataset - " << dataset << ", k" << " = {k}, D = {D} Started";
	std::filesystem::create_directory(DIRECTORY_NAME);
	std::filesystem::path path(DIRECTORY_NAME + "\\abc.txt");
	std::ofstream dataFile(path,std::ios_base::app);

	dataFile << "\n1234";
	dataFile.close();
}

void main(void)
{
	MeasureOfflinePart1("Koko", 1, 2, 3, 4);
	std::vector<std::vector<int8_t>> Rk;
	std::vector<std::vector<int8_t>> Rx;
	Protocols::SimulateSingleVendorWorkInComputingSimilarityMatrix(Rk, 2, "abc");

}