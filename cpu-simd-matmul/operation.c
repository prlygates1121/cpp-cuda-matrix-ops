//
// Created by 30959 on 4/10/2024.
//

#include <immintrin.h>
#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <cblas.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "operation.h"

#define CHECK(op) if (A == NULL || B == NULL || C == NULL) { \
    fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to perform %s due to a NULL input (Matrix).\n",  __LINE__, __FUNCTION__, op); \
    return false;} \
    if (A->data == NULL || B->data == NULL || C->data == NULL) { \
    fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to perform %s due to a NULL input (Matrix data).\n",  __LINE__, __FUNCTION__, op); \
    return false;} \
    if (!(A->row == B->row && A->row == C->row && A->col == B->col && A->col == C->col)) { \
    fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to perform %s because the matrices are not equal in SIZE.\n",  __LINE__, __FUNCTION__, op); \
    return false;}

#define C(i, j) C->data[i * B->col + j]
#define A(i, j) A->data[i * A->col + j]
#define B(i, j) B->data[i * B->col + j]

#define BLOCK_SIZE 64

// frees the memory of a matrix struct and set its pointer to NULL if it has not been freed before
void freeMat(Matrix ** matrix) {
    if (*matrix == NULL) { // check if the pointer to struct Matrix is NULL, which means it has been freed before
        return;
    }
    if ((*matrix)->data == NULL) {
        free(*matrix);
        *matrix = NULL;
        return;
    }
    free((*matrix)->data);
    free(*matrix);
    *matrix = NULL;
}

// allocate memory for a square matrix, set its length and set all its data to zero
Matrix * initMat(size_t row, size_t col) {
    if (row == 0 || col == 0) {
        fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to initialize a matrix struct due to invalid matrix SIZE.\n",  __LINE__, __FUNCTION__);
        return NULL;
    }
    Matrix * newMatrix = (Matrix *)malloc(sizeof(Matrix));
    if (newMatrix == NULL) {
        fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to initialize a matrix struct due to memory allocation failure (for matrix struct).\n",  __LINE__, __FUNCTION__);
        return NULL;
    }
    newMatrix->row = row;
    newMatrix->col = col;
    // memory allocated for matrix data is contiguous
    newMatrix->data = (float *)calloc(row * col, sizeof(float));
    if (newMatrix->data == NULL) {
        fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to initialize a matrix struct due to memory allocation failure (for matrix data).\n",  __LINE__, __FUNCTION__);
        free(newMatrix);
        return NULL;
    }

    return newMatrix;
}

// fill the matrix with random values
void fillMat(Matrix * matrix) {
    for (size_t i = 0; i < matrix->col * matrix->row; i++) {
        matrix->data[i] = (float)((double)rand() / (double)RAND_MAX - (float)1 / 2) * 10; // NOLINT(*-msc50-cpp)
    }
}

// display the matrix
void showMat(Matrix * matrix) {
    for (size_t i = 0; i < matrix->col * matrix->row; i++) {
        printf("%f ", matrix->data[i]);
        if ((i + 1) % matrix->col == 0) {
            printf("\n");
        }
    }
}

// compare the results of each implementation method with the naive method, and display the average error
void checkAcc() {
    int size = 128;
    Matrix *A = initMat(size, size);
    Matrix *B = initMat(size, size);
    if (A == NULL || B == NULL) {
        fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to perform checkAcc due to a NULL input (Matrix).\n",  __LINE__, __FUNCTION__);
        freeMat(&A);
        freeMat(&B);
        return;
    }
    fillMat(A);
    fillMat(B);

    Matrix *C_rck = initMat(size, size);
    Matrix *C_rck_trans = initMat(size, size);
    Matrix *C_rck_intrinsics = initMat(size, size);
    Matrix *C_rck_intrinsics_unroll = initMat(size, size);
    Matrix *C_rck_intrinsics_unroll_blocked = initMat(size, size);
    Matrix *C_rkc = initMat(size, size);
    Matrix *C_rkc_intrinsics = initMat(size, size);
    Matrix *C_rkc_intrinsics_unroll = initMat(size, size);
    Matrix *C_rkc_intrinsics_unroll_blocked = initMat(size, size);
    Matrix *C_rkc_blocked = initMat(size, size);
    Matrix *C_rkc_blocked_omp = initMat(size, size);
    Matrix *C_improved = initMat(size, size);
    Matrix *C_OpenBLAS = initMat(size, size);

    if (C_rck == NULL || C_rck_trans == NULL || C_rck_intrinsics == NULL || C_rck_intrinsics_unroll == NULL || C_rck_intrinsics_unroll_blocked == NULL ||
        C_rkc == NULL || C_rkc_intrinsics == NULL || C_rkc_intrinsics_unroll == NULL || C_rkc_blocked == NULL || C_rkc_blocked_omp == NULL || C_rkc_intrinsics_unroll_blocked == NULL ||
        C_improved == NULL || C_OpenBLAS == NULL ) {
        fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to perform checkAcc due to memory allocation failure (for result matrices).\n",  __LINE__, __FUNCTION__);
        freeMat(&A);
        freeMat(&B);
        freeMat(&C_rck);
        freeMat(&C_rck_trans);
        freeMat(&C_rck_intrinsics);
        freeMat(&C_rck_intrinsics_unroll);
        freeMat(&C_rck_intrinsics_unroll_blocked);
        freeMat(&C_rkc);
        freeMat(&C_rkc_intrinsics);
        freeMat(&C_rkc_intrinsics_unroll);
        freeMat(&C_rkc_blocked);
        freeMat(&C_rkc_blocked_omp);
        freeMat(&C_rkc_intrinsics_unroll_blocked);
        freeMat(&C_improved);
        freeMat(&C_OpenBLAS);
        return;
    }

    matmul_rck(A, B, C_rck);
    matmul_rck_trans(A, B, C_rck_trans);
    matmul_rck_intrinsics(A, B, C_rck_intrinsics);
    matmul_rck_intrinsics_unroll(A, B, C_rck_intrinsics_unroll);
    matmul_rck_intrinsics_unroll_blocked(A, B, C_rck_intrinsics_unroll_blocked);
    matmul_rkc(A, B, C_rkc);
    matmul_rkc_intrinsics(A, B, C_rkc_intrinsics);
    matmul_rkc_intrinsics_unroll(A, B, C_rkc_intrinsics_unroll);
    matmul_rkc_blocked(A, B, C_rkc_blocked);
    matmul_rkc_blocked_omp(A, B, C_rkc_blocked_omp);
    matmul_rkc_intrinsics_unroll_blocked(A, B, C_rkc_intrinsics_unroll_blocked);
    matmul_improved(A, B, C_improved);
    matmul_OpenBLAS(A, B, C_OpenBLAS);

    double error[20] = {0};
    for (size_t i = 0; i < size * size; i++) {
        error[0] += fabsf(C_rck->data[i] - C_rck_trans->data[i]);
        error[1] += fabsf(C_rck->data[i] - C_rck_intrinsics->data[i]);
        error[2] += fabsf(C_rck->data[i] - C_rck_intrinsics_unroll->data[i]);
        error[3] += fabsf(C_rck->data[i] - C_rck_intrinsics_unroll_blocked->data[i]);

        error[4] += fabsf(C_rck->data[i] - C_rkc->data[i]);
        error[5] += fabsf(C_rck->data[i] - C_rkc_intrinsics->data[i]);
        error[6] += fabsf(C_rck->data[i] - C_rkc_intrinsics_unroll->data[i]);
        error[7] += fabsf(C_rck->data[i] - C_rkc_intrinsics_unroll_blocked->data[i]);

        error[8] += fabsf(C_rck->data[i] - C_rkc_blocked->data[i]);
        error[9] += fabsf(C_rck->data[i] - C_rkc_blocked_omp->data[i]);

        error[10] += fabsf(C_rck->data[i] - C_improved->data[i]);
        error[11] += fabsf(C_rck->data[i] - C_OpenBLAS->data[i]);
    }

    printf("                                      Average error\n");
    printf("matmul_rck_trans                        %f\n", error[0] / (size * size));
    printf("matmul_rck_intrinsics                   %f\n", error[1] / (size * size));
    printf("matmul_rck_intrinsics_unroll            %f\n", error[2] / (size * size));
    printf("matmul_rck_intrinsics_unroll_blocked    %f\n", error[3] / (size * size));

    printf("matmul_rkc                              %f\n", error[4] / (size * size));
    printf("matmul_rkc_intrinsics                   %f\n", error[5] / (size * size));
    printf("matmul_rkc_intrinsics_unroll            %f\n", error[6] / (size * size));
    printf("matmul_rkc_intrinsics_unroll_blocked    %f\n", error[7] / (size * size));

    printf("matmul_rkc_blocked                      %f\n", error[8] / (size * size));
    printf("matmul_rkc_blocked_omp                  %f\n", error[9] / (size * size));

    printf("matmul_improved                         %f\n", error[10] / (size * size));
    printf("matmul_OpenBLAS                         %f\n", error[11] / (size * size));

    freeMat(&A);
    freeMat(&B);
    freeMat(&C_rck);
    freeMat(&C_rck_trans);
    freeMat(&C_rck_intrinsics);
    freeMat(&C_rck_intrinsics_unroll);
    freeMat(&C_rck_intrinsics_unroll_blocked);

    freeMat(&C_rkc);
    freeMat(&C_rkc_intrinsics);
    freeMat(&C_rkc_intrinsics_unroll);
    freeMat(&C_rkc_intrinsics_unroll_blocked);

    freeMat(&C_rkc_blocked);
    freeMat(&C_rkc_blocked_omp);
    freeMat(&C_improved);
    freeMat(&C_OpenBLAS);
}

// merge four sub-matrices into a complete matrix using memcpy
void merge_memcpy(const Matrix *C1, const Matrix *C2, const Matrix *C3, const Matrix *C4, Matrix *dest) {
    size_t rowCenterPosition = dest->col * C1->row; // rowCenterPosition locates the first element of the first row in the lower half of the main matrix
    for (size_t r = 0; r < C1->row; r++) { // each loop deals with a single row
        size_t subMatrixRowsIndex = C1->col * r;
        size_t mainMatrixRowsIndex = dest->col * r;
        memcpy(&((dest->data)[mainMatrixRowsIndex]), &((C1->data)[subMatrixRowsIndex]), C1->col * sizeof(float));
        memcpy(&((dest->data)[mainMatrixRowsIndex + C1->col]), &((C2->data)[subMatrixRowsIndex]), C1->col * sizeof(float));
        memcpy(&((dest->data)[mainMatrixRowsIndex + rowCenterPosition]), &((C3->data)[subMatrixRowsIndex]), C1->col * sizeof(float));
        memcpy(&((dest->data)[mainMatrixRowsIndex + rowCenterPosition + C1->col]), &((C4->data)[subMatrixRowsIndex]), C1->col * sizeof(float));
    }
}

// split A and B respectively into four sub-matrices using memcpy
void splits_memcpy(const Matrix *A, const Matrix *B, Matrix *A1, Matrix *A2, Matrix *A3, Matrix *A4, Matrix *B1, Matrix *B2, Matrix *B3, Matrix *B4) {
    size_t rowCenterPosition = A->col * A1->row; // rowCenterPosition locates the first element of the first row in the lower half of the main matrix
    size_t subMatrixRowsIndex = 0;
    size_t mainMatrixRowsIndex = 0;
    size_t A1ColBytes = A1->col * sizeof(float);
    size_t B1ColBytes = B1->col * sizeof(float);
    for (size_t r = 0; r < A1->row; r++) { // each loop deals with a single row
        memcpy(&((A1->data)[subMatrixRowsIndex]), &((A->data)[mainMatrixRowsIndex]), A1ColBytes);
        memcpy(&((A2->data)[subMatrixRowsIndex]), &((A->data)[mainMatrixRowsIndex + A1->col]), A1ColBytes);
        memcpy(&((A3->data)[subMatrixRowsIndex]), &((A->data)[mainMatrixRowsIndex + rowCenterPosition]), A1ColBytes);
        memcpy(&((A4->data)[subMatrixRowsIndex]), &((A->data)[mainMatrixRowsIndex + rowCenterPosition + A1->col]), A1ColBytes);
        subMatrixRowsIndex += A1->col;
        mainMatrixRowsIndex += A->col;
    }
    subMatrixRowsIndex = 0;
    mainMatrixRowsIndex = 0;
    for (size_t r = 0; r < B1->row; r++) { // each loop deals with a single row
        memcpy(&((B1->data)[subMatrixRowsIndex]), &((B->data)[mainMatrixRowsIndex]), B1ColBytes);
        memcpy(&((B2->data)[subMatrixRowsIndex]), &((B->data)[mainMatrixRowsIndex + B1->col]), B1ColBytes);
        memcpy(&((B3->data)[subMatrixRowsIndex]), &((B->data)[mainMatrixRowsIndex + rowCenterPosition]), B1ColBytes);
        memcpy(&((B4->data)[subMatrixRowsIndex]), &((B->data)[mainMatrixRowsIndex + rowCenterPosition + B1->col]), B1ColBytes);
        subMatrixRowsIndex += B1->col;
        mainMatrixRowsIndex += B->col;
    }
}

bool addMat(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("addition")
    float * rowA = A->data;
    float * rowB = B->data;
    float * rowRes = C->data;
    for (size_t r = 0; r < A->row * A->col; r++) {
        *(rowRes++) = *(rowA++) + *(rowB++);
    }
    return true;
}

bool subMat(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("subtraction")
    float * rowA = A->data;
    float * rowB = B->data;
    float * rowRes = C->data;
    for (size_t r = 0; r < A->row * A->col; r++) {
        *(rowRes++) = *(rowA++) - *(rowB++);
    }
    return true;
}

bool matmul_rck(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
    // multiply A and B to get C
    for (size_t r = 0; r < A->row; r++) {
        for (size_t c = 0; c < B->col; c++) {
            for (size_t k = 0; k < A->col; k++) {
                C(r, c) += A(r, k) * B(k, c);
            }
        }
    }
    return true;
}

bool matmul_rck_trans(const Matrix * A, const Matrix * B, Matrix * C) {
    Matrix *BT = initMat(B->col, B->row);
    if (BT == NULL) {
        fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to perform multiplication due to memory allocation failure (for BT).\n",  __LINE__, __FUNCTION__);
        return false;
    }
    CHECK("multiplication")

    // transpose matrix B
    for (size_t r = 0; r < B->row; r++) {
        for (size_t c = 0; c < B->col; c++) {
            BT->data[c * B->col + r] = B->data[r * B->col + c];
        }
    }

    // multiply A and BT to get C
    for (size_t r = 0; r < A->row; r++) {
        for (size_t c = 0; c < B->col; c++) {
            for (size_t k = 0; k < A->col; k++) {
                C->data[r * B->col + c] += A->data[r * A->col + k] * BT->data[c * B->col + k];
            }
        }
    }
    freeMat(&BT);
    return true;
}

bool matmul_rck_intrinsics(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
    // multiply A and B to get C
    for (size_t r = 0; r < A->row; r++) {
        for (size_t c = 0; c < B->col; c += 8) {
            __m256 sum_vec = _mm256_setzero_ps();
            for (size_t k = 0; k < A->col; k++) {
//                sum_vec = _mm256_add_ps(sum_vec,
//                                        _mm256_mul_ps(_mm256_loadu_ps(&B(k, c)),
//                                                      _mm256_broadcast_ss(&A(r, k))));
                sum_vec = _mm256_fmadd_ps(_mm256_loadu_ps(&B(k, c)), _mm256_broadcast_ss(&A(r, k)), sum_vec);
                // C(r, c) += A(r, k) * B(k, c);
            }
            _mm256_storeu_ps(&C(r, c), sum_vec);
        }
    }
    return true;
}

bool matmul_rck_intrinsics_unroll(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
    // multiply A and B to get C
    for (size_t r = 0; r < A->row; r++) {
        for (size_t c = 0; c < B->col; c += 32) {
            __m256 sum_vec[4];
            sum_vec[0] = _mm256_loadu_ps(&C(r, c));
            sum_vec[1] = _mm256_loadu_ps(&C(r, c + 8));
            sum_vec[2] = _mm256_loadu_ps(&C(r, c + 16));
            sum_vec[3] = _mm256_loadu_ps(&C(r, c + 24));
            for (size_t k = 0; k < A->col; k++) {
                sum_vec[0] = _mm256_fmadd_ps(_mm256_loadu_ps(&B(k, c)), _mm256_broadcast_ss(&A(r, k)), sum_vec[0]);
                sum_vec[1] = _mm256_fmadd_ps(_mm256_loadu_ps(&B(k, c + 8)), _mm256_broadcast_ss(&A(r, k)), sum_vec[1]);
                sum_vec[2] = _mm256_fmadd_ps(_mm256_loadu_ps(&B(k, c + 16)), _mm256_broadcast_ss(&A(r, k)), sum_vec[2]);
                sum_vec[3] = _mm256_fmadd_ps(_mm256_loadu_ps(&B(k, c + 24)), _mm256_broadcast_ss(&A(r, k)), sum_vec[3]);
            }
            _mm256_storeu_ps(&C(r, c), sum_vec[0]);
            _mm256_storeu_ps(&C(r, c + 8), sum_vec[1]);
            _mm256_storeu_ps(&C(r, c + 16), sum_vec[2]);
            _mm256_storeu_ps(&C(r, c + 24), sum_vec[3]);
        }
    }
    return true;
}

bool matmul_rck_intrinsics_unroll_blocked(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
    for (size_t block_r = 0; block_r < A->row; block_r += BLOCK_SIZE) {
        for (size_t block_k = 0; block_k < A->col; block_k += BLOCK_SIZE) {
            for (size_t block_c = 0; block_c < B->col; block_c += BLOCK_SIZE) {
                for (size_t r = block_r; r < block_r + BLOCK_SIZE; r++) {
                    for (size_t c = block_c; c < block_c + BLOCK_SIZE; c += 32) {
                        __m256 sum_vec[4];
                        sum_vec[0] = _mm256_loadu_ps(&C(r, c));
                        sum_vec[1] = _mm256_loadu_ps(&C(r, c + 8));
                        sum_vec[2] = _mm256_loadu_ps(&C(r, c + 16));
                        sum_vec[3] = _mm256_loadu_ps(&C(r, c + 24));
                        for (size_t k = block_k; k < block_k + BLOCK_SIZE; k++) {
                            sum_vec[0] = _mm256_fmadd_ps(_mm256_loadu_ps(&B(k, c)), _mm256_broadcast_ss(&A(r, k)), sum_vec[0]);
                            sum_vec[1] = _mm256_fmadd_ps(_mm256_loadu_ps(&B(k, c + 8)), _mm256_broadcast_ss(&A(r, k)), sum_vec[1]);
                            sum_vec[2] = _mm256_fmadd_ps(_mm256_loadu_ps(&B(k, c + 16)), _mm256_broadcast_ss(&A(r, k)), sum_vec[2]);
                            sum_vec[3] = _mm256_fmadd_ps(_mm256_loadu_ps(&B(k, c + 24)), _mm256_broadcast_ss(&A(r, k)), sum_vec[3]);
                        }
                        _mm256_storeu_ps(&C(r, c), sum_vec[0]);
                        _mm256_storeu_ps(&C(r, c + 8), sum_vec[1]);
                        _mm256_storeu_ps(&C(r, c + 16), sum_vec[2]);
                        _mm256_storeu_ps(&C(r, c + 24), sum_vec[3]);
                    }
                }
            }
        }
    }
    return true;
}

bool matmul_rkc(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
    for (size_t r = 0; r < A->row; r++) {
        for (size_t k = 0; k < A->col; k++) {
            for (size_t c = 0; c < B->col; c++) {
                C(r, c) += A(r, k) * B(k, c);
            }
        }
    }
    return true;
}

bool matmul_rkc_intrinsics(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
    for (size_t r = 0; r < A->row; r++) {
        for (size_t k = 0; k < A->col; k++) {
            for (size_t c = 0; c < B->col; c += 8) {
                __m256 c_vec = _mm256_loadu_ps(&C(r, c));
                c_vec = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, c)), c_vec);
                _mm256_storeu_ps(&C(r, c), c_vec);
            }
            // C(r, c) += A(r, k) * B(k, c);
        }
    }
    return true;
}

