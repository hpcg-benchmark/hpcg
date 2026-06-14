# NVIDIA HPC-Benchmarks Arm CPU 适用性与源码分析

生成日期：2026-06-14

## 1. 范围和资料来源

分析对象：

- NVIDIA HPC-Benchmarks 官方文档：https://docs.nvidia.com/nvidia-hpc-benchmarks/
- NVIDIA HPCG 源码：`external_sources/nvidia-hpcg`，HEAD `7dd63cd add stream sync before host buffer delete`
- HPL 参考源码：`external_sources/hpl-reference`，HEAD `9edeab5 Update link to current home VSIPL spec`
- HPL-MxP/HPL-AI 参考源码：`external_sources/hpl-mxp-reference`，HEAD `8c7ab18 Add instructions for bootstrap and configuration`
- STREAM 参考源码：`external_sources/stream-reference`
- OSU Micro-Benchmarks 源码包：`external_sources/osu-micro-benchmarks-7.5.2`

边界说明：

- NVIDIA HPC-Benchmarks 文档说明该包包含 HPL、HPL-MxP、HPCG、STREAM 四类 benchmark，以及 NCCL tests、NVSHMEM tests、OSU MPI benchmark、GEMM microbenchmark。
- 文档资源里明确公开链接到 GitHub 源码的是 `NVIDIA/nvidia-hpcg`。HPL、HPL-MxP、STREAM 在 NVIDIA 包中主要以运行包/容器和二进制说明出现，本报告对 NVIDIA 版本只做官方文档级分析，不做未公开源码推断。
- HPL、HPL-MxP/HPL-AI、STREAM、OSU Micro-Benchmarks 的源码级分析基于公开上游参考实现，用于解释 benchmark 原理和 Arm CPU 关注对象，不等同 NVIDIA Grace CPU 优化二进制。
- 本报告只讨论适用于 Arm CPU/Grace CPU 的部分。NCCL、NVSHMEM、GPU GEMM、CUDA L2 compression、MIG 等属于 NVIDIA GPU/加速器路径，不作为通用 Arm CPU benchmark 项。

### 1.1 已下载公开源码清单

| Benchmark | 本地目录 | 源码性质 | 用途 |
|---|---|---|---|
| NVIDIA HPCG | `external_sources/nvidia-hpcg` | NVIDIA 优化源码 | 分析 Grace-only CPU 路径、NVPL Sparse、SpMV/SpSV/MG/Allreduce |
| HPL | `external_sources/hpl-reference` | Netlib/ICL 参考实现 | 分析 HPL.dat、right-looking LU、panel broadcast、DGEMM/TRSM |
| HPL-MxP/HPL-AI | `external_sources/hpl-mxp-reference` | ICL 参考实现 | 分析单精度 LU、双精度 GMRES、混合精度残差校验 |
| STREAM | `external_sources/stream-reference` | 官方参考源码 | 分析 Copy/Scale/Add/Triad 和可持续内存带宽计算 |
| OSU Micro-Benchmarks | `external_sources/osu-micro-benchmarks-7.5.2` | OSU 7.5.2 官方源码包 | 分析 MPI latency、bandwidth、Allreduce、one-sided PUT 等通信微基准 |

## 2. NVIDIA HPC-Benchmarks 官方包概览

官方文档说明：

- 包含 HPL、HPL-MxP、HPCG、STREAM 四类 HPC benchmark。
- CPU 架构支持为 x86_64 和 NVIDIA Grace CPU Arm SBSA。
- x86_64 包只包含 GPU benchmark；Arm SBSA 包同时提供 GPU 和 NVIDIA Grace CPU benchmark。
- Arm SBSA 包包含 NVPL BLAS、NVPL LAPACK、NVPL Sparse、LLVM OpenMP 等 CPU 侧依赖。
- MPI 支持 OpenMPI 和 MPICH ABI 兼容实现。

对 Arm CPU 的意义：

- 这个包对“通用 Arm CPU”不是完全中立的，它主要针对 NVIDIA Grace CPU 和 Grace Hopper/Grace Blackwell 系统。
- 如果目标是 NVIDIA Grace CPU，文档和源码中的 CPU-only 路径直接有参考价值。
- 如果目标是其他 Arm HPC CPU，HPCG 算法结论仍有参考意义，但 NVPL、Grace NUMA、脚本亲和性示例和编译选项不能直接照搬。

## 3. 适用于 Arm CPU 的 benchmark 项

| NVIDIA 包内项目 | Arm CPU 适用性 | 文档/源码证据 | 主要测试对象 | 说明 |
|---|---|---|---|---|
| HPL Grace CPU-only | 直接适用于 NVIDIA Grace CPU | `hpl-aarch64.sh`、`hpl-linux-aarch64`、`xhpl` | FP64 dense BLAS、core/SVE、cache、MPI | 用标准 HPL.dat 格式，支持 CPU/memory affinity |
| HPL-MxP Grace CPU-only | 条件适用 | `hpl-mxp-aarch64.sh`、`hpl-mxp-linux-aarch64`、`xhpl_mxp` | 混合精度、BLAS、core/vector、数值迭代 | 更偏 AI-HPC 混合精度；不应替代 FP64 HPL/HPCG |
| HPCG Grace-only | 直接适用于 NVIDIA Grace CPU，且有源码 | `hpcg-aarch64.sh --exm 1`、`xhpcg-cpu`、`USE_GRACE` | SpMV、SpSV/SYMGS、MG、DDOT/Allreduce、DDR/CMN/MPI | 本报告源码级分析重点 |
| STREAM Grace CPU | 直接适用于 NVIDIA Grace CPU | `stream-cpu-test.sh`、`stream-cpu-linux-aarch64`、`stream_test` | DDR/HBM 持续带宽、NUMA、线程绑定 | 文档提供 `--n` 元素数和 `--t` 线程数 |
| OSU MPI benchmark | 适用于 Arm CPU 集群通信诊断 | `microbenchmarks/osu_mpi_tests.sh` | MPI latency/bw/collective/one-sided，NIC/PCIe/SMMU/GIC | 是 CPU/网络路径最有用的微基准 |
| NCCL tests | 非通用 Arm CPU benchmark | NCCL collectives | GPU/NVLink/NVSwitch/IB GPU 通信 | 只可作为 NVIDIA GPU 系统 readiness |
| NVSHMEM tests | 非通用 Arm CPU benchmark | NVSHMEM host/device | GPU/NVSHMEM 通信 | 不是 Arm CPU 架构通用测试 |
| GEMM microbenchmark | 主要是 GPU benchmark | `gemm_tests.sh` 文档说明矩阵乘 kernels used in NVIDIA HPL | GPU GEMM/FP64 emulation | 不作为 Grace CPU 通用 GEMM 测试 |

