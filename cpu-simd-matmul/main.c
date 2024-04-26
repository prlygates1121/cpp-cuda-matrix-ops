#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "operation.h"

#define SIZE 1024
#define WARMUP 20

int mod = 0;

int main() {
    srand(time(NULL));
    if (mod == 1) {
        checkAcc();
        return 0;
    }
    Matrix *A = initMat(SIZE, SIZE);
    Matrix *B = initMat(SIZE, SIZE);
    if (A == NULL || B == NULL) {
        fprintf(stderr,"Error in LINE %d, FUNCTION %s(): \nFailed to create matrices due to an initialization error.\n",
                __LINE__, __FUNCTION__);
        freeMat(&A);
        freeMat(&B);
        return 1;
    }
    fillMat(A);
    fillMat(B);
    Matrix *C = initMat(SIZE, SIZE);
    clock_t startTime, endTime;
    double duration, average = 0;
    int count = 100;
    // perform 100 naive computations
    for (int i = 1; i <= count; i++) {
        fillMat(A);
        fillMat(B);
        startTime = clock();

        if (!matmul_rkc_blocked(A, B, C)) {
            fprintf(stderr, "Error: \nFailed to perform No.%d computation (C).\n", i);
            freeMat(&A);
            freeMat(&B);
            freeMat(&C);
            return 1;
        }

        endTime = clock();
        duration = ((double) (endTime - startTime)) / (double) CLOCKS_PER_SEC;
        if (i >= WARMUP) {
            average += duration;
        }
        printf("The %2d computation time is %.3f seconds.\n", i, duration);
    }
    printf("The average computation time is %f seconds.", average / (count - WARMUP + 1));
    freeMat(&A);
    freeMat(&B);
    freeMat(&C);
    return 0;
}