bool matmul_rkc_intrinsics_unroll(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
    for (size_t r = 0; r < A->row; r++) {
        for (size_t k = 0; k < A->col; k++) {
            for (size_t c = 0; c < B->col; c += 64) {
                __m256 c_vec[8];
                c_vec[0] = _mm256_loadu_ps(&C(r, c));
                c_vec[1] = _mm256_loadu_ps(&C(r, c + 8));
                c_vec[2] = _mm256_loadu_ps(&C(r, c + 16));
                c_vec[3] = _mm256_loadu_ps(&C(r, c + 24));
                c_vec[4] = _mm256_loadu_ps(&C(r, c + 32));
                c_vec[5] = _mm256_loadu_ps(&C(r, c + 40));
                c_vec[6] = _mm256_loadu_ps(&C(r, c + 48));
                c_vec[7] = _mm256_loadu_ps(&C(r, c + 56));
                c_vec[0] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, c)), c_vec[0]);
                c_vec[1] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, c + 8)), c_vec[1]);
                c_vec[2] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, c + 16)), c_vec[2]);
                c_vec[3] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, c + 24)), c_vec[3]);
                c_vec[4] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, c + 32)), c_vec[4]);
                c_vec[5] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, c + 40)), c_vec[5]);
                c_vec[6] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, c + 48)), c_vec[6]);
                c_vec[7] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, c + 56)), c_vec[7]);
                _mm256_storeu_ps(&C(r, c), c_vec[0]);
                _mm256_storeu_ps(&C(r, c + 8), c_vec[1]);
                _mm256_storeu_ps(&C(r, c + 16), c_vec[2]);
                _mm256_storeu_ps(&C(r, c + 24), c_vec[3]);
                _mm256_storeu_ps(&C(r, c + 32), c_vec[4]);
                _mm256_storeu_ps(&C(r, c + 40), c_vec[5]);
                _mm256_storeu_ps(&C(r, c + 48), c_vec[6]);
                _mm256_storeu_ps(&C(r, c + 56), c_vec[7]);
            }
        }
    }
    return true;
}