## 4. HPL Grace CPU-only 分析

官方文档中，Arm 包包含 `hpl-aarch64.sh`，用于在 NVIDIA Grace CPU 上调用 `xhpl`。参数包括：

- `--dat`：HPL.dat 路径。
- `--cpu-affinity`：按 MPI rank 指定 CPU core 范围。
- `--mem-affinity`：按 MPI rank 指定 NUMA memory node。
- `--ucx-affinity` 和 `--ucx-tls`：多节点通信路径设置。
- `--no-multinode`：无网络执行。

测试原理：

- 求解随机 dense linear system。
- 核心计算是 FP64 LU factorization、DGEMM、TRSM 和 panel 相关通信。

Arm CPU 关注对象：

- Grace/Arm core 的 FP64 throughput。
- NVPL BLAS/LAPACK 对 SVE/微架构的优化程度。
- L1/L2/SLC cache blocking 和 NUMA 绑定。
- 多节点时 UCX/MPI broadcast、panel communication。

目的：

- 验证 Grace CPU 在规则 dense FP64 负载上的上限。
- 适合对比 BLAS、编译器、线程/进程绑定。

不直接覆盖：

- 稀疏访存、随机访存、SMMU/GIC、PCIe/NIC 中断开销。
- DDR 延迟敏感应用。HPL 更偏计算密集，不能替代 STREAM/HPCG。

## 5. HPL-MxP Grace CPU-only 分析

官方文档中，Arm 包包含 `hpl-mxp-aarch64.sh` 和 `hpl-mxp-linux-aarch64`。CPU-only 运行需要：

- `--nprow`、`--npcol`、`--nporder`：进程网格。
- `--n`、`--nb`：矩阵规模和 blocking。
- 可选 `--cpu-affinity`、`--mem-affinity`、`--tolerance`、`--u-panel-chunk-nbs` 等。

测试原理：

- mixed precision linear solve。
- 低精度 factorization 加迭代修正，以高精度误差标准收敛。

Arm CPU 关注对象：

- 低精度计算吞吐，如果平台支持高效 FP16/BF16/SVE/SME 路径。
- BLAS 和迭代 refinement 的数值稳定性。
- 内存容量和 NUMA placement。

目的：

- 评价 AI-HPC 混合精度趋势下的矩阵求解能力。
- 对纯 FP64 HPC CPU 评价是补充项，不是主项。

## 6. STREAM Grace CPU 分析

官方文档中，Arm 包包含：

- `stream-cpu-test.sh`
- `stream-cpu-linux-aarch64/stream_test`

CPU 运行参数：

- `--n`：数组元素数量，默认 120000000。
- `--t`：线程数。

测试原理：

- STREAM 本质上测大数组 Copy/Scale/Add/Triad 类顺序访问。
- 结果反映可持续内存带宽，不等同硬件峰值带宽。

Arm CPU 关注对象：

- Grace CPU 内存带宽和 OpenMP 线程扩展。
- NUMA 和 memory placement。
- 预取、写分配、cache writeback、内存控制器通道利用率。

目的：

- 为 HPCG、NPB MG/CG、Graph500 等内存敏感负载建立带宽上限参照。

## 7. Microbenchmarks 中适用于 Arm CPU 的部分

### 7.1 OSU MPI benchmark

NVIDIA 包提供 `microbenchmarks/osu_mpi_tests.sh` 包装 OSU MPI benchmarks，支持：

- collectives：allgather、allreduce、alltoall、barrier、bcast、gather、reduce、scatter 等。
- non-blocking collectives：iallgather、iallreduce、ialltoall、ibarrier、ibcast 等。
- point-to-point：latency、bw、bibw、multi-lat、mbw-mr 等。
- congestion：fan-in、fan-out。
- one-sided：put/get latency/bw、accumulate、CAS、fetch-and-op 等。

对 Arm CPU 架构分析最有价值的部分：

- 小消息 latency：受 MPI、NIC、PCIe、SMMU、GIC/ITS/MSI、IRQ affinity 影响。
- 大消息 bandwidth：受 NIC、PCIe、NUMA、DDR、DMA、SMMU/IOTLB 影响。
- Allreduce：直接关联 HPCG DDOT 扩展瓶颈。
- Alltoall：关联 FFT、Graph500、PTRANS 类通信模式。

### 7.2 NCCL、NVSHMEM、GEMM

这些项目不建议作为通用 Arm CPU benchmark：

- NCCL tests：GPU collective correctness/performance。
- NVSHMEM：NVIDIA SHMEM host/device 侧 GPU 通信。
- GEMM microbenchmark：文档明确用于 NVIDIA HPL 中的 GPU GEMM kernel，并支持 Blackwell FP64 emulation。

它们可以用于 Grace Hopper/Grace Blackwell 系统 readiness，但不能说明单独 Arm CPU 的 core/cache/DDR/CMN 能力。

## 8. NVIDIA HPCG 源码结构

源码路径：`external_sources/nvidia-hpcg`

