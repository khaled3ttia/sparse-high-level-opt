#ifndef SPMAT_GPU_H
#define SPMAT_GPU_H

#include "spmat.h"

template <typename T, typename I=int>
class SpMatG : public SpMat {

public:
    void SpMVTiled_(const std::vector<T>& denseVec, std::vector<T>& result) override;
    void SpMV_(const CSRMatrix<T,I>& tile, const std::vector<T>& denseVec, std::vector<T>& result, int colOffset=0) override;
}


#endif
