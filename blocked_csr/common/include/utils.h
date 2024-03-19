// By: Khaled Abdelaal
// khaled.abdelaal@ou.edu
// December 2020

// Utility to load MTX files and find the full sparsity pattern as a plot
#include <algorithm>
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <unistd.h>


template <typename T> struct triplet {
  int row;
  int col;
  T val;
  inline bool operator<(const triplet &other) {
    if (row != other.row)
      return row < other.row;
    return col < other.col;
  }
};
// functions prototypes
int flatIndex(int, int, int, int);

template <typename T>
void readMTX(const std::string& filename, std::vector<int>& rows, std::vector<int>& cols, std::vector<T>& values, int* numRows, int* numCols, int* nnz, bool verbose){

  if (verbose)
  std::cout << "Loading file " << filename << "....." << std::endl;

  int numValues = 0;
  int currentLine = 0;
  int currentIndex = 0;

  std::vector<triplet<T>> entries;
  std::string line;
  std::ifstream mtx_file(filename);

  std::getline(mtx_file, line);
  std::istringstream iss(line);
  std::string id, object, format, field, symmetry;

  if (!(iss >> id >> object >> format >> field >> symmetry)) {
    std::cout << "Error reading the first line!" << std::endl;
    exit(1);
  }
  bool symmetric = false;
  bool pattern = false;
  if (symmetry == "symmetric") {
    symmetric = true;
  }

  if (field == "pattern") {
    pattern = true;
  }

  while (std::getline(mtx_file, line)) {
    if (line[0] != '%')
      break;
  }

  iss = std::istringstream(line);
  if (!(iss >> *numRows >> *numCols >> *nnz)) {
    std::cout << "Error reading rows, cols, and nnz" << std::endl;
    exit(1);
  }

  if (symmetric) {
    *nnz *= 2;
  }
  while (std::getline(mtx_file, line)) {

    int rowIdx, colIdx;
    T value;
    iss = std::istringstream(line);
    if (pattern) {
      if (!(iss >> rowIdx >> colIdx))
        break;
      value = (T)(0.012 + rand() / (RAND_MAX + 1.0));
    } else {
      if (!(iss >> rowIdx >> colIdx >> value))
        break;
    }
    struct triplet<T> t = {
      rowIdx - 1, colIdx - 1, value
    };
    entries.push_back(t);
    currentIndex++;

    if (symmetric && t.row != t.col) {
      struct triplet<T> t_mirror = {
        t.col, t.row, t.val
      };
      entries.push_back(t_mirror);
      currentIndex++;
    }
  }
  *nnz = currentIndex;

  if (verbose) {
    std::cout << "rows: " << *numRows << ", cols: " << *numCols << ", nnz: " << *nnz
              << std::endl;
  }

  std::sort(entries.begin(), entries.end());

  //nnzRowIdx = new int[*nnz];
  //nnzColIdx = new int[*nnz];
  //nnzVal = new T[*nnz];

  for (int i = 0; i < entries.size(); i++) {
    rows.emplace_back(entries[i].row);
    cols.emplace_back(entries[i].col);
    values.emplace_back(entries[i].val);
  }

  mtx_file.close();


}


// This function takes a MTX file as an input and loads it in the Coordinate
// format
//  arguments:
//	*&nnzRowIdx : (in-out) an integer pointer reference to the output COO
//row indices array
//	*&nnzColIdx : (in-out) an integer pointer reference to the output COO
//col indices array
//	*&nnzVal    : (in-out) a float pointer reference to the output COO float
//values array 	*nnz 	    : (in-out) an integer pointer to the number of
//non-zeros in the matrix 	*nrows 	    : (in-out) an integer pointer to the
//number of rows 	*ncols 	    : (in-out) an integer pointer to the number
//of columns 	filename    : (in)     a string representing filename (file path)
//   verbose     : (in)     a boolean to enable/disable printing stdout messages
//
template <typename T>
void loadMTX(int *&nnzRowIdx, int *&nnzColIdx, T *&nnzVal, int *nnz, int *nrows,
             int *ncols, std::string filename, bool verbose) {

  if (verbose)
  std::cout << "Loading file " << filename << "....." << std::endl;

  int numValues = 0;
  int currentLine = 0;
  int currentIndex = 0;

  std::vector<triplet<T>> entries;
  std::string line;
  std::ifstream mtx_file(filename);

  std::getline(mtx_file, line);
  std::istringstream iss(line);
  std::string id, object, format, field, symmetry;

  if (!(iss >> id >> object >> format >> field >> symmetry)) {
    std::cout << "Error reading the first line!" << std::endl;
    exit(1);
  }
  bool symmetric = false;
  bool pattern = false;
  if (symmetry == "symmetric") {
    symmetric = true;
  }

  if (field == "pattern") {
    pattern = true;
  }

  while (std::getline(mtx_file, line)) {
    if (line[0] != '%')
      break;
  }

  iss = std::istringstream(line);
  if (!(iss >> *nrows >> *ncols >> *nnz)) {
    std::cout << "Error reading rows, cols, and nnz" << std::endl;
    exit(1);
  }

  if (symmetric) {
    *nnz *= 2;
  }
  while (std::getline(mtx_file, line)) {

    int rowIdx, colIdx;
    T value;
    iss = std::istringstream(line);
    if (pattern) {
      if (!(iss >> rowIdx >> colIdx))
        break;
      value = (T)(0.012 + rand() / (RAND_MAX + 1.0));
    } else {
      if (!(iss >> rowIdx >> colIdx >> value))
        break;
    }
    struct triplet<T> t = {
      rowIdx - 1, colIdx - 1, value
    };
    entries.push_back(t);
    currentIndex++;

    if (symmetric && t.row != t.col) {
      struct triplet<T> t_mirror = {
        t.col, t.row, t.val
      };
      entries.push_back(t_mirror);
      currentIndex++;
    }
  }
  *nnz = currentIndex;

  if (verbose) {
    std::cout << "rows: " << *nrows << ", cols: " << *ncols << ", nnz: " << *nnz
              << std::endl;
  }

  std::sort(entries.begin(), entries.end());

  nnzRowIdx = new int[*nnz];
  nnzColIdx = new int[*nnz];
  nnzVal = new T[*nnz];

  for (int i = 0; i < entries.size(); i++) {
    nnzRowIdx[i] = entries[i].row;
    nnzColIdx[i] = entries[i].col;
    nnzVal[i] = entries[i].val;
  }

  mtx_file.close();
}