关键文件：

- `README.md`：说明支持 GPU-only、Grace-only、GPU-Grace，依赖 cuSPARSE/NVPL Sparse。
- `bin/RUNNING-aarch64`：aarch64 运行模式、参数和示例。
- `bin/hpcg-aarch64.sh`：实际运行脚本，处理 CPU/memory/UCX affinity 和 `--exm` 模式。
- `setup/Make.CUDA_AARCH64`：aarch64 构建选项，`USE_GRACE` 链接 NVPL Sparse。
- `src/main.cpp`：主流程，模式检查、问题生成、V&V、优化、timed benchmark、报告。
- `src/OptimizeProblem.cpp`：CPU/GPU 优化路径；CPU 路径创建 SELL 格式和 NVPL Sparse 描述符。
- `src/CpuKernels.cpp`：Grace CPU 内存分配、着色、SELL 重排、OpenMP 辅助 kernel。
- `src/ComputeSPMV.cpp`：CPU 路径调用 NVPL Sparse SpMV，并处理 halo/external SpMV。
- `src/ComputeSYMGS.cpp`：CPU 路径调用 NVPL Sparse SpSV 做 L/U sweep。
- `src/ComputeDotProduct.cpp`：本地 dot product 后 MPI_Allreduce。
- `src/CG.cpp`：PCG 迭代流程和 timing 分解。
- `src/ReportResults.cpp`：GFLOP/s、GB/s、内存模型、validity 和 Allreduce 统计。

## 9. 源码级执行路径分析

### 9.1 构建和模式选择

`README.md` 和 `setup/Make.CUDA_AARCH64` 表明：

- `USE_GRACE=1` 编译 Grace CPU 路径。
- `USE_CUDA=0` 且 `USE_GRACE=1` 生成 `xhpcg-cpu`，即 Grace-only CPU 二进制。
- `USE_CUDA=1` 且 `USE_GRACE=1` 支持 GPU-Grace。
- CPU 路径链接 `-lnvpl_sparse`。
- 编译选项含 OpenMP、`-mcpu=$(CPU_ARCH)`、`-mtune=$(CPU_ARCH)`、vectorize/unroll。

结论：

- CPU-only 是明确的一等执行模式，不是单纯 reference fallback。
- 但它绑定 NVIDIA Grace/NVPL Sparse 生态，不等于所有 Arm CPU 都可直接使用。

### 9.2 运行脚本和亲和性

`bin/hpcg-aarch64.sh` 的关键行为：

- `--exm 0`：GPU-only。
- `--exm 1`：Grace-only，脚本选择 `xhpcg-cpu`。
- `--exm 2`：GPU+Grace。
- `--cpu-affinity`：转换成 `numactl --physcpubind=...`。
- `--mem-affinity`：转换成 `numactl --membind=...`。
- `--ucx-affinity`：设置 `UCX_NET_DEVICES`。
- 默认 `OMP_PROC_BIND=TRUE`，`OMP_PLACES=sockets`。

Arm CPU 解释：

- 这个脚本直接体现了 Arm HPC CPU 测试的关键：MPI rank、OpenMP thread、NUMA memory、NIC/UCX 设备必须绑定。
- 若不绑定，HPCG 可能测到 OS 调度和跨 NUMA 抖动，而不是 CPU/DDR/CMN 真实能力。

### 9.3 main.cpp 主流程

`src/main.cpp` 的主流程：

1. `MPI_Init`。
2. `HPCG_Init` 解析参数，包括 `--exm`、`--p2p`、problem size、rank grid、slice size。
3. 检查 CPU/GPU 编译能力和 P2P 模式。CPU-only/GPU+CPU 禁止 NCCL、CUDA-aware MPI、MPI_GPU_All2allv 等 CPU 不适用模式。
4. `InitializeRanks` 分配 CPU/GPU rank 类型。
5. `GenerateGeometry`、`GenerateProblem`、`SetupHalo`、`GenerateCoarseProblem` 生成 3D 27 点 stencil 和多重网格层级。
6. CPU-only 下执行 `CheckProblem`。
7. 非 benchmark mode 下执行 reference SpMV+MG timing 和 reference CG，用于收敛基准。
8. `OptimizeProblem` 创建优化数据结构。
9. `TestCG`、`TestSymmetry`、timed CG、`TestNorms`。
10. `ReportResults` 输出性能和有效性。

CPU-only 结论：

- CPU-only 路径包含完整正确性测试和 timed benchmark。
- 如果开启 benchmark mode，会跳过 CPU reference execution，这会减少 V&V 参考路径成本，但也改变验证完整性。

### 9.4 OptimizeProblemCpu

`src/OptimizeProblem.cpp` 的 `OptimizeProblemCpu` 是 Grace CPU 优化核心：

- 调用 `AllocateMemCpu` 为每层 MG 分配 CPU/Grace 优化数据结构。
- `ColorMatrixCpu` 做矩阵着色，后续用于 SYMGS/SpSV 数据依赖处理。
- `CreateSellPermCpu` 将矩阵重排为 sliced-ELLPACK。
- 用 `nvpl_sparse_create_sliced_ell` 创建 L、U、A 三个稀疏矩阵描述符。
- 为 SpMV 创建 `nvpl_sparse_spmv_create_descr` 和 buffer size。
- 为 SpSV 创建 `nvpl_sparse_spsv_create_descr`、设置 fill mode、做 analysis，并更新 diagonal。
- 对 coarse grid 做 `F2cPermCpu`，保证 restriction/prolongation 的重排序一致。

硬件意义：

- SELL 格式试图提高 sparse 访问的连续性和向量化友好程度。
- 着色和 SpSV analysis 试图降低 SYMGS 数据依赖带来的串行瓶颈。
- NVPL Sparse 是 CPU sparse kernel 的核心性能来源。

### 9.5 CpuKernels.cpp