bool matmul_rkc_blocked(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
    // separate the result matrix C into blocks, and block_r, block_k, block_c are the indexes of the blocks
    for (size_t block_r = 0; block_r < A->row; block_r += BLOCK_SIZE) {
        for (size_t block_k = 0; block_k < A->col; block_k += BLOCK_SIZE) {
            for (size_t block_c = 0; block_c < B->col; block_c += BLOCK_SIZE) {
                // compute each block of matrix C
                for (size_t r = block_r; r < block_r + BLOCK_SIZE; r++) {
                    for (size_t k = block_k; k < block_k + BLOCK_SIZE; k++) {
                        for (size_t c = block_c; c < block_c + BLOCK_SIZE; c++) {
                            C(r, c) += A(r, k) * B(k, c);
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool matmul_rkc_blocked_omp(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
#pragma omp parallel for
    for (size_t block_r = 0; block_r < A->row; block_r += BLOCK_SIZE) {
        for (size_t block_k = 0; block_k < A->col; block_k += BLOCK_SIZE) {
            for (size_t block_c = 0; block_c < B->col; block_c += BLOCK_SIZE) {
                // compute each block of matrix C
                for (size_t r = block_r; r < block_r + BLOCK_SIZE; r++) {
                    for (size_t k = block_k; k < block_k + BLOCK_SIZE; k++) {
                        for (size_t c = block_c; c < block_c + BLOCK_SIZE; c++) {
                            C(r, c) += A(r, k) * B(k, c);
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool matmul_rkc_intrinsics_unroll_blocked(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
    // separate the result matrix C into blocks, and block_r, block_k, block_c are the indexes of the blocks
    for (size_t block_r = 0; block_r < A->row; block_r += BLOCK_SIZE) {
        for (size_t block_k = 0; block_k < A->row; block_k += BLOCK_SIZE) {
            for (size_t block_c = 0; block_c < A->row; block_c += BLOCK_SIZE) {
                // compute each block of matrix C
                for (size_t r = block_r; r < block_r + BLOCK_SIZE; r++) {
                    for (size_t k = block_k; k < block_k + BLOCK_SIZE; k++) {
                        __m256 c_vec[8];
                        c_vec[0] = _mm256_loadu_ps(&C(r, block_c));
                        c_vec[1] = _mm256_loadu_ps(&C(r, block_c + 8));
                        c_vec[2] = _mm256_loadu_ps(&C(r, block_c + 16));
                        c_vec[3] = _mm256_loadu_ps(&C(r, block_c + 24));
                        c_vec[4] = _mm256_loadu_ps(&C(r, block_c + 32));
                        c_vec[5] = _mm256_loadu_ps(&C(r, block_c + 40));
                        c_vec[6] = _mm256_loadu_ps(&C(r, block_c + 48));
                        c_vec[7] = _mm256_loadu_ps(&C(r, block_c + 56));
                        c_vec[0] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, block_c)), c_vec[0]);
                        c_vec[1] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, block_c + 8)), c_vec[1]);
                        c_vec[2] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, block_c + 16)), c_vec[2]);
                        c_vec[3] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, block_c + 24)), c_vec[3]);
                        c_vec[4] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, block_c + 32)), c_vec[4]);
                        c_vec[5] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, block_c + 40)), c_vec[5]);
                        c_vec[6] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, block_c + 48)), c_vec[6]);
                        c_vec[7] = _mm256_fmadd_ps(_mm256_broadcast_ss(&A(r, k)), _mm256_loadu_ps(&B(k, block_c + 56)), c_vec[7]);
                        _mm256_storeu_ps(&C(r, block_c), c_vec[0]);
                        _mm256_storeu_ps(&C(r, block_c + 8), c_vec[1]);
                        _mm256_storeu_ps(&C(r, block_c + 16), c_vec[2]);
                        _mm256_storeu_ps(&C(r, block_c + 24), c_vec[3]);
                        _mm256_storeu_ps(&C(r, block_c + 32), c_vec[4]);
                        _mm256_storeu_ps(&C(r, block_c + 40), c_vec[5]);
                        _mm256_storeu_ps(&C(r, block_c + 48), c_vec[6]);
                        _mm256_storeu_ps(&C(r, block_c + 56), c_vec[7]);
                    }
                }
            }
        }
    }
    return true;
}

