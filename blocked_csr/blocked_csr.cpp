#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "utils.h"

struct CSRMatrix {
	std::vector<float> values;
	std::vector<int> colIndices;
	std::vector<int> rowPtrs;
    
    CSRMatrix() = default;
	CSRMatrix(int numRows) : rowPtrs(numRows + 1 , 0) {}
};


std::vector<CSRMatrix> tileAndConvertToCSR(const std::vector<int>& rows, const std::vector<int>& cols, const std::vector<float>& values, int numRows, int numCols, int rowsPerBlock, int colsPerBlock) {

	std::vector<CSRMatrix> csrBlocks; 

	std::map<std::pair<int, int>, CSRMatrix> tiles;

	for (size_t i = 0 ; i < rows.size(); ++i){
		
		int blockRow = rows[i] / rowsPerBlock;
		int blockCol = cols[i] / colsPerBlock;

		std::pair<int, int> key = std::make_pair(blockRow, blockCol);

		if (tiles.find(key) == tiles.end()){
			tiles[key] = CSRMatrix(rowsPerBlock);
		}

		CSRMatrix& tile = tiles[key];
		tile.values.push_back(values[i]);
		tile.colIndices.push_back(cols[i] % colsPerBlock);
		tile.rowPtrs[rows[i] % rowsPerBlock + 1]++;

	}

	for (auto& pair : tiles) {
		CSRMatrix& tile = pair.second; 
		for (int i = 1 ; i <= rowsPerBlock; ++i) {
			tile.rowPtrs[i] += tile.rowPtrs[i -1];
		}
		csrBlocks.push_back(tile);
	}
	return csrBlocks;

}

int main() {

	std::string filename = "bcsstk22.mtx";
	std::vector<int> rows, cols;
    std::vector<float> values;
	//readMTX(rows, cols, values);

	int numRows; // Total number of rows in matrix
	int numCols; // Total number of columns in matrix
	int numNZ; // Total number of non-zeros in a matrix
	int rowsPerBlock = 32;
	int colsPerBlock = 32;


    readMTX(filename, rows, cols, values, &numRows, &numCols, &numNZ, true);

	std::vector<CSRMatrix> csrBlocks = tileAndConvertToCSR(rows, cols, values, numRows, numCols, rowsPerBlock, colsPerBlock);

    int tileIdx = 0; 
    for (CSRMatrix& tile: csrBlocks){
        std::cout << "Tile " << tileIdx << std::endl;
        std::cout << "Values: " << std::endl;
        for (auto& val: tile.values){
            std::cout << val << " ";
        }
        std::cout << std::endl;

        std::cout << "Cols:" << " ";
        for (auto& colIdx : tile.colIndices){
            std::cout << colIdx << " ";
        }
        std::cout << std::endl;

        std::cout << "RowPtrs:" << std::endl;
        for (auto& rowPtr: tile.rowPtrs){
            std::cout << rowPtr << " ";
        }
        std::cout << std::endl;
        tileIdx++;
        std::cout << "=================================" <<std::endl;
    }
}