`src/CpuKernels.cpp` 包含：

- `AllocateMemCpu`：分配 diagonal、ref2opt/opt2ref、color、CSR external、SELL A/L/U、SpSV buffer。
- `PrefixsumCpu`、`Prefixsum8ColCpu`、`parCount`、`parMaxElement`：OpenMP 并行辅助。
- `ColorMatrixCpu`：矩阵着色。
- `CreateSellPermCpu`：创建 SELL 格式和置换。
- `ComputeDotProductCpu`：CPU 本地 dot product。
- `ComputeWAXPBYCpu`：向量线性组合。
- `SpmvDiagCpu`、`AxpbyCpu`、`SpFmaCpu`、`ExtSpMVCpu`：辅助向量和 external SpMV。

硬件意义：

- OpenMP 线程并行直接压 core、L1/L2 和共享内存带宽。
- SELL/CSR external 结构会暴露 cache/TLB 和内存带宽问题。
- external SpMV 加 halo exchange 会暴露 MPI rank 网格和 NUMA/网络路径问题。

### 9.6 ComputeSPMV

CPU 路径：

- 设置 NVPL dense vector 指针。
- 调用 `nvpl_sparse_spmv` 计算本地 `A*x`。
- 如果存在 halo，调用 `ExchangeHaloCpu`，再用 `ExtSpMVCpu` 处理外部列贡献。

测试对象：

- 本地 SpMV：DDR/HBM、SLC/L2、TLB、NVPL Sparse、OpenMP。
- Halo：MPI 点对点、NIC、PCIe/SMMU、NUMA、进程网格。

### 9.7 ComputeSYMGS

CPU 路径：

- 前向/后向 sweep 使用 NVPL Sparse SpSV 解 L/U。
- 对 diagonal 做 `SpmvDiagCpu`/`SpFmaCpu`。
- 在 MG 层里结合 L/U SpMV 和 halo exchange。
- 源码里有注释：在某些路径调用 `MPI_Ibarrier` 可改善后续 DDOT 的 MPI_Allreduce。

测试对象：

- SpSV/SYMGS 是 HPCG 中最难跑满硬件的部分之一。
- 它会暴露数据依赖、cache/TLB、内存延迟、OpenMP 调度和稀疏库质量。

### 9.8 ComputeDotProduct 和 CG

`ComputeDotProduct.cpp`：

- GPU 路径用 cuBLAS/NCCL 或 MPI。
- CPU 路径调用 `ComputeDotProductCpu` 做本地 dot。
- 跨 rank 汇总使用 `MPI_Allreduce`，并累积 `time_allreduce`。

`CG.cpp`：

- 每次迭代包括 `ComputeMG`、`ComputeWAXPBY`、`ComputeSPMV`、`ComputeDotProduct`。
- timing 数组把 dot product、WAXPBY、SpMV、Allreduce、preconditioner 分开。

测试对象：

- 本地 dot/WAXPBY：内存带宽和 OpenMP reduction。
- Allreduce：MPI collective、网络延迟、进程映射。
- CG 总时间：稀疏计算、内存、通信的组合表现。

### 9.9 ReportResults

`ReportResults.cpp` 做三类模型：

- FLOP count：DDOT、WAXPBY、SpMV、preconditioner。
- Memory bandwidth model：读写字节模型。
- Memory usage model：矩阵、向量、MG 层级和优化结构。

还统计：

- DDOT `MPI_Allreduce` min/max/avg。
- 官方最短运行时间阈值 1800 秒。
- correctness/V&V 结果。

架构解释：

- 如果 GFLOP/s 低但 STREAM 高，需要看 SpSV、TLB、CMN、MPI。
- 如果 DDOT Allreduce 占比高，多节点扩展瓶颈在网络/MPI/进程网格。
- 如果 memory model 带宽远低于 STREAM，可能是稀疏访存局部性、TLB、SLC 或 NUMA 问题。

## 10. 公开参考源码补充分析

本节分析的是公开上游参考源码，用于解释 benchmark 原理和 Arm CPU 关注对象。除 `NVIDIA/nvidia-hpcg` 外，这些源码不是 NVIDIA HPC-Benchmarks 包内 Grace CPU 优化二进制源码。

### 10.1 HPL 参考源码

源码路径：`external_sources/hpl-reference`

关键文件：

- `README`：说明 HPL 2.3 求解双精度随机稠密线性系统，需要 MPI 和 BLAS/VSIPL。
- `TUNING`：说明 `HPL.dat` 的 problem size、block size、process grid、broadcast topology、lookahead、panel factorization 等调参项。
- `testing/ptest/HPL_pddriver.c`：HPL 主入口，读取 `HPL.dat`，遍历 N、NB、P x Q、broadcast、lookahead、panel factorization 等组合。
- `src/pgesv/HPL_pdgesv.c`：主求解入口，选择无 lookahead 或 lookahead LU 路径，最后调用三角求解。
- `src/pgesv/HPL_pdupdateNN.c`：trailing submatrix update 路径，结合 panel broadcast、row swap、`HPL_dtrsm` 和 `HPL_dgemm`。

源码级执行路径：

1. `MPI_Init` 建立分布式运行环境。
2. `HPL_pdinfo` 读取 `HPL.dat`，得到矩阵规模 N、block size NB、进程网格 P/Q、阈值和算法参数。
3. `HPL_grid_init` 根据 P x Q 创建 row/column communicator。
4. `HPL_pdgesv` 执行 right-looking LU factorization，按照 lookahead depth 选择 `HPL_pdgesv0` 或 `HPL_pdgesvK2`。
5. panel factorization 处理当前 panel，broadcast topology 把 panel 沿进程行传播。
6. trailing submatrix update 主要调用 BLAS3 的 `DGEMM` 和 `TRSM`。
7. `HPL_pdtrsv` 做最终上三角回代，并做残差校验。

测试对象和 Arm CPU 解释：

