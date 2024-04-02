#include "spmat.h"
#include <iomanip>

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
                     std::vector<float> &result, TimeResults &mklTime,
                     int times = 10) {
  std::vector<float> values = mat.values();
  std::vector<int> rows = mat.rows();
  std::vector<int> cols = mat.cols();

  std::vector<int> rowPtrs(mat.nrows() + 1, 0);
  std::vector<int> colIndices(mat.nnz());
  std::vector<float> vals(mat.nnz());

  coo_to_csr_scipy_v(mat.nrows(), mat.ncols(), mat.nnz(), rows, cols, values,
                     rowPtrs, colIndices, vals);

  Timer timerMKL;
  for (int i = 0; i < times; i++) {
    timerMKL.start();
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

    timerMKL.stop();
  }
  mklTime = timerMKL.getResult();
}

int main() {

  SpMat<float> myMat =
      SpMat<float>("../small_sparse_matrix_8x8_empty_block.mtx");

  uint32_t experiments_per_config = 10;

  int nrows = myMat.nrows();
  int ncols = myMat.ncols();
  int nnz = myMat.nnz();

  std::cout << "Processing a " << nrows << " x " << ncols << " matrix with "
            << nnz << " non-zeros" << std::endl;
  std::map<std::pair<int, int>, TimeResults> perf;

  std::vector<float> mkl_result(myMat.nrows(), 0);

  // Comparing tiled SpMV with ref MKL
  // std::cout << "Verifying tiled against ref MKL..." << std::endl;
  TimeResults mklTime;
  std::vector<float> denseVec(myMat.ncols());

  fillRandom(denseVec, 0.0f, 1.0f);

  evaluateGoldMKL(myMat, denseVec, mkl_result, mklTime);

  for (int rowsPerBlock = 1; rowsPerBlock <= nrows; rowsPerBlock *= 2) {
    for (int colsPerBlock = 1; colsPerBlock <= ncols; colsPerBlock *= 2) {
      std::pair<int, int> tileConfig =
          std::make_pair(rowsPerBlock, colsPerBlock);

      std::cout << "Processing tile config " << rowsPerBlock << "x"
                << colsPerBlock << " ..." << std::endl;
      if (perf.find(tileConfig) != perf.end()) {
        continue;
      }

      myMat.tile(rowsPerBlock, colsPerBlock);

      std::vector<float> result(myMat.nrows());

      myMat.setComputeTimes(experiments_per_config);
      myMat.multiply(denseVec, result);

      perf[tileConfig] = myMat.getComputeTimeResults();

      int countErrs = verify(mkl_result, result, 0.001f);
      if (countErrs > 0) {
        std::cout << "Verification Failed with " << countErrs << " errors"
                  << std::endl;
      } else {
        std::cout << "Verified Successfully! " << std::endl;
      }
    }
  }
  std::cout << " Tiled SpMV execution time (ms) for " << myMat.getComputeTimes()
            << " runs for each tile configuration :" << std::endl;
  std::cout << " -------------------------------------" << std::endl;
  std::cout << std::left << std::setw(8) << "rows" << std::left << std::setw(8)
            << "cols" << std::left << std::setw(12) << "mean" << std::left
            << std::setw(12) << "stdev" << std::left << std::setw(12)
            << "median" << std::endl;
  for (auto &pair : perf) {
    int rowsPerTile = pair.first.first;
    int colsPerTile = pair.first.second;
    double mean = pair.second.mean;
    double stdev = pair.second.stdev;
    double median = pair.second.median;
    std::cout << std::left << std::setw(8) << rowsPerTile << std::left
              << std::setw(8) << colsPerTile << std::left << std::fixed
              << std::setprecision(5) << std::setw(12) << mean << std::left
              << std::fixed << std::setprecision(5) << std::setw(12) << stdev
              << std::left << std::fixed << std::setprecision(5)
              << std::setw(12) << median << std::endl;
  }

  std::cout << std::left << std::setw(8) << "MKL" << std::left << std::setw(8)
            << "MKL" << std::left << std::fixed << std::setprecision(5)
            << std::setw(12) << mklTime.mean << std::left << std::fixed
            << std::setprecision(5) << std::setw(12) << mklTime.stdev
            << std::left << std::fixed << std::setprecision(5) << std::setw(12)
            << mklTime.median << std::endl;
}
