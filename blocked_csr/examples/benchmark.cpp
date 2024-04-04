#include "spmat.h"
#include <fstream>
#include <iomanip>
#include <filesystem>

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

void usage() {
  std::cout << "Options: " << std::endl;
  std::cout << "-i <inputfile> : file path for a mtx file" << std::endl;
  std::cout << "-o <outdir> : output directory for csv files" << std::endl;
  std::cout << "-n <num_of_experiments> : number of SpMV experiments per "
               "configuration"
            << std::endl;
  std::cout << "-w <min_rows_per_tile> : minmum number of rows per tile (default = 1)"
            << std::endl;
 
  std::cout << "-r <max_rows_per_tile> : maximum number of rows per tile (default = number of rows)"
            << std::endl;
  std::cout << "-u <min_cols_per_tile> : minimum number of columns per tile (default = 1)"
            << std::endl;
 
  std::cout << "-c <max_cols_per_tile> : maximum number of columns per tile (default = number of columns)"
            << std::endl;
  std::cout << "-h : prints usage information" << std::endl;
  exit(0);
}

int main(int argc, char **argv) {

  int max_row = 0;
  int max_col = 0;
  int min_row = 0;
  int min_col = 0; 
  int opt;
  std::string filepath = "../small_sparse_matrix_8x8_empty_block.mtx";
  std::string outdir = "./results";
  uint32_t experiments_per_config = 10;
  while ((opt = getopt(argc, argv, "i:o:n:r:w:c:u:h")) != -1) {
    switch (opt) {
    case 'i':
      filepath = optarg;
      break;
    case 'o':
      outdir = optarg;
      break;
    case 'n':
      try {
        experiments_per_config = atoi(optarg);

      } catch (...) {
        std::cout << "Invalid number of experiments" << std::endl;
        usage();
      }
      break;
    case 'r':
      try {
        max_row = atoi(optarg);

      } catch (...) {
        std::cout << "Invalid maximum number of rows per tile" << std::endl;
        usage();
      }
      break;
    case 'w':
      try {
          min_row = atoi(optarg);
          
      } catch (...) {
          std::cout << "Invalid minimum number of rows per tile" << std::endl;
          usage();
      }
      break;
    case 'c':
      try {
        max_col = atoi(optarg);
      } catch (...) {
        std::cout << "Invalid maximum number of columns per tile" << std::endl;
        usage();
      }
      break;
    case 'u':
      try{
          min_col = atoi(optarg);
      } catch (...) {
          std::cout << "Invalid minimum number of columns per tile" << std::endl;
          usage();
      }
      break;
    case 'h':
      usage();
      break;
    }
  }
  
  std::size_t lastSlashPos = filepath.find_last_of("/\\");
  std::string filename = (lastSlashPos == std::string::npos) ? filepath : filepath.substr(lastSlashPos + 1);
  std::filesystem::path outdirPath(outdir);
  if (!std::filesystem::exists(outdirPath)){
      std::filesystem::create_directories(outdirPath);
  }
  std::ofstream outFile(outdir + "/" +filename + ".csv", std::ios::app);
  if (!outFile) {
    std::cerr << "Error could not open file " << filename << ".csv for writing."
              << std::endl;
    exit(-1);
  }
  outFile << "rows,cols,preprocess,mean,stdev,median,exitcode\n";

  SpMat<float> myMat = SpMat<float>(filepath);

  myMat.setComputeTimes(experiments_per_config);
  int nrows = myMat.nrows();
  int ncols = myMat.ncols();
  int nnz = myMat.nnz();

  std::cout << "Input Matrix : " << filename << std::endl;
  std::cout << nrows << " x " << ncols << " matrix with " << nnz << " non-zeros"
            << std::endl;
  std::map<std::pair<int, int>, TimeResults> perf;

  std::vector<float> mkl_result(myMat.nrows(), 0);

  // Comparing tiled SpMV with ref MKL
  // std::cout << "Verifying tiled against ref MKL..." << std::endl;
  TimeResults mklTime;
  std::vector<float> denseVec(myMat.ncols());

  fillRandom(denseVec, 0.0f, 1.0f);

  evaluateGoldMKL(myMat, denseVec, mkl_result, mklTime);

  outFile << "0,0,0," << std::fixed << std::setprecision(5) << mklTime.mean << ","
          << std::fixed << std::setprecision(5) << mklTime.stdev << ","
          << std::fixed << std::setprecision(5) << mklTime.median << ","
          << "0\n";

  if (max_row != 0) {
    max_row = std::min(max_row, nrows);
  } else {
    max_row = nrows;
  }
  if (max_col != 0) {
    max_col = std::min(max_col, ncols);
  } else {
    max_col = ncols;
  }

  min_col = std::max(min_col, 1);
  min_row = std::max(min_row, 1);


  for (int rowsPerBlock = min_row; rowsPerBlock <= max_row; rowsPerBlock *= 2) {
    for (int colsPerBlock = min_col; colsPerBlock <= max_col; colsPerBlock *= 2) {
      std::pair<int, int> tileConfig =
          std::make_pair(rowsPerBlock, colsPerBlock);

      std::cout << "Processing tile config " << rowsPerBlock << "x"
                << colsPerBlock << " ..." << std::endl;
      if (perf.find(tileConfig) != perf.end()) {
        continue;
      }

      myMat.tile(rowsPerBlock, colsPerBlock);

      std::vector<float> result(myMat.nrows());

      myMat.multiply(denseVec, result);

      TimeResults tileTime = myMat.getComputeTimeResults();
      TimeResults preTime = myMat.getPreprocessTimeResults();

      int countErrs = verify(mkl_result, result, 0.001f);
      int exitcode = 0;
      if (countErrs > 0) {
        exitcode = 1;
        std::cout << "Verification Failed with " << countErrs << " errors"
                  << std::endl;
      } else {
        std::cout << "Verified Successfully! " << std::endl;
      }
      outFile << rowsPerBlock << "," << colsPerBlock << "," 
          << std::fixed << std::setprecision(5) << preTime.mean << ","
          << std::fixed
              << std::setprecision(5) << tileTime.mean << "," << std::fixed
              << std::setprecision(5) << tileTime.stdev << "," << std::fixed
              << std::setprecision(5) << tileTime.median << "," << exitcode
              << "\n";
    }
  }
  outFile.close();
}