- 主要压力是 FP64 dense BLAS，尤其是 `DGEMM` 的 FMA 吞吐、SVE/NEON 向量化、register blocking 和 L1/L2/SLC 数据复用。
- `NB`、P x Q、broadcast topology 和 lookahead depth 会改变 cache blocking、通信重叠和 panel latency。
- 对 Arm CPU，HPL 更适合看 core/vector/math library 上限，不适合单独判断 DDR 随机访问、SMMU、GIC 或小消息中断路径。
- NVIDIA HPL Grace CPU-only 可用相同算法概念理解，但性能依赖 NVPL BLAS、Grace NUMA 绑定和 NVIDIA 包内调优，不能直接由参考源码推断。

### 10.2 HPL-MxP/HPL-AI 参考源码

源码路径：`external_sources/hpl-mxp-reference`

关键文件：

- `README`：说明运行形式为 `./hpl-ai <n> <max_iter>`，输出 conversion、factorization、solve、GMRES、total time 和 scaled residual。
- `hpl-ai.c`：主入口，生成矩阵和向量，将 double 转 single，执行单精度 LU，回转 double 后用 GMRES 修正。
- `sgetrf_nopiv.c`：无 pivot 单精度 LU factorization，内部用 `SGEMM` 和 `STRSM` 更新。
- `gmres.c`：Flexible GMRES 迭代，使用 double precision 残差和三角求解做修正。
- `blas.c`：参考 BLAS 子集，包含 `SGEMM`、`STRSM`、`DTRSM`、`DGEMV`、`DLANGE` 等简化实现。

源码级执行路径：

1. `matgen` 和 `vecgen` 生成 double precision 的 `A` 和 `b`。
2. `convert_double_to_float` 将 `A` 和 `b` 转成 single precision。
3. `sgetrf_nopiv` 做单精度 LU，无 pivot，核心更新使用 `SGEMM`。
4. `STRSM` 做 forward/backward substitution，得到单精度解。
5. `convert_float_to_double` 把 LU 和解转回 double。
6. `gmres` 用 double precision 做预条件残差迭代修正。
7. 最后计算 `||Ax-b||_oo / (eps * (||x||_oo * ||A||_oo + ||b||_oo) * N)`，阈值默认为 16。

测试对象和 Arm CPU 解释：

- 该参考实现是单进程教学/参考路径，不是分布式 HPL-MxP 优化实现。
- 它清楚展示了 mixed precision linear solve 的基本结构：低精度 factorization 提供速度，高精度迭代修正确保结果可信。
- 对 Arm CPU，真正有价值的测试点是 FP32/FP16/BF16/SME/SVE2 的矩阵吞吐、数据转换成本、GMRES 迭代次数和数值稳定性。
- NVIDIA HPL-MxP Grace CPU-only 的文档参数可用于运行配置，但源码级结论只能基于此参考实现解释算法，不应推断 NVIDIA 二进制内部优化。

### 10.3 STREAM 参考源码

源码路径：`external_sources/stream-reference`

关键文件：

- `READ.ME`：说明 STREAM 是测量 sustained memory bandwidth 的事实标准；官方 MPI 版本不是首选，STREAM 更偏共享内存带宽。
- `stream.c`：C 参考实现，默认 `STREAM_TYPE` 为 `double`，默认 `NTIMES=10`，可通过编译宏设置数组规模和数据类型。
- `stream.f`：Fortran 参考实现。
- `mysecond.c`：Fortran 版本外部计时函数，C 版本内嵌计时。

源码级执行路径：

1. 分配/初始化 `a`、`b`、`c` 三个大数组。
2. 预热后循环执行 `NTIMES` 次四个 kernel。
3. Copy：`c[j] = a[j]`，一个读流和一个写流。
4. Scale：`b[j] = scalar * c[j]`，一个读流、一个写流和少量浮点乘。
5. Add：`c[j] = a[j] + b[j]`，两个读流和一个写流。
6. Triad：`a[j] = b[j] + scalar * c[j]`，两个读流、一个写流和 FMA/乘加。
7. 统计时跳过第一次迭代，用每个 kernel 的最短时间计算 `bytes / min_time`。
8. `checkSTREAMresults` 校验最终数组结果。

测试对象和 Arm CPU 解释：

- STREAM 直接压 DDR/HBM 持续带宽、预取器、write allocate、writeback、内存控制器通道均衡和 NUMA placement。
- 它的顺序访问非常规则，对 TLB、CMN、cache 的压力比 HPCG/Graph500 更可控，因此适合作为内存带宽上限参考。
- 对 Arm CPU，必须确保工作集大于 cache，并控制 OpenMP 线程数、线程绑定、NUMA 本地/远端/interleave 策略。
- NVIDIA STREAM Grace CPU 文档中的 `--n` 和 `--t` 对应数组元素数和线程数，源码原理可由 `stream.c` 解释；NVIDIA 二进制调优细节仍以文档和实测为准。

### 10.4 OSU Micro-Benchmarks 参考源码

源码路径：`external_sources/osu-micro-benchmarks-7.5.2`

关键文件：

- `README`：说明 GNU build system、MPI/OpenSHMEM/UPC++/CUDA/ROCm/SYCL/OpenACC 扩展，以及 pt2pt、collective、one-sided 等测试说明。
- `c/mpi/pt2pt/standard/osu_latency.c`：两进程 ping-pong latency 测试。
- `c/mpi/pt2pt/standard/osu_bw.c`：窗口化连续 `MPI_Isend`/`MPI_Irecv` bandwidth 测试。
- `c/mpi/collective/blocking/osu_allreduce.c`：`MPI_Allreduce` latency 测试，输出 min/max/avg。
- `c/mpi/one-sided/osu_put_latency.c`：`MPI_Put` one-sided latency，支持 lock/fence/flush 等同步方式。

源码级执行路径：