bool matmul_improved(const Matrix * A, const Matrix * B, Matrix * C) { // NOLINT(*-no-recursion)
    CHECK("multiplication")
    if (A->row <= 2048) {
        matmul_rkc_blocked_omp(A, B, C);
    } else {
        size_t subLength = A->row / 2;
        Matrix *A1 = initMat(subLength, subLength);
        Matrix *A2 = initMat(subLength, subLength);
        Matrix *A3 = initMat(subLength, subLength);
        Matrix *A4 = initMat(subLength, subLength);
        Matrix *B1 = initMat(subLength, subLength);
        Matrix *B2 = initMat(subLength, subLength);
        Matrix *B3 = initMat(subLength, subLength);
        Matrix *B4 = initMat(subLength, subLength);
        Matrix *C1 = initMat(subLength, subLength);
        Matrix *C2 = initMat(subLength, subLength);
        Matrix *C3 = initMat(subLength, subLength);
        Matrix *C4 = initMat(subLength, subLength);
        Matrix *P1 = initMat(subLength, subLength);
        Matrix *P2 = initMat(subLength, subLength);
        Matrix *P3 = initMat(subLength, subLength);
        Matrix *P4 = initMat(subLength, subLength);
        Matrix *P5 = initMat(subLength, subLength);
        Matrix *P6 = initMat(subLength, subLength);
        Matrix *P7 = initMat(subLength, subLength);
        Matrix *add1 = initMat(subLength, subLength);
        Matrix *add2 = initMat(subLength, subLength);
        Matrix *add3 = initMat(subLength, subLength);
        Matrix *add4 = initMat(subLength, subLength);
        Matrix *add5 = initMat(subLength, subLength);
        Matrix *add6 = initMat(subLength, subLength);
        Matrix *add7 = initMat(subLength, subLength);
        Matrix *add8 = initMat(subLength, subLength);
        Matrix *add9 = initMat(subLength, subLength);
        Matrix *sub1 = initMat(subLength, subLength);
        Matrix *sub2 = initMat(subLength, subLength);
        Matrix *sub3 = initMat(subLength, subLength);
        Matrix *sub4 = initMat(subLength, subLength);
        Matrix *sub5 = initMat(subLength, subLength);

        // security checks for the initialization of sub-matrices
        if (A1 == NULL || A2 == NULL || A3 == NULL || A4 == NULL || B1 == NULL || B2 == NULL || B3 == NULL || B4 == NULL || C1 == NULL ||
            C2 == NULL || C3 == NULL || C4 == NULL || P1 == NULL || P2 == NULL || P3 == NULL || P4 == NULL || P5 == NULL || P6 == NULL ||
            P7 == NULL || add1 == NULL || add2 == NULL || add3 == NULL || add4 == NULL || add5 == NULL || add6 == NULL || add7 == NULL ||
            add8 == NULL || add9 == NULL || sub1 == NULL || sub2 == NULL || sub3 == NULL || sub4 == NULL || sub5 == NULL) {
            fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to perform sMul due to memory allocation failure (for sub-matrices).\n",  __LINE__, __FUNCTION__);
            freeMat(&A1); freeMat(&A2); freeMat(&A3); freeMat(&A4); freeMat(&B1);
            freeMat(&B2); freeMat(&B3); freeMat(&B4); freeMat(&C1); freeMat(&C2);
            freeMat(&C3); freeMat(&C4); freeMat(&P1); freeMat(&P2); freeMat(&P3);
            freeMat(&P4); freeMat(&P5); freeMat(&P6); freeMat(&P7); freeMat(&add1);
            freeMat(&add2); freeMat(&add3); freeMat(&add4); freeMat(&add5); freeMat(&add6);
            freeMat(&add7); freeMat(&add8); freeMat(&add9); freeMat(&sub1); freeMat(&sub2);
            freeMat(&sub3); freeMat(&sub4); freeMat(&sub5);
            return false;
        }

        // split A and B into four sub-matrices
        splits_memcpy(A, B, A1, A2, A3, A4, B1, B2, B3, B4);

        // recursive calls to perform the Strassen algorithm
        if (!subMat(B2, B4, sub1) ||
            !addMat(A1, A2, add1) ||
            !addMat(A3, A4, add2) ||
            !subMat(B3, B1, sub2) ||
            !addMat(A1, A4, add3) ||
            !addMat(B1, B4, add4) ||
            !subMat(A2, A4, sub3) ||
            !addMat(B3, B4, add5) ||
            !subMat(A1, A3, sub4) ||
            !addMat(B1, B2, add6) ||
            !matmul_improved(A1, sub1, P1) ||
            !matmul_improved(add1, B4, P2) ||
                !matmul_improved(add2, B1, P3) ||
                !matmul_improved(A4, sub2, P4) ||
                !matmul_improved(add3, add4, P5) ||
                !matmul_improved(sub3, add5, P6) ||
                !matmul_improved(sub4, add6, P7) ||
                !addMat(P5, P6, add7) ||
            !subMat(P4, P2, sub5) ||
            !addMat(P1, P5, add8) ||
            !addMat(P3, P7, add9) ||
            !addMat(add7, sub5, C1) ||
            !addMat(P1, P2, C2) ||
            !addMat(P3, P4, C3) ||
            !subMat(add8, add9, C4)) {
            fprintf(stderr, "Error in LINE %d, FUNCTION %s(): \nFailed to perform sMul due to memory allocation failure (for sub-matrices).\n",  __LINE__, __FUNCTION__);
            freeMat(&A1);
            freeMat(&A2);
            freeMat(&A3);
            freeMat(&A4);
            freeMat(&B1);
            freeMat(&B2);
            freeMat(&B3);
            freeMat(&B4);
            freeMat(&C1);
            freeMat(&C2);
            freeMat(&C3);
            freeMat(&C4);
            freeMat(&P1);
            freeMat(&P2);
            freeMat(&P3);
            freeMat(&P4);
            freeMat(&P5);
            freeMat(&P6);
            freeMat(&P7);
            freeMat(&add1);
            freeMat(&add2);
            freeMat(&add3);
            freeMat(&add4);
            freeMat(&add5);
            freeMat(&add6);
            freeMat(&add7);
            freeMat(&add8);
            freeMat(&add9);
            freeMat(&sub1);
            freeMat(&sub2);
            freeMat(&sub3);
            freeMat(&sub4);
            freeMat(&sub5);
            return false;
        }

        // merge the four sub-matrices into the final matrix
        merge_memcpy(C1, C2, C3, C4, C);

        // free memory for all sub-matrices
        freeMat(&A1);
        freeMat(&A2);
        freeMat(&A3);
        freeMat(&A4);
        freeMat(&B1);
        freeMat(&B2);
        freeMat(&B3);
        freeMat(&B4);
        freeMat(&C1);
        freeMat(&C2);
        freeMat(&C3);
        freeMat(&C4);
        freeMat(&P1);
        freeMat(&P2);
        freeMat(&P3);
        freeMat(&P4);
        freeMat(&P5);
        freeMat(&P6);
        freeMat(&P7);
        freeMat(&add1);
        freeMat(&add2);
        freeMat(&add3);
        freeMat(&add4);
        freeMat(&add5);
        freeMat(&add6);
        freeMat(&add7);
        freeMat(&add8);
        freeMat(&add9);
        freeMat(&sub1);
        freeMat(&sub2);
        freeMat(&sub3);
        freeMat(&sub4);
        freeMat(&sub5);
    }
    return true;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
bool matmul_OpenBLAS(const Matrix * A, const Matrix * B, Matrix * C) {
    CHECK("multiplication")
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, A->row, B->col, A->col, 1, A->data, A->col, B->data, B->col, 0, C->data, C->col);
    return true;
}
#pragma clang diagnostic pop


