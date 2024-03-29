#ifndef SPMAT_H
#define SPMAT_H

#include <vector>
#include <string>
#include <map>
#include "utils.h"

#include "mkl_spblas.h"
#include "mkl_types.h"


template <typename T, typename I>
struct CSRMatrix {
    std::vector<T> values;
    std::vector<I> colIndices;
    std::vector<I> rowPtrs;

    CSRMatrix() = default;
    CSRMatrix(int numRows) : rowPtrs(numRows + 1, 0) {}

};

template <typename T, typename I=int>
class SpMat{
    
public:

    SpMat() = delete;
    SpMat(const std::string &filename);
    SpMat(const std::string &filename, const int rowsPerBlock, const int colsPerBlock);
    SpMat(const std::string &filename, const int blockSize);

    SpMat(const std::vector<I>& rows, const std::vector<I>& cols, const std::vector<T>& vals);

    SpMat(const std::vector<I>& rows, const std::vector<I>& cols, const std::vector<T>& vals, const int rowsPerBlock, const int colsPerBlock);
    SpMat(const std::vector<I>& rows, const std::vector<I>& cols, const std::vector<T>& vals, const int blockSize);
    void tile(const int rowsPerBlock, const int colsPerBlock);
    void multiply(const std::vector<T>& denseVec, std::vector<T>& result);

    void printTiles() const;
    std::vector<T> values() const { return coo_values_; }
    std::vector<I> rows() const { return coo_rows_; }
    std::vector<I> cols() const { return coo_cols_; }
    int nrows() const {return numRows_;}
    int ncols() const {return numCols_;}
    int nnz() const {return numNZ_;}

private:
    std::vector<I> coo_rows_;
    std::vector<I> coo_cols_;
    std::vector<T> coo_values_;
    std::map<std::pair<int, int>, CSRMatrix<T,I>> csrBlocks_;
    int numRows_;
    int numCols_;
    int numNZ_;
    int rowsPerBlock_;
    int colsPerBlock_;
    bool tiled_=false;
    void tileAndConvertToCSR_();
    virtual void SpMVTiled_(const std::vector<T>& denseVec, std::vector<T>& result);
    virtual void SpMV_(const CSRMatrix<T,I>& tile, const T* denseVec, T* result);
    void SpMV_ref_(const CSRMatrix<T,I>& tile, const T* denseVec, T* result);
};

#include "spmat_impl.h"
#endif