- `osu_latency` 要求两个进程。rank 0 发送消息给 rank 1，再接收同尺寸回复，循环多次后用总时间除以 `2 * iterations` 得到单向 latency。
- `osu_bw` 要求两个进程。发送端用 window size 批量发出多个非阻塞消息，接收端收完后返回确认，按传输 MB 数除以耗时得到 MB/s。
- `osu_allreduce` 要求至少两个进程。每个消息尺寸执行多次 `MPI_Allreduce`，用 `MPI_Reduce` 汇总各 rank 的 min/max/avg latency。
- `osu_put_latency` 使用 `MPI_Win` 和 `MPI_Put`，不同同步模式会触发不同 MPI RMA 和底层 RDMA/内存注册路径。

测试对象和 Arm CPU 解释：

- 小消息 latency 对 MPI progress、NIC doorbell、PCIe/CXL root complex、SMMU/IOMMU、GIC/ITS/MSI、IRQ affinity、CPU 频率和进程绑定敏感。
- 大消息 bandwidth 对 RDMA/NIC 带宽、PCIe lane、NUMA 到 NIC 的距离、DMA mapping、IOTLB 和内存带宽敏感。
- `osu_allreduce` 直接帮助解释 HPCG `DDOT -> MPI_Allreduce` 的扩展瓶颈。
- one-sided PUT/GET 更适合暴露 RDMA、IOMMU/SMMU 和内存注册策略问题。
- OSU 源码同时包含 GPU/加速器扩展，但本报告只把 host-based MPI C benchmarks 作为 Arm CPU 通信诊断对象。

## 11. NVIDIA HPCG 对 Arm CPU 架构组件的覆盖

| 测试对象 | 覆盖程度 | 源码/运行路径 | 说明 |
|---|---|---|---|
| FP64 core/SVE | 间接 | SpMV/SYMGS/DDOT/WAXPBY | 算术强度低，不适合测 FP64 峰值 |
| L1/L2 cache | 直接 | SELL SpMV、SpSV、WAXPBY | 稀疏索引和值访问强依赖 cache |
| SLC/CMN/CHI | 直接但需 PMU 解释 | 多线程 SpMV/SYMGS、NUMA 绑定 | 输出不直接报告 CMN，需要 PMU/拓扑辅助 |
| TLB/MMU | 直接但需 PMU 解释 | 大稀疏矩阵、SELL/CSR external、halo vector | 可通过 hugepage/page size A/B 实验分析 |
| DDR/HBM | 直接 | SpMV、WAXPBY、MG | 与 STREAM 结果配合解释 |
| NUMA | 直接 | `--cpu-affinity`、`--mem-affinity`、numactl | 绑定不当会显著影响结果 |
| MPI 网络 | 直接 | halo exchange、MPI_Allreduce | DDOT Allreduce 是关键扩展瓶颈 |
| PCIe/NIC | 间接 | UCX/MPI/RDMA path | 需要 OSU/UCX counters 辅助 |
| SMMU/IOMMU | 间接 | NIC DMA、MPI buffers | HPCG 不直接测 SMMU，需 IOMMU on/off 和 OSU/fio |
| GIC/ITS/MSI | 弱到间接 | MPI progress、小消息/中断 | HPCG 不是中断 benchmark |
| 存储 | 不覆盖 | 无 timed I/O 主路径 | 需 IOR/mdtest/fio |
| RAS/热/功耗 | 间接 | 长时间 timed run | 需外部功耗、温度、RAS 日志 |

## 12. 对 Boss 关注项的直接回答

### cache?

HPCG、Graph500、NPB、SPEC CPU 都能暴露 cache 行为。NVIDIA HPCG 的 SELL/SpSV/SpMV 会让 L1/L2/SLC 命中率和预取有效性直接影响性能。但 benchmark 输出不告诉你 cache miss，需要 perf/PMU/LIKWID。

### core?

HPL/HPL-MxP/SPEC CPU 更适合测 core 和向量单元。HPCG 对 core 峰值不敏感，更多是内存和稀疏库效率。

### CMN 总线/片上互连?

STREAM scaling、HPCG、Graph500、HPCC RandomAccess 对 CMN/CHI/SLC 更敏感。NVIDIA HPCG 可通过线程数、NUMA 绑定、rank grid 和 PMU 对比分析 CMN/SLC 瓶颈。

### DDR?

STREAM 是最直接上限测试。HPCG 是更真实的稀疏访问压力。二者都需要测试本地 NUMA、远端 NUMA、interleave。

### PCIe?

NVIDIA HPCG CPU-only 不直接压 PCIe。多节点 MPI 经过 NIC 时会间接经过 PCIe/CXL root complex。要直接测 PCIe/NIC，请使用 OSU MPI、RDMA 工具、fio/IOR。

### SMMU?

NVIDIA HPCG 不直接报告 SMMU 开销。SMMU 影响 NIC/NVMe DMA 地址翻译，应通过 OSU MPI one-sided/point-to-point、fio/IOR、IOMMU on/off、ATS/PRI 配置和 PMU/IOMMU counters 分析。

### GIC?

HPCG 不直接测 GIC。GIC/ITS/MSI 主要体现在小消息通信、网络/存储中断、IRQ affinity。OSU latency、message rate、fio、网络工具更适合。

## 13. 后续建议

### 13.1 NVIDIA Grace CPU 建议运行顺序

如果目标是 NVIDIA Grace CPU，建议按“先测硬件上限，再测应用有效性能，最后拆通信”的顺序执行：

1. 先跑 NVIDIA STREAM CPU，确认内存带宽和 NUMA 绑定。
2. 跑 NVIDIA HPL CPU，确认 NVPL BLAS、FP64、进程网格。
3. 跑 NVIDIA HPCG Grace-only，采集 SpMV/MG/DDOT/Allreduce 分项。
4. 跑 NVIDIA HPL-MxP CPU，确认混合精度路径、迭代收敛和低精度库表现。
5. 跑 OSU MPI，重点看 latency、bw、allreduce、alltoall、one-sided。
6. 同步采集 PMU、CMN、memory controller、IRQ、NUMA、MPI/UCX 配置。