void coo_to_csr(int *&nnzRowIdx, const int nnz, const int nrows, int *&rowPtr){
  rowPtr = new int[nrows+1];
  std::fill(rowPtr, rowPtr + nrows, -1);
  rowPtr[0] = 0;
  rowPtr[nrows] = nnz;

  int x = 0; 
  for (int i = 0; i < nnz; i++){
      while (nnzRowIdx[i] != x){
          rowPtr[++x] = i;
      }
  }
 
  // if used rows < total rows, repeat last value 
  for (int i = x+1 ; i < nrows; i++){
      rowPtr[i] = nnz;
  }
  
}
// SciPy's implementation of coo to csr conversion
// left here for reference and comparison
// original version at: https://github.com/scipy/scipy/blob/3b36a57/scipy/sparse/sparsetools/coo.h#L34
template <typename T>
void coo_to_csr_scipy(const int n, const int m, const int nnz, int *&nnzRowIdx, int *&nnzColIdx, T *&nnzVal, int *&Bp, int *&Bj, T *&Bx){
    Bp = new int[n+1];
    Bj = new int[nnz];
    Bx = new T[nnz];

    std::fill(Bp, Bp+n, 0);
    for (int i = 0; i < nnz; i++){
        Bp[nnzRowIdx[i]]++;
    }
    for (int i =0, cumsum=0; i<n; i++){
        int temp = Bp[i];
        Bp[i] = cumsum;
        cumsum += temp;
    }
    Bp[n] = nnz;

    for (int i =0; i < nnz; i++){
        int row = nnzRowIdx[i];
        int dest = Bp[row];

        Bj[dest] = nnzColIdx[i];
        Bx[dest] = nnzVal[i];

        Bp[row]++;
    }

    for (int i =0, last=0; i <= n; i++){
        int temp = Bp[i];
        Bp[i] = last;
        last = temp;
    }
}


template <typename T>
void coo_to_csc(const int n, const int m, const int nnz, int *&nnzRowIdx, int *&nnzColIdx, T *&nnzVal, int *&Bp, int *&Bi, T *&Bx){
    coo_to_csr_scipy<T>(m, n, nnz, nnzColIdx, nnzRowIdx, nnzVal, Bp, Bi, Bx);
}



template <typename T> void print(T *x, const int size, std::string name = " ") {
  if (name != " ") {
    std::cout << name << " : ";
  }
  for (int i = 0; i < size; i++) {
    std::cout << x[i] << " ";
  }
  std::cout << std::endl;
}

template <typename T> void fill(T *x, const int n, const float maxi) {

  for (int i = 0; i < n; i++) {

    x[i] = ((T)maxi * (rand() / (RAND_MAX + 1.0f)));
  }
}

template <typename T>
void spmv_ref_coo(int *&nnzRowIdx, int *&nnzColIdx, T *&nnzVal, int *nnz,
                  int *nrows, T *&x, T alpha, T beta, T *&ref) {
  ref = new T[*nrows];
  std::fill(ref, ref + (*nrows), 0);
  for (int i = 0; i < *nnz; i++) {

    int row = nnzRowIdx[i];
    int col = nnzColIdx[i];
    ref[row] += alpha * nnzVal[i] * x[col] + beta * ref[row];
  }
}


template <typename T>
void spmv_ref_csr(int *&rowPtr, int *&nnzColIdx, T *&nnzVal, const int nnz, const int nrows, T *&x, T alpha, T beta, T *&ref){
    ref = new T[nrows];
    std::fill(ref, ref+(nrows),0);
    for (int i = 0; i < nrows; i++) {
        int firstValIdx = rowPtr[i];
        int lastValIdx = rowPtr[i+1];
        while (firstValIdx < lastValIdx){
            ref[i] += alpha * nnzVal[firstValIdx] * x[nnzColIdx[firstValIdx]] + beta * ref[i];
            firstValIdx++;
        }
    }
}
void cooToDense(int *&nnzRowIdx, int *&nnzColIdx, float *&nnzVal, int nnz,
                int nrows, int ncols, float *&denseMatrix) {

  // denseMatrix = {0};
  for (int i = 0; i < nnz; i++) {
    int row = nnzRowIdx[i];
    int col = nnzColIdx[i];
    int idx = flatIndex(row, col, nrows, ncols);
    denseMatrix[idx] = nnzVal[i];
  }
}

int flatIndex(int i, int j, int nrows, int ncols) {
  if (i >= 0 && i < nrows) {
    if (j >= 0 && j < ncols) {
      return i * ncols + j;
    }
  }

  return -1;
}

void unpackIndex(int index, int ncols, int &row, int &col) {
  row = (int)(index / ncols);
  col = (int)(index % ncols);
}
