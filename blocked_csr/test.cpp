#include <iostream>
#include <vector>
#include <string>
#include <map>
#include "utils.h"

int main() {

	std::string filename = "bcsstk22.mtx";
	std::vector<int> rows, cols;
    std::vector<float> values;

	int numRows; // Total number of rows in matrix
	int numCols; // Total number of columns in matrix
	int numNZ; // Total number of non-zeros in a matrix

    int *nnzRowIdx, *nnzColIdx;
    float *nnzVal;
    int nrows;
    int ncols; 
    int nnz; 

    loadMTX(nnzRowIdx, nnzColIdx, nnzVal, &nnz, &nrows, &ncols, filename, true);

    readMTX(filename, rows, cols, values, &numRows, &numCols, &numNZ, true);

    if (nnz != numNZ){
        std::cout << "nnz: " << nnz  << " , numNZ: " << numNZ << std::endl;
        exit(1);
    }

    int rowErrors = 0;
    int colErrors = 0; 
    int valErrors = 0;
    float MAX_ERR = 0.001;
    for (int i = 0 ; i < nnz; ++i){
        if (nnzRowIdx[i] != rows[i]){
            rowErrors++;
        }
        if (nnzColIdx[i] != cols[i]){
            colErrors++;
        }
        if (abs(nnzVal[i] - values[i]) > MAX_ERR){
            std::cout << nnzVal[i] << " vs " << values[i] << std::endl;
            valErrors++;
        }
    }

    if (rowErrors == 0 && colErrors == 0 && valErrors == 0){
        std::cout << "Verified Successfully!" << std::endl;
    }else {
        std::cout << "Verification failed" << std::endl;
        std::cout << "Row Errors: " << rowErrors << std::endl;
        std::cout << "Col Errors: " << colErrors << std::endl;
        std::cout << "Val Errors: " << valErrors << std::endl;
    }
}