这样排序的原因：

- STREAM 给出内存带宽上限，后续 HPCG、Graph500、NPB MG/CG 都要以它作为解释参照。
- HPL 先确认 FP64 dense math、NVPL BLAS、进程网格和基本频率/功耗是否正常。
- HPCG 再验证真实稀疏访存、NVPL Sparse、SpSV/SYMGS、halo 和 Allreduce。
- HPL-MxP 放在 HPL 之后，避免把混合精度结果误当成传统 FP64 峰值。
- OSU MPI 用来拆解 HPCG DDOT/Allreduce、halo exchange 或多节点扩展问题。

### 13.2 三个必须先锁定的配置项

| 配置项 | 影响对象 | 检查方法 | 失败表现 |
|---|---|---|---|
| NVPL 版本和链接路径 | HPL/HPL-MxP/HPCG CPU kernel | 记录 `NVPL_BLAS_PATH`、`NVPL_LAPACK_PATH`、`NVPL_SPARSE_PATH` 和运行包版本 | HPL/HPL-MxP/HPCG 同时偏低，或不同节点结果不可比 |
| CPU/memory affinity | core、NUMA、DDR、CMN | 记录 `--cpu-affinity`、`--mem-affinity`、`numactl -H`、`lscpu -e` | STREAM 带宽低、HPCG 波动、跨 NUMA 远端访问增加 |
| 问题规模 | cache/SLC/DDR、HPCG 有效性 | STREAM 数组远大于 SLC；HPL N 占用足够内存；HPCG local problem size 满足规则 | 数据被 cache 兜住导致虚高，或规模太小导致通信/启动开销占比异常 |

### 13.3 Grace 亲和性和 first-touch 原则

Grace CPU 运行脚本提供 `--cpu-affinity` 和 `--mem-affinity`，但具体拆分必须以目标机器的 `lscpu -e`、`numactl -H`、固件 NUMA 暴露方式为准。

建议原则：

- STREAM 初始化和 timed loop 都必须在最终线程绑定下运行；Linux first-touch 会让首次写入的线程决定内存页落在哪个 NUMA node。
- HPL 可从“每 NUMA 域一个或少数 MPI rank”起步，用 P x Q 网格和 `HPL.dat` 调整通信/计算比例。
- HPCG 可从更细粒度 rank 划分起步，让每个 rank 的 local 3D 子域尽量贴近本地 NUMA 内存，并观察 halo 与 Allreduce 占比。
- OSU MPI 测 NIC 路径时，需要把 MPI rank、内存、UCX device/NIC 绑定到同一个 NUMA 邻近区域。

常见起点：

- 对 72 核粒度清晰的 Grace NUMA 域，可先试 HPL `0-71:72-143` 这类大 rank 绑定。
- 对 HPCG/STREAM，若平台暴露更细 NUMA 或内存控制器分组，可试 `0-35:36-71:72-107:108-143` 这类更细分绑定。
- 这些只是起点，不是固定推荐值；最终以 STREAM 带宽、HPCG 分项时间、OSU latency/bw 和 PMU 计数共同判断。

### 13.4 结果判读顺序

| 现象 | 优先检查 |
|---|---|
| STREAM 低 | 线程数、CPU affinity、memory affinity、first-touch、频率、NUMA interleave、内存通道状态 |
| HPL 低但 STREAM 正常 | NVPL BLAS、`HPL.dat` 的 N/NB/P x Q、FP64 频率、OpenMP/MPI 进程绑定 |
| HPCG 低但 STREAM/HPL 正常 | NVPL Sparse、SpSV/SYMGS、SELL/CSR external、TLB/page size、rank grid、halo exchange |
| HPCG Allreduce 占比高 | OSU allreduce、小消息 latency、MPI/UCX collective 算法、rank placement、NIC 亲和性 |
| 多节点扩展差 | OSU latency/bw/alltoall、UCX device、IB/RoCE 配置、PCIe/NIC NUMA 位置、SMMU/IOMMU 策略 |
| 结果波动大 | CPU governor、热降频、IRQ affinity、后台服务、NUMA 漂移、MPI 进程映射 |

### 13.5 非 NVIDIA Arm CPU 建议

如果目标是非 NVIDIA Arm CPU：

1. 使用标准 HPL/HPCG/STREAM/NPB/OSU，而不是直接采用 NVIDIA Grace 专用二进制。
2. 数学库替换为该平台推荐库，比如 ArmPL、OpenBLAS、BLIS、厂商 BLAS/Sparse。
3. 用 LIKWID/perf/pmu-tools 补硬件计数。
4. 把 NVIDIA HPCG 源码中的 SELL、着色、SpSV/NVPL 思路作为优化参考，而不是直接照搬。

## 14. NVIDIA 和源码分析相关术语

通用 benchmark、Arm 架构 IP、I/O/中断、MPI 和算法 kernel 的全称见 `01_hpc_cpu_benchmark_arm_architecture_report.md` 的“专业术语与英文全称”。本节补充 NVIDIA HPC-Benchmarks 和源码阅读中高频出现的专用术语。

