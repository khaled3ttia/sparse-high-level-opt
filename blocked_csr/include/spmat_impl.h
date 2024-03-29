template <typename T, typename I> SpMat<T,I>::SpMat(const std::string &filename){
    readMTX(filename, coo_rows_, coo_cols_, coo_values_, &numRows_, &numCols_, &numNZ_, true);
}

template <typename T, typename I> SpMat<T,I>::SpMat(const std::string &filename, const int rowsPerBlock, const int colsPerBlock){
    readMTX(filename, coo_rows_, coo_cols_, coo_values_, &numRows_, &numCols_, &numNZ_, true);
    tile(rowsPerBlock, colsPerBlock);
}

template <typename T, typename I> SpMat<T,I>::SpMat(const std::string &filename, const int blockSize){
    readMTX(filename, coo_rows_, coo_cols_, coo_values_, &numRows_, &numCols_, &numNZ_, true);
    tile(blockSize, blockSize);
}

template <typename T, typename I> SpMat<T,I>::SpMat(const std::vector<I>& rows, const std::vector<I>& cols, const std::vector<T>& vals) : coo_rows_(rows), coo_cols_(cols), coo_values_(vals){}

template <typename T, typename I> SpMat<T,I>::SpMat(const std::vector<I>& rows, const std::vector<I>& cols, const std::vector<T>& vals, const int rowsPerBlock, const int colsPerBlock) : SpMat(rows, cols, vals){
    tile(rowsPerBlock, colsPerBlock);
}

template <typename T, typename I> SpMat<T,I>::SpMat(const std::vector<I>& rows, const std::vector<I>& cols, const std::vector<T>& vals, const int blockSize) : SpMat(rows, cols, vals){
    tile(blockSize, blockSize);
}



template <typename T, typename I> void SpMat<T,I>::tile(int rowsPerBlock, int colsPerBlock){
    rowsPerBlock_ = rowsPerBlock;
    colsPerBlock_ = colsPerBlock;
    tileAndConvertToCSR_();
    tiled_ = true;
}

template <typename T, typename I> void SpMat<T,I>::tileAndConvertToCSR_(){

  // Cannot be parallelized
  for (size_t i = 0; i < coo_rows_.size(); ++i) {

    int blockRow = coo_rows_[i] / rowsPerBlock_;
    int blockCol = coo_cols_[i] / colsPerBlock_;

    // potential improvement, use integer instead of std::pair
    // the index of the tile should be (blockRow * tilesPerRow) + blockCol
    std::pair<int, int> key = std::make_pair(blockRow, blockCol);

    if (csrBlocks_.find(key) == csrBlocks_.end()) {
      csrBlocks_[key] = CSRMatrix<T,I>(rowsPerBlock_);
    }

    CSRMatrix<T,I> &tile = csrBlocks_[key];
    tile.values.push_back(coo_values_[i]);
    tile.colIndices.push_back(coo_cols_[i] % colsPerBlock_);

    tile.rowPtrs[coo_rows_[i] % rowsPerBlock_ + 1]++;
  }

  // outer loop can be parallelized
  for (auto &pair : csrBlocks_) {
    CSRMatrix<T,I> &tile = pair.second;
    // cannot be parallelized (loop-carried dep.)
    for (int i = 1; i <= rowsPerBlock_; ++i) {
      tile.rowPtrs[i] += tile.rowPtrs[i - 1];
    }
  }

}

template <typename T, typename I> void SpMat<T,I>::multiply(const std::vector<T>& denseVec, std::vector<T>& result){

    if (tiled_) SpMVTiled_(denseVec, result);
}


template <typename T, typename I> void SpMat<T,I>::SpMVTiled_(const std::vector<T>& denseVec, std::vector<T>& result){
  std::fill(result.begin(), result.end(), T{0});
  int tilesPerRow = (numCols_ + colsPerBlock_ - 1) / colsPerBlock_;

  for (auto& pair : csrBlocks_){
    
    //int blockRow = blockIdx / tilesPerRow;
    //int blockCol = blockIdx % tilesPerRow;
    
    int blockRow = pair.first.first;
    int blockCol = pair.first.second; 

    const CSRMatrix<T,I>& tile = pair.second;

    int rowOffset = blockRow * rowsPerBlock_;
    int colOffset = blockCol * colsPerBlock_;

    const T* subOperand = &denseVec[colOffset];
    T* subResult = &result[rowOffset];
    // This will be eventually replaced with MKL/ cuSparse call
    SpMV_(tile, subOperand, subResult);

  }

}


template <typename T, typename I> void SpMat<T,I>::SpMV_ref_(const CSRMatrix<T,I> &tile, const T* denseVec, T* result){

    //result.assign(tile.rowPtrs.size() - 1, T{0});
  for (int i = 0; i < tile.rowPtrs.size() - 1; ++i) {
    float y = T{0};
    for (int j = tile.rowPtrs[i]; j < tile.rowPtrs[i + 1]; ++j) {
      y += tile.values[j] * denseVec[tile.colIndices[j]];
    }
    result[i] += y;
  }

}


template <typename T, typename I> void SpMat<T,I>::SpMV_(const CSRMatrix<T,I> &tile, const T* denseVec, T* result){

    sparse_matrix_t csrA;
     
    auto constexpr single_prec =  (std::is_same_v<T, float>);
    auto constexpr double_prec = (std::is_same_v<T, double>);

    I* rowPtrs = &(tile.rowPtrs[0]);
    I* colIndices = &(tile.colIndices[0]);
    T* values = &(tile.values[0]);

    if (double_prec){
        mkl_sparse_d_create_csr(&csrA, SPARSE_INDEX_BASE_ZERO, numRows_, numCols_, rowPtrs, rowPtrs+1, colIndices, tile.values);
    }else if (single_prec){
        mkl_sparse_s_create_csr(&csrA, SPARSE_INDEX_BASE_ZERO, numRows_, numCols_, rowPtrs, rowPtrs+1, colIndices, values);
    }

    matrix_descr descrA; 
    descrA.type = SPARSE_MATRIX_TYPE_GENERAL;
    mkl_sparse_optimize(csrA);

    if (double_prec){

        mkl_sparse_d_mv(SPARSE_OPERATION_NON_TRANSPOSE, T{1.0}, csrA, descrA, denseVec, T{0.0}, result);
    }else if (single_prec){
        mkl_sparse_s_mv(SPARSE_OPERATION_NON_TRANSPOSE, T{1.0}, csrA, descrA, denseVec, T{0.0}, result);
    }
}


template <typename T, typename I> void SpMat<T,I>::printTiles() const {


    for (auto& pair : csrBlocks_){
        int blockRowIdx = pair.first.first;
        int blockColIdx = pair.first.second;
        CSRMatrix<T,I> tile = pair.second;

        std::cout << "Tile (" << blockRowIdx << "," << blockColIdx << ")" << std::endl;
        std::cout << "RowPtrs: "; 
        for (auto& rowPtr : tile.rowPtrs) {
            std::cout << rowPtr << " ";
        }
        std::cout << std::endl;
        
        std::cout << "ColIdx: ";
        for (auto& colIdx: tile.colIndices){
            std::cout << colIdx << " ";
        }
        std::cout << std::endl;

        std::cout << "Values: ";
        for (auto& val: tile.values){
            std::cout << val << " ";
        }
        std::cout << std::endl;
        std::cout << "==========================" << std::endl;
    }
}
