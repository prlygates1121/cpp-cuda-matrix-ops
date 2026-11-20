# High-Performance Matrix Operations (C / C++ / CUDA)

A progressive systems and high-performance computing project series implementing dense matrix operations from scratch across **CPU (C11 + AVX2/FMA + OpenMP)**, **modern C++ (C++17 templated library with reference-counted ROI views)**, and **GPU (CUDA C++ SGEMM kernels + cuBLAS)**.

## Modules

| Module | Stack | Focus & Highlights |
| :--- | :--- | :--- |
| [`cpu-simd-matmul/`](./cpu-simd-matmul) | C11, AVX2/FMA, OpenMP, OpenBLAS | Systematic SGEMM optimization: loop reordering (`rck` → `rkc`), transposition, AVX2/FMA intrinsics (`_mm256_fmadd_ps`), loop unrolling, cache blocking (`BLOCK_SIZE = 64`), OpenMP, and hybrid Strassen recursion |
| [`cpp-matrix-library/`](./cpp-matrix-library) | C++17, OpenMP, GoogleTest | Generic `Matrix<T>` class with nested reference-counted `MatrixData`, shallow copy + explicit `clone()` / `detach()`, nested Region of Interest (`ROI`) views, operator overloading, file I/O, and exception safety |
| [`cuda-matrix-ops/`](./cuda-matrix-ops) | CUDA C++, cuBLAS, Visual Studio | Dual host/device `Matrix` buffer management; element-wise kernels and progressive CUDA SGEMM kernels (naive → coalesced → `32×32` shared-memory tiling → 4-element/thread register coarsening) |

---

## Performance Highlights

### 1. CPU Matrix Multiplication (`cpu-simd-matmul/`)
Benchmarked on `float` square matrices (Release build, `-march=native -fopenmp`, AMD Ryzen 7 7840H, 16 GB DDR5; full data in [`cpu-simd-matmul/report/report.pdf`](./cpu-simd-matmul/report/report.pdf)):

| Matrix Size | Naive (`matmul_rck`) | Blocked + OpenMP (`matmul_rkc_blocked_omp`) | Hybrid Strassen + Blocked OpenMP (`matmul_improved`) | OpenBLAS (`cblas_sgemm`) |
| :---: | :---: | :---: | :---: | :---: |
| `1024 × 1024` | 6.443 s | 0.010 s | 0.010 s | 0.004 s |
| `2048 × 2048` | 49.841 s | 0.071 s | 0.073 s | 0.019 s |
| `4096 × 4096` | — | 0.695 s | 0.610 s | 0.139 s |
| `8192 × 8192` | — | 7.394 s | 4.768 s | 1.153 s |

- Verified numerical accuracy against naive `matmul_rck` (`checkAcc()`, average entry error `0.000000`) and zero memory leaks on core allocation paths via Valgrind.

### 2. Generic C++ Matrix Library (`cpp-matrix-library/`)
- **Memory Management**: Reference-counted `MatrixData<U>` avoids unnecessary deep copies on assignment and submatrix slicing; `clone()` and `detach()` provide explicit copy isolation.
- **Region of Interest (`ROI`)**: `getROI()`, `adjustROI()`, and `cancelROI()` support nested submatrix views sharing underlying storage with stride-aware indexing (`STEP`).
- **Blocked OpenMP Multiplication**: Multiplies `6231 × 4567` by `4567 × 5291` in ~1 s (`BLOCK_SIZE = 112` with edge padding).
- **Verification**: Unit test suite in [`test.cpp`](./cpp-matrix-library/test.cpp) (GoogleTest) and Valgrind zero-leak checks documented in [`cpp-matrix-library/report/report.md`](./cpp-matrix-library/report/report.md).

### 3. GPU Matrix Multiplication (`cuda-matrix-ops/`)
Benchmarked end-to-end (including host-to-device and device-to-host `cudaMemcpy` transfers) against `cublasSgemm` and CPU OpenBLAS (see [`cuda-matrix-ops/report/Project 5 GPU Acceleration with CUDA.md`](./cuda-matrix-ops/report/Project%205%20GPU%20Acceleration%20with%20CUDA.md)):

| Matrix Size | `GPU_coalesce` | `GPU_blocking` (`32×32` shared memory) | `GPU_blocking_inside` (4 outputs/thread) | `GPU_cuBLAS` | CPU OpenBLAS |
| :---: | :---: | :---: | :---: | :---: | :---: |
| `2560 × 2560` | 86 ms | 53 ms | **25 ms** | 56 ms | 36 ms |
| `5120 × 5120` | 592 ms | 324 ms | **154 ms** | 113 ms | 275 ms |
| `10240 × 10240` | 4522 ms | 2518 ms | **1176 ms** | 549 ms | 2176 ms |

- Handled row-major to column-major mapping for `cublasSgemm` via $C^T = B^T A^T$ argument ordering, achieving `Max diff: 0` against cuBLAS.

---

## Build Notes

- **`cpu-simd-matmul/`**: Built with CMake (C11, `-march=native -fopenmp`, links OpenBLAS). Update `OpenBLAS_DIR` in [`cpu-simd-matmul/CMakeLists.txt`](./cpu-simd-matmul/CMakeLists.txt) to match your local OpenBLAS path.
- **`cpp-matrix-library/`**: Built with CMake (C++17, `-fopenmp -march=native`). Uncomment the `FetchContent` GoogleTest block in [`cpp-matrix-library/CMakeLists.txt`](./cpp-matrix-library/CMakeLists.txt) to run [`test.cpp`](./cpp-matrix-library/test.cpp).
- **`cuda-matrix-ops/`**: Visual Studio CUDA solution ([`Project5.sln`](./cuda-matrix-ops/Project5.sln)) linking CUDA Runtime and `cublas`.