| 缩写/名称 | 英文全称 | 中文解释 |
|---|---|---|
| NVIDIA HPC-Benchmarks | NVIDIA High Performance Computing Benchmarks | NVIDIA 发布的 HPC benchmark 包，包含 HPL、HPL-MxP、HPCG、STREAM 以及 microbenchmarks。 |
| Grace CPU | NVIDIA Grace Central Processing Unit | NVIDIA Arm Neoverse 数据中心 CPU，是 NVIDIA Arm SBSA 包的 CPU 侧重点。 |
| Arm SBSA | Arm Server Base System Architecture | Arm 服务器基础系统架构规范，用于保证服务器软件栈兼容性。 |
| GH200 | NVIDIA Grace Hopper Superchip | Grace CPU 与 Hopper GPU 的紧耦合平台。 |
| GB200 | NVIDIA Grace Blackwell Superchip | Grace CPU 与 Blackwell GPU 的紧耦合平台。 |
| NVPL | NVIDIA Performance Libraries | NVIDIA 性能库集合，Arm SBSA 包里包含 NVPL BLAS、LAPACK、Sparse。 |
| NVPL BLAS | NVIDIA Performance Libraries Basic Linear Algebra Subprograms | NVIDIA BLAS 实现，服务 HPL/HPL-MxP 等 dense linear algebra。 |
| NVPL LAPACK | NVIDIA Performance Libraries Linear Algebra Package | NVIDIA LAPACK 实现，服务 dense solver 和相关线性代数例程。 |
| NVPL Sparse | NVIDIA Performance Libraries Sparse | NVIDIA 稀疏线性代数库，NVIDIA HPCG Grace-only 路径用它执行 SpMV/SpSV。 |
| cuSPARSE | CUDA Sparse Matrix Library | NVIDIA GPU 稀疏矩阵库，属于 GPU 路径，不是通用 Arm CPU 稀疏库。 |
| cuBLAS | CUDA Basic Linear Algebra Subprograms | NVIDIA GPU BLAS 库，属于 GPU 路径。 |
| NCCL | NVIDIA Collective Communications Library | NVIDIA GPU collective 通信库，常用于 GPU Allreduce/Broadcast 等。 |
| NVSHMEM | NVIDIA OpenSHMEM implementation | NVIDIA SHMEM 编程模型实现，主要用于 GPU/host-device 通信。 |
| CUDA-aware MPI | CUDA-aware Message Passing Interface | MPI 可直接识别 CUDA device buffer 的能力；CPU-only Grace 路径不应依赖它。 |
| UCX_NET_DEVICES | Unified Communication X network devices | UCX 网卡选择环境变量，NVIDIA 运行脚本用它绑定 MPI/UCX 网络设备。 |
| `--exm` | execution mode | NVIDIA HPCG aarch64 脚本中的执行模式参数：GPU-only、Grace-only、GPU-Grace。 |
| `--p2p` | peer-to-peer mode | NVIDIA HPCG 中选择 MPI CPU、CUDA-aware MPI、NCCL 等通信路径的参数。 |
| CPU affinity | CPU affinity | 进程或线程绑定到指定 CPU core/range，减少迁移和 NUMA 抖动。 |
| memory affinity | memory affinity | 内存绑定到指定 NUMA node，控制本地/远端内存访问。 |
| SELL | Sliced ELLPACK sparse matrix format | 分片 ELLPACK 稀疏格式，NVIDIA HPCG CPU 路径用它改善稀疏访存连续性。 |
| CSR external | Compressed Sparse Row external matrix part | HPCG 中跨 rank halo/external 列贡献的 CSR 存储。 |
| halo exchange | halo exchange | 分布式 stencil/稀疏计算中交换边界数据的通信过程。 |
| SpSV analysis | Sparse Triangular Solve analysis | 稀疏三角求解前的结构分析，用于确定依赖、缓冲区和执行策略。 |
| color / coloring | graph coloring | 矩阵/图着色，用颜色分组降低 SYMGS/SpSV 中的数据依赖冲突。 |
| benchmark mode | benchmark mode | NVIDIA HPCG 中可跳过部分 reference execution 的运行模式，需注意 V&V 覆盖变化。 |
| host-based benchmark | host-based benchmark | OSU 中使用 CPU host memory/MPI 的 benchmark，区别于 CUDA/ROCm/SYCL device buffer 测试。 |
| one-sided / RMA | one-sided communication / Remote Memory Access | MPI 一侧发起的远端内存访问模型，如 `MPI_Put`、`MPI_Get`。 |
| PAPI | Performance Application Programming Interface | 性能计数接口库，OSU 源码可通过它采集硬件事件。 |
| XCCL | Accelerator collective communication abstraction | OSU 中对 NCCL/RCCL collective benchmark 的统一抽象，属于加速器通信路径。 |

## 15. 参考资料

- NVIDIA HPC-Benchmarks 文档：https://docs.nvidia.com/nvidia-hpc-benchmarks/
- NVIDIA HPL 文档：https://docs.nvidia.com/nvidia-hpc-benchmarks/HPL_benchmark.html
- NVIDIA HPL-MxP 文档：https://docs.nvidia.com/nvidia-hpc-benchmarks/HPL_MxP_benchmark.html
- NVIDIA HPCG 文档：https://docs.nvidia.com/nvidia-hpc-benchmarks/HPCG_benchmark.html
- NVIDIA STREAM 文档：https://docs.nvidia.com/nvidia-hpc-benchmarks/STREAM_Benchmark.html
- NVIDIA Microbenchmarks 文档：https://docs.nvidia.com/nvidia-hpc-benchmarks/Microbenchmarks.html
- NVIDIA HPCG GitHub：https://github.com/NVIDIA/nvidia-hpcg
- HPL 官方说明：https://netlib.org/benchmark/hpl/
- HPL GitHub 参考源码：https://github.com/icl-utk-edu/hpl
- HPL-MxP 官方说明：https://hpl-mxp.org/
- HPL-AI/HPL-MxP 参考源码：https://bitbucket.org/icl/hpl-ai
- HPCG 官方软件页：https://www.hpcg-benchmark.org/software/index.html
- STREAM 官方页：https://www.cs.virginia.edu/stream/
- STREAM 官方源码目录：https://www.cs.virginia.edu/stream/FTP/Code/
- OSU Micro-Benchmarks：https://mvapich.cse.ohio-state.edu/benchmarks/
