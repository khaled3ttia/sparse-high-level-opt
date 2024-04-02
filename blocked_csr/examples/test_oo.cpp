#include "spmat.h"

int verify(std::vector<float> gold, std::vector<float> actual, float max_err) {
  if (gold.size() != actual.size()) {
    std::cout << "Sizes do not match. Verification failed!" << std::endl;
    return -1;
  }
  // std::cout << "Output size: " << gold.size() << std::endl;

  int count_errors = 0;
  for (int i = 0; i < actual.size(); ++i) {
    if (abs(actual[i] - gold[i]) > max_err) {
      // std::cout << "Error: at row (" << i << "): " << actual[i] << " " <<
      // gold[i] << std::endl;
      count_errors++;
    }
  }

  /*
  if (count_errors) {
    std::cout << "Verification failed with " << count_errors << " errors"
              << std::endl;
  } else {
    std::cout << "Verified Successfully! " << std::endl;
  }
  */
  // std::cout << " =========================== " << std::endl;
  return count_errors;
}

template <typename T>
void evaluateGold(const SpMat<T> &mat, const std::vector<T> &denseVec,
                  std::vector<T> &result) {
  // Get the COO rep of the sparse matrix
  std::vector<float> values = mat.values();
  std::vector<int> rows = mat.rows();
  std::vector<int> cols = mat.cols();

  // Convert it to CSR
  std::vector<int> rowPtrs(mat.nrows() + 1, 0);
  std::vector<int> colIndices(mat.nnz());
  std::vector<float> vals(mat.nnz());

  coo_to_csr_scipy_v(mat.nrows(), mat.ncols(), mat.nnz(), rows, cols, values,
                     rowPtrs, colIndices, vals);

  // Run SpMV for CSR on the input full matrix (no tiling)
  spmv_ref_csr_v(rowPtrs, colIndices, vals, mat.nnz(), mat.nrows(), denseVec,
                 1.0f, 0.0f, result);
}

void evaluateGoldMKL(SpMat<float> &mat, const std::vector<float> &denseVec,
                     std::vector<float> &result) {
  std::vector<float> values = mat.values();
  std::vector<int> rows = mat.rows();
  std::vector<int> cols = mat.cols();

  std::vector<int> rowPtrs(mat.nrows() + 1, 0);
  std::vector<int> colIndices(mat.nnz());
  std::vector<float> vals(mat.nnz());

  coo_to_csr_scipy_v(mat.nrows(), mat.ncols(), mat.nnz(), rows, cols, values,
                     rowPtrs, colIndices, vals);

  sparse_matrix_t csrA;

  mkl_sparse_s_create_csr(&csrA, SPARSE_INDEX_BASE_ZERO, mat.nrows(),
                          mat.ncols(), rowPtrs.data(), rowPtrs.data() + 1,
                          colIndices.data(), values.data());

  matrix_descr descrA;
  descrA.type = SPARSE_MATRIX_TYPE_GENERAL;
  mkl_sparse_optimize(csrA);

  mkl_sparse_s_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, csrA, descrA,
                  denseVec.data(), 0.0f, result.data());

  mkl_sparse_destroy(csrA);
}

int main() {

  SpMat<float> myMat =
      SpMat<float>("../small_sparse_matrix_8x8_empty_block.mtx");

  int nrows = myMat.nrows();
  int ncols = myMat.ncols();
  int nnz = myMat.nnz();

  std::cout << "Processing a " << nrows << " x " << ncols << " matrix with "
            << nnz << " non-zeros" << std::endl;
  std::map<std::pair<int, int>, int> errorCount;

  for (int rowsPerBlock = 1; rowsPerBlock <= nrows; rowsPerBlock *= 2) {
    for (int colsPerBlock = 1; colsPerBlock <= ncols; colsPerBlock *= 2) {
      std::pair<int, int> tileConfig =
          std::make_pair(rowsPerBlock, colsPerBlock);

      if (errorCount.find(tileConfig) == errorCount.end()) {
        errorCount[tileConfig] = 0;
        std::cout << "Evaluating Tile size : " << rowsPerBlock << " x "
                  << colsPerBlock << std::endl;
      } else {
        continue;
      }

      myMat.tile(rowsPerBlock, colsPerBlock);

      std::vector<float> denseVec(myMat.ncols());
      std::vector<float> result(myMat.nrows());

      myMat.printTiles();
      fillRandom(denseVec, 0.0f, 1.0f);
      // std::cout << "Dense Vector: " << std::endl;
      // for (auto &elem: denseVec){
      //     std::cout << elem << " ";
      // }
      // std::cout << std::endl;
      myMat.multiply(denseVec, result);

      std::vector<float> ref_result(myMat.nrows(), 0);
      std::vector<float> mkl_result(myMat.nrows(), 0);

      // Comparing tiled SpMV with ref SpMV
      // std::cout << "Verifying Tiled against Ref..." << std::endl;
      evaluateGold(myMat, denseVec, ref_result);
      verify(ref_result, result, 0.001f);

      // Comparing tiled SpMV with ref MKL
      // std::cout << "Verifying tiled against ref MKL..." << std::endl;
      evaluateGoldMKL(myMat, denseVec, mkl_result);
      errorCount[tileConfig] += verify(mkl_result, result, 0.001f);

      // Comparing ref SpMV with ref MKL
      // std::cout << "Verifying ref against ref MKL..." << std::endl;
      verify(mkl_result, ref_result, 0.001f);
    }
  }
  std::cout << "rows | cols | num_errors " << std::endl;
  for (auto &pair : errorCount) {
    std::cout << pair.first.first << " | ";
    std::cout << pair.first.second << " | ";
    std::cout << pair.second << std::endl;
  }
}
