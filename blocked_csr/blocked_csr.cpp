#include "utils.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct CSRMatrix {
  std::vector<float> values;
  std::vector<int> colIndices;
  std::vector<int> rowPtrs;

  CSRMatrix() = default;
  CSRMatrix(int numRows) : rowPtrs(numRows + 1, 0) {}
};

std::vector<CSRMatrix> tileAndConvertToCSR(const std::vector<int> &rows,
                                           const std::vector<int> &cols,
                                           const std::vector<float> &values,
                                           int numRows, int numCols,
                                           int rowsPerBlock, int colsPerBlock) {

  std::vector<CSRMatrix> csrBlocks;

  std::map<std::pair<int, int>, CSRMatrix> tiles;

  // Cannot be parallelized
  for (size_t i = 0; i < rows.size(); ++i) {

    int blockRow = rows[i] / rowsPerBlock;
    int blockCol = cols[i] / colsPerBlock;

    // potential improvement, use integer instead of std::pair
    // the index of the tile should be (blockRow * tilesPerRow) + blockCol
    std::pair<int, int> key = std::make_pair(blockRow, blockCol);

    if (tiles.find(key) == tiles.end()) {
      tiles[key] = CSRMatrix(rowsPerBlock);
    }

    CSRMatrix &tile = tiles[key];
    tile.values.push_back(values[i]);
    tile.colIndices.push_back(cols[i] % colsPerBlock);
    tile.rowPtrs[rows[i] % rowsPerBlock + 1]++;
  }

  // outer loop can be parallelized
  for (auto &pair : tiles) {
    CSRMatrix &tile = pair.second;
    // cannot be parallelized (loop-carried dep.)
    for (int i = 1; i <= rowsPerBlock; ++i) {
      tile.rowPtrs[i] += tile.rowPtrs[i - 1];
    }
    csrBlocks.push_back(tile);
  }
  return csrBlocks;
}

std::vector<float> SpMV(const CSRMatrix &tile,
                        const std::vector<float> &denseVec, int colOffset) {

  std::vector<float> result(tile.rowPtrs.size() - 1, 0.0f);
  for (int i = 0; i < tile.rowPtrs.size() - 1; ++i) {
    float y = 0.0f;
    for (int j = tile.rowPtrs[i]; j < tile.rowPtrs[i + 1]; ++j) {
      y += tile.values[j] * denseVec[tile.colIndices[j] + colOffset];
    }
    result[i] += y;
  }

  return result;
}

void SpMVTiled(const std::vector<CSRMatrix> &csrBlocks,
               const std::vector<float> &denseVec, std::vector<float> &result,
               int numRows, int numCols, int rowsPerBlock, int colsPerBlock) {

  std::fill(result.begin(), result.end(), 0.0f);
  int tilesPerRow = (numCols + colsPerBlock - 1) / colsPerBlock;

  for (size_t blockIdx = 0; blockIdx < csrBlocks.size(); ++blockIdx) {
    int blockRow = blockIdx / tilesPerRow;
    int blockCol = blockIdx % tilesPerRow;

    int colOffset = blockCol * colsPerBlock;
    // This will be eventually replaced with MKL/ cuSparse call
    std::vector<float> tileResult =
        SpMV(csrBlocks[blockIdx], denseVec, colOffset);

    for (size_t i = 0; i < tileResult.size(); ++i) {

      int rowIdx = blockRow * rowsPerBlock + i;
      result[rowIdx] += tileResult[i];
    }
  }
}

int main() {

  // std::string filename = "bcsstk22.mtx";
  // std::string filename = "small_sparse_matrix.mtx";
  std::string filename = "small_sparse_matrix_8x8.mtx";
  std::vector<int> rows, cols;
  std::vector<float> values;
  // readMTX(rows, cols, values);

  int numRows; // Total number of rows in matrix
  int numCols; // Total number of columns in matrix
  int numNZ;   // Total number of non-zeros in a matrix
  // int rowsPerBlock = 32;
  // int colsPerBlock = 32;
  // int rowsPerBlock = 4;
  // int colsPerBlock = 4;

  readMTX(filename, rows, cols, values, &numRows, &numCols, &numNZ, true);

  // first try one tile
  int rowsPerBlock = numRows;
  int colsPerBlock = numCols;
  std::vector<CSRMatrix> csrBlocks = tileAndConvertToCSR(
      rows, cols, values, numRows, numCols, rowsPerBlock, colsPerBlock);

  // TODO: populate denseVec
  std::vector<float> denseVec(numCols);
  fillRandom(denseVec, 0.0f, 1.0f);

  // TODO: is this population redundant? or the one in the function?
  std::vector<float> result_ref(numRows, 0.0f);
  std::vector<float> result(numRows, 0.0f);

  SpMVTiled(csrBlocks, denseVec, result_ref, numRows, numCols, rowsPerBlock,
            colsPerBlock);

  rowsPerBlock = 4;
  colsPerBlock = 4;
  csrBlocks = tileAndConvertToCSR(rows, cols, values, numRows, numCols,
                                  rowsPerBlock, colsPerBlock);
  SpMVTiled(csrBlocks, denseVec, result, numRows, numCols, rowsPerBlock,
            colsPerBlock);

  float MAX_ERR = 0.001;
  int count_errors = 0;
  for (int i = 0; i < result.size(); ++i) {
    if (abs(result[i] - result_ref[i]) > MAX_ERR) {
      count_errors++;
      std::cout << result[i] << " " << result_ref[i] << std::endl;
    }
  }

  std::cout << "Total errors : " << count_errors << std::endl;

  /*
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
  */
}
