#include "spmat.h"

void verify(std::vector<float> gold, std::vector<float> actual, float max_err) {
  if (gold.size() != actual.size()) {
    std::cout << "Sizes do not match. Verification failed!" << std::endl;
    return;
  }

  int count_errors = 0;
  for (int i = 0; i < actual.size(); ++i) {
    if (abs(actual[i] - gold[i]) > max_err) {
      count_errors++;
    }
  }

  if (count_errors) {
    std::cout << "Verification failed with " << count_errors << " errors"
              << std::endl;
  } else {
    std::cout << "Verified Successfully! " << std::endl;
  }
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

int main() {

  SpMat<float> myMat =
      SpMat<float>("../small_sparse_matrix_8x8_empty_block.mtx");
  myMat.tile(4, 4);

  std::vector<float> denseVec(myMat.ncols());
  std::vector<float> result(myMat.nrows());

  myMat.printTiles();
  fillRandom(denseVec, 0.0f, 1.0f);
  myMat.multiply(denseVec, result);

  std::vector<float> ref_result(myMat.nrows(), 0);
  evaluateGold(myMat, denseVec, ref_result);
  verify(ref_result, result, 0.001f);

}
