//
// Created by 30959 on 4/10/2024.
//

#ifndef PROJECT3_OPERATION_H
#define PROJECT3_OPERATION_H

#include <stdbool.h>

typedef struct {
    size_t row;
    size_t col;
    float * data;
} Matrix;

void freeMat(Matrix ** matrix);

Matrix * initMat(size_t row, size_t col);

void checkAcc();

void merge_memcpy(const Matrix *C1, const Matrix *C2, const Matrix *C3, const Matrix *C4, Matrix *dest);

void splits_memcpy(const Matrix *A, const Matrix *B, Matrix *A1, Matrix *A2, Matrix *A3, Matrix *A4,
                   Matrix *B1, Matrix *B2, Matrix *B3, Matrix *B4);

bool addMat(const Matrix * A, const Matrix * B, Matrix * C);

bool subMat(const Matrix * A, const Matrix * B, Matrix * C);

bool matmul_rck(const Matrix * A, const Matrix * B, Matrix * C);

bool matmul_rck_trans(const Matrix * A, const Matrix * B, Matrix * C);

bool matmul_rck_AVX(const Matrix * A, const Matrix * B, Matrix * C);

bool matmul_rck_AVX_unroll(const Matrix * A, const Matrix * B, Matrix * C);

bool matmul_rck_AVX_unroll_blocked(const Matrix * A, const Matrix * B, Matrix * C);

bool matmul_rkc(const Matrix * A, const Matrix * B, Matrix * C);

bool matmul_rkc_blocked(const Matrix * A, const Matrix * B, Matrix * C);

bool matmul_rkc_blocked_AVX(const Matrix * A, const Matrix * B, Matrix * C);

bool matmul_improved(const Matrix * A, const Matrix * B, Matrix * C);

bool matmul_OpenBLAS(const Matrix * A, const Matrix * B, Matrix * C);

void fillMat(Matrix * matrix);

#endif //PROJECT3_OPERATION_H
