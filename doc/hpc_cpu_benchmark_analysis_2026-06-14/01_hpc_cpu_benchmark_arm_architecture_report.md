# HPC CPU/Arm 架构主流 Benchmark 测试项分析报告

生成日期：2026-06-14

## 1. 范围和结论

本报告面向 HPC CPU 芯片和整机系统架构分析，重点关注 Arm 服务器/HPC CPU。这里的“benchmark”不只是一两个跑分项，而是覆盖从微架构、内存子系统、片上互连、I/O 到多节点通信的分层测试体系。

核心结论：

1. 只看 HPL/TOP500 不足以评价 Arm HPC CPU。HPL 主要体现 FP64 dense GEMM/LU、BLAS、编译器和 MPI 规模效率，通常不能充分暴露 DDR/HBM、CMN/CHI、cache/TLB、随机访存、MPI 小消息和 I/O 问题。
2. Arm HPC CPU 更应该组合使用 HPL、HPCG、STREAM、HPCC、OSU Micro-Benchmarks、Graph500、NAS Parallel Benchmarks、SPEC CPU/SPEChpc，以及 LIKWID/lmbench/IOR 等微基准和系统基准。
3. 对 Arm 芯片架构分析，应按“组件路径”组织结果：core/SVE/FPU、L1/L2/SLC、TLB/MMU、CMN/CHI 一致性互连、DDR/HBM/NUMA、PCIe/CXL/SMMU、GIC/ITS/MSI、NIC/RDMA、存储和 OS/MPI/OpenMP runtime。
4. SMMU、GIC、PCIe 这类 I/O/中断 IP 很少被 HPL/HPCG 直接测到，通常需要 OSU MPI、RDMA 工具、fio/IOR、irq affinity、IOMMU on/off、perf/PMU 计数配合分析。

建议方案：

- 第一层：微基准和 bring-up。STREAM、LIKWID、lmbench、OSU MPI、fio/IOR、PMU 计数，确认单节点硬件路径是否健康。
- 第二层：HPC kernel/mini-app。HPL、HPCG、HPCC、Graph500、NPB，定位 core、内存、互连、MPI 和稀疏/随机访问瓶颈。
- 第三层：应用/采购级。SPEChpc、SPEC CPU、代表性应用或 miniapp，用于比较真实工作负载吞吐、能效和可扩展性。

## 2. Arm HPC CPU 架构测试对象

| 架构组件 | Arm HPC 常见对象/IP | 主要关注问题 | 典型 benchmark |
|---|---|---|---|
| CPU core 前端/后端 | Arm Neoverse N/V/E 系列、A64FX 等 | IPC、分支、乱序窗口、load/store、整数和 FP 执行单元 | SPEC CPU speed、NPB EP/BT/SP/LU、HPL 单 rank |
| SIMD/向量/矩阵 | NEON、SVE/SVE2、部分平台可能有 SME 或厂商矩阵扩展 | FP64/FP32/FP16/BF16/INT8 吞吐、编译器自动向量化、数学库质量 | HPL、DGEMM、HPL-MxP、SPEC FP、SPEChpc |
| L1/L2 cache | core 私有 I/D cache、私有 L2 | 热数据命中率、load-use、store buffer、预取器、cache line 回填 | SPEC CPU、LIKWID bench、lmbench、NPB kernel |
| 系统级 cache/SLC/LLC | CMN/DSU/厂商 SLC | 多核共享数据、远端 cache 命中、容量、带宽、cache-to-cache 迁移 | STREAM scaling、HPCG、Graph500、HPCC RandomAccess |
| TLB/MMU | stage-1 translation、huge page、page walk | 大工作集、随机访问、页表 walk、THP/hugepage 效果 | lmbench lat_mem_rd、Graph500、HPCG、SPEC mcf 类负载 |
| 一致性互连 | CMN-600/650/700、CHI、Home Node、RN-F/RN-I、SN-F | 多核一致性、跨 socket/跨 NUMA、DVM、RFO、mesh hot spot | STREAM 多线程、HPCC RandomAccess、Graph500、OpenMP scaling |
| 内存控制器 | DDR4/DDR5/HBM、NUMA node | 持续带宽、读写混合、bank/channel 均衡、内存容量 | STREAM、HPCG、NPB MG/CG/FT、LIKWID |
| MPI/片外网络 | InfiniBand/RoCE/Slingshot/厂商网络、RDMA NIC | 点对点带宽/延迟、collective、Allreduce、Alltoall、拥塞 | OSU MPI、HPCG DDOT、Graph500、HPCC PTRANS/FFT |
| PCIe/CXL root complex | PCIe Gen4/5/6、CXL.mem/cache | NIC/NVMe/加速器 DMA 带宽、拓扑、NUMA 亲和性 | OSU MPI with RDMA、fio、IOR、CXL microbench |
| SMMU/IOMMU | Arm SMMU v3、ATS/PRI、I/O page table | DMA 地址翻译开销、IOMMU TLB、设备隔离、虚拟化 | OSU MPI/RDMA、fio/IOR、设备直通测试 |
| GIC/ITS/MSI | GICv3/v4、ITS、MSI/MSI-X | 中断分发、IRQ affinity、MSI 路由、网络/存储中断负载 | OSU 小消息、fio、netperf/iperf、系统级延迟测试 |
| RAS/电源/频率 | RAS controller、thermal、DVFS、PMU | 长时间稳定性、降频、错误恢复、能效 | HPL/HPCG 长跑、SPECpower、功耗采集 |
| OS/runtime | Linux NUMA、OpenMP、MPI、编译器、BLAS/FFT/Sparse 库 | 进程绑定、线程绑定、hugepage、libc、MPI progress | 所有 benchmark，尤其 HPL/HPCG/SPEChpc |

注意：CMN、SLC、SMMU、GIC 往往不是 benchmark 输出项里的显式字段。它们需要通过拓扑绑定、A/B 参数、PMU 计数和系统日志来反推。

## 3. 主流 benchmark 总览

| Benchmark | 测试项 | 原理 | 对 Arm CPU 的主要关注对象 | 目的 |
|---|---|---|---|---|
| HPL / TOP500 | FP64 dense linear solve | 生成稠密矩阵，做 LU 分解和回代，报告 FLOP/s | core FPU/SVE、BLAS、L1/L2/cache blocking、MPI panel/bcast | 评价峰值附近 FP64 计算能力和大规模 dense math 可扩展性 |
| HPL-MxP | mixed precision linear solve | 低精度 LU + GMRES/迭代细化恢复精度 | 低精度向量/矩阵单元、BLAS、编译器、库 | 评价 HPC+AI 混合精度能力，不等同传统 FP64 |
| HPCG | 稀疏 PCG + multigrid + SYMGS | 27 点 stencil 稀疏矩阵，SpMV、MG、DDOT/Allreduce、WAXPBY | DDR/HBM、cache/TLB、SLC/CMN、MPI Allreduce、NUMA | 弥补 HPL，对稀疏、内存和通信瓶颈更敏感 |
| STREAM | Copy/Scale/Add/Triad | 大数组顺序读写，工作集大于 cache | DDR/HBM 持续带宽、NUMA、内存控制器、预取 | 测量可持续内存带宽和读写混合效率 |
| HPCC | HPL、DGEMM、STREAM、PTRANS、RandomAccess、FFT、通信 | 一套覆盖计算、内存、随机访问、FFT、通信的组合 | core、DDR、CMN、MPI 网络、随机访存、FFT all-to-all | 用多个指标描述系统性能画像 |
| OSU Micro-Benchmarks | MPI latency/bw、collective、one-sided、startup | ping-pong、窗口化连续消息、collective 反复计时 | NIC、PCIe/SMMU、GIC/IRQ、MPI、NUMA | 诊断 MPI/网络/进程映射和小消息/大消息瓶颈 |
| Graph500 | BFS/SSSP，GTEPS | 生成大图，随机/不规则图遍历 | cache/TLB、SLC/CMN、DDR、MPI all-to-all/消息风暴 | 评价数据密集、低局部性、图分析能力 |
| NAS Parallel Benchmarks | IS、EP、CG、MG、FT、BT、SP、LU、UA、BT-IO、DC、DT | CFD/科学计算 kernel 和 pseudo-app | 计算、内存、通信、I/O、混合并行 | 作为应用特征更丰富的可移植 mini-app 套件 |
| SPEC CPU 2026/2017 | integer/floating point speed/rate | 真实应用源码，编译后计时，speed 测单任务，rate 测吞吐 | core、cache、内存、编译器 | 评价单核/多副本 CPU 计算性能和编译器质量 |
| SPEChpc 2021 | Tiny/Small/Medium/Large HPC 应用/mini-app | MPI、MPI+OpenMP、offload 多模型 | 单节点到多节点应用吞吐、MPI/OpenMP、内存、编译器 | 采购/应用级 HPC 平台比较 |
| LIKWID | topology、perfctr、pin、bench | 拓扑枚举、PMU 读数、线程绑定、微基准 | cache/NUMA/PMU/线程绑定，支持 Armv8 等 | 解释 benchmark 背后的硬件计数和拓扑 |
| lmbench | latency/bandwidth microbench | POSIX/UNIX 微操作、内存层次延迟/带宽 | cache/TLB、内存、系统调用、进程切换 | 获得基础系统成本曲线 |
| IOR / mdtest | parallel I/O 和 metadata | MPI 并行文件读写、metadata 创建/删除/stat | NVMe、网络文件系统、PCIe/SMMU、page cache | 评价 HPC 存储和并行文件系统 |

## 4. 逐项原理、目的和 Arm 关注点

### 4.1 HPL / TOP500

HPL 是 High Performance LINPACK 的分布式实现，官方说明指出它在分布式内存机器上求解双精度随机稠密线性系统，算法包含二维 block-cyclic 数据分布、right-looking LU、panel factorization、broadcast、swap-broadcast 和回代。

测试原理：

- 计算密集。大部分时间应落在 DGEMM/TRSM 等 BLAS3 操作。
- 数据重用高。良好的 cache blocking 可以把大量计算留在寄存器、L1/L2/SLC 中。
- 通信集中在 panel broadcast、pivoting、回代等阶段。

Arm CPU 关注对象：

- SVE/NEON FP64 实现、load/store 带宽、FMA 吞吐。
- BLAS 库，比如 Arm Performance Libraries、OpenBLAS、BLIS、NVPL BLAS 等。
- 编译器对 `-mcpu`、SVE 向量长度无关代码和 OpenMP/MPI 绑定的支持。
- 多 NUMA/多节点时的 MPI broadcast、panel 延迟和内存亲和性。

目的：

- 评价“可优化 dense FP64 峰值”。
- 适合回答“这个 CPU 在高度规则、计算密集矩阵任务上能跑到理论峰值多少”。
- 不适合单独回答“真实 HPC 稀疏、随机访存、DDR、CMN、网络小消息是否强”。

### 4.2 HPL-MxP

HPL-MxP 的官方说明强调低精度 LU 分解和迭代细化，目标是连接传统 HPC 和 AI 低精度硬件趋势。

测试原理：

- 用较低精度做主要 factorization。
- 用迭代 refinement/GMRES 把解恢复到接近 FP64 的精度。
- 性能取决于低精度矩阵计算吞吐、数据转换、迭代次数和数值稳定性。

Arm CPU 关注对象：

- 若平台有高效 FP16/BF16/INT8/SME/SVE2 路径，则可用于评估混合精度科学计算潜力。
- 对传统仅强调 FP64 的 Arm HPC CPU，HPL-MxP 更偏趋势性指标，不应替代 HPL/HPCG。

目的：

- 评价“低精度加速 + 高精度收敛”的综合能力。
- 适合 AI-HPC 融合平台，不适合作为通用 Arm CPU 系统组件完整覆盖指标。

### 4.3 HPCG

HPCG 使用多重网格预条件共轭梯度。它的典型 kernel 包括 SpMV、SYMGS、restriction/prolongation、WAXPBY、DDOT/Allreduce。问题是 3D 27 点 stencil，工作集通常远大于 cache。

测试原理：

- SpMV 读取矩阵值、列索引和向量，算术强度低。
- SYMGS/SpSV 有数据依赖和不规则访存，难以完全向量化。
- DDOT 需要全局归约，扩展到多节点后常由 `MPI_Allreduce` 限制。
- halo exchange 暴露节点间边界通信和进程网格映射问题。

Arm CPU 关注对象：

- DDR/HBM 读带宽和读延迟。
- L1/L2/SLC 命中率、TLB miss、page size。
- CMN/CHI 上跨核共享向量、RFO 和远端访问。
- MPI Allreduce、halo exchange、NUMA 绑定和线程绑定。

目的：

- 评价真实稀疏科学计算中更常见的“内存 + 通信 + 不规则访问”能力。
- 对 Arm CPU 架构分析非常重要，尤其用于解释为什么 HPL 很高但应用性能不一定高。

### 4.4 STREAM

STREAM 官方定义为可持续内存带宽 benchmark，常用 Copy、Scale、Add、Triad 四类向量操作。

测试原理：

- 使用大数组，避免完全驻留 cache。
- 通过简单顺序访问测量内存子系统可持续带宽。
- Triad 通常是 `a[i] = b[i] + scalar * c[i]`，有两个读流和一个写流。

Arm CPU 关注对象：

- DDR/HBM channel 是否跑满。
- NUMA 本地/远端带宽差异。
- 预取器、写分配、cache writeback 行为。
- OpenMP 线程绑定和 memory placement。

目的：

- 得到内存带宽上限或近似上限。
- 为 HPCG、NPB MG/CG、Graph500 等内存敏感负载解释瓶颈。

### 4.5 HPCC

HPCC 把 HPL、DGEMM、STREAM、PTRANS、RandomAccess、FFT、通信带宽/延迟放在一个套件里。官方页面明确列出了这 7 类测试。

测试原理和对象：

- HPL/DGEMM：dense FP64 和 BLAS。
- STREAM：本地内存带宽。
- PTRANS：并行矩阵转置，测试大量成对通信。
- RandomAccess：随机 64-bit 更新，测试低局部性、TLB、SLC/CMN、远端访问。
- FFT：3D/1D 变换中的 all-to-all 和内存重排。
- b_eff 通信：网络带宽和延迟。

目的：

- 比单个 HPL 更像“性能画像”。
- 适合 Arm CPU 初步评价，因为它能同时覆盖核心计算、内存、随机访存和通信。

### 4.6 OSU Micro-Benchmarks

OSU Micro-Benchmarks 官方说明覆盖 MPI 点对点、blocking/non-blocking collectives、one-sided、startup，以及 OpenSHMEM、UPC、NCCL 等模型。点对点 latency 使用 ping-pong，bandwidth 使用窗口化连续消息。

Arm CPU 关注对象：

- NIC 到 CPU 的 PCIe/CXL 亲和性。
- SMMU/IOMMU translation、DMA mapping、IOTLB 行为。
- GIC/ITS/MSI 中断路由和 IRQ affinity，尤其小消息和高消息率场景。
- MPI library、UCX/libfabric、进程绑定、NUMA 内存绑定。

目的：

- 定位通信栈问题，而不是应用算法问题。
- 用于解释 HPCG `MPI_Allreduce`、Graph500 消息风暴、SPEChpc 多节点扩展不佳的底层原因。

### 4.7 Graph500

Graph500 官方页面将其定位为补充 Top500 的 data-intensive benchmark，使用 BFS 和 SSSP，指标为 TEPS/GTEPS。

测试原理：

- 图遍历访问模式不规则，局部性差。
- frontier 展开带来大量随机访存和跨 rank 消息。
- BFS/SSSP 更看重内存层次、通信和负载均衡，而非 FP64。

Arm CPU 关注对象：

- TLB、SLC、CMN/CHI、DDR 延迟。
- MPI all-to-all/点对点消息率。
- NUMA 和进程图划分。

目的：

- 评价数据分析、图计算、低局部性负载。
- 对分析 CMN/SLC 和内存延迟非常有价值。

### 4.8 NAS Parallel Benchmarks

NASA 官方页面说明 NPB 源自 CFD 应用，原始套件包含 IS、EP、CG、MG、FT 五个 kernel 和 BT、SP、LU 三个 pseudo-application，并扩展到 UA、BT-IO、DC、DT、多区版本等。

Arm CPU 关注对象：

- IS：整数排序和随机访存。
- EP：embarrassingly parallel，偏 core/编译器/随机数。
- CG/MG：稀疏、内存、通信。
- FT：FFT 和 all-to-all。
- BT/SP/LU：PDE solver，混合计算和内存访问。
- BT-IO/DT/DC：I/O 和数据移动。

目的：

- 用更贴近应用的 mini-app 组合评价系统。
- 对 Arm CPU 来说，NPB 能帮助区分“纯计算强”“内存强”“通信强”的不同瓶颈。

### 4.9 SPEC CPU 2026/2017 与 SPEChpc 2021

SPEC CPU 2026 官方页面说明它包含 CPU intensive suites，用于比较 compute intensive performance，压力对象包括 processor、memory subsystem 和 compiler，包含 52 个 benchmark，分为 SPECspeed/SPECrate 的 Integer/Floating Point 四套。SPEC CPU 2017 在 2026 年仍处于过渡期，但 SPEC 已发布 CPU 2026。

SPEChpc 2021 官方页面说明它有 Tiny、Small、Medium、Large 四套，覆盖科学应用和 mini-app，支持 MPI、MPI+OpenACC、MPI+OpenMP、MPI+OpenMP target 等模型。

Arm CPU 关注对象：

- SPEC CPU speed：单任务、单核或少核强度，适合看 core/cache/compiler。
- SPEC CPU rate：多副本吞吐，适合看 core 数量、共享 cache、DDR、NUMA。
- SPEChpc：更接近多节点 HPC 应用，适合看 MPI/OpenMP、内存和应用可扩展性。

目的：

- SPEC CPU 更适合 CPU 产品横向比较。
- SPEChpc 更适合 HPC 采购和应用级比较。

### 4.10 LIKWID、lmbench、IOR/mdtest

LIKWID 官方 README 说明它支持 Intel、AMD、ARMv8、POWER9 等 Linux 处理器，并包含 topology、perfctr、pin、bench 等工具。它不是传统排名 benchmark，但对 Arm 架构分析非常实用。

lmbench 官方说明它是一组 UNIX/POSIX ANSI C microbenchmark，主要测 latency 和 bandwidth，用于理解关键操作基础成本。

IOR/mdtest 仓库说明它包含 IOR 和 mdtest parallel I/O benchmark，用于并行文件系统和 metadata 测试。

Arm CPU 关注对象：

- LIKWID：topology、cache/NUMA、PMU、线程绑定，解释 benchmark 结果。
- lmbench：cache/TLB/memory latency、进程/系统调用成本。
- IOR/mdtest：NVMe、PCIe/SMMU、并行文件系统、metadata、page cache。

目的：

- 这些工具更适合“定位问题”，不一定适合公开跑分排名。
- 在 Arm bring-up 和芯片系统分析中，它们比单一 HPL 更能快速定位硬件/系统路径问题。

## 5. 组件覆盖矩阵

| 测试对象 | HPL | HPL-MxP | HPCG | STREAM | HPCC | OSU MPI | Graph500 | NPB | SPEC CPU | SPEChpc | LIKWID/lmbench | IOR/mdtest |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| FP64 core/SVE | 直接 | 间接 | 弱 | 弱 | 直接 | 弱 | 弱 | 直接 | 直接 | 直接 | 间接 | 弱 |
| FP16/BF16/INT8 | 弱 | 直接 | 弱 | 弱 | 弱 | 弱 | 弱 | 弱 | 部分 | 部分 | 间接 | 弱 |
| L1/L2 cache | 直接 | 直接 | 直接 | 间接 | 直接 | 弱 | 直接 | 直接 | 直接 | 直接 | 直接 | 弱 |
| SLC/LLC/CMN | 间接 | 间接 | 直接 | 直接 | 直接 | 间接 | 直接 | 直接 | 间接 | 直接 | 直接 | 弱 |
| TLB/MMU | 间接 | 间接 | 直接 | 间接 | 直接 | 弱 | 直接 | 直接 | 直接 | 直接 | 直接 | 弱 |
| DDR/HBM bandwidth | 间接 | 间接 | 直接 | 直接 | 直接 | 弱 | 直接 | 直接 | 直接 | 直接 | 直接 | 间接 |
| NUMA placement | 间接 | 间接 | 直接 | 直接 | 直接 | 直接 | 直接 | 直接 | 直接 | 直接 | 直接 | 直接 |
| MPI latency/bw | 间接 | 间接 | 直接 | 弱 | 直接 | 直接 | 直接 | 直接 | 弱 | 直接 | 间接 | 直接 |
| PCIe/CXL | 弱 | 弱 | 间接 | 弱 | 间接 | 直接 | 间接 | 间接 | 弱 | 间接 | 间接 | 直接 |
| SMMU/IOMMU | 弱 | 弱 | 间接 | 弱 | 间接 | 直接 | 间接 | 间接 | 弱 | 间接 | 间接 | 直接 |
| GIC/ITS/MSI | 弱 | 弱 | 间接 | 弱 | 间接 | 直接 | 间接 | 间接 | 弱 | 间接 | 间接 | 直接 |
| 文件系统/I/O | 弱 | 弱 | 弱 | 弱 | 弱 | 弱 | 弱 | 部分 | 弱 | 部分 | 弱 | 直接 |
| 编译器/数学库 | 直接 | 直接 | 直接 | 间接 | 直接 | 间接 | 间接 | 直接 | 直接 | 直接 | 间接 | 弱 |

## 6. Arm CPU 测试设计建议

### 6.1 单节点 bring-up

目标是确认硬件路径和 OS 配置没有明显问题。

- 拓扑：`lscpu -e`、`numactl -H`、`hwloc-ls`、LIKWID topology。
- 频率/功耗：固定 governor，记录温度和降频。
- 内存：STREAM 按 1、2、4、8、全核扩展，分别测本地 NUMA、跨 NUMA、interleave。
- 延迟：lmbench 或等价 pointer-chasing，画 cache/TLB/memory latency 阶梯。
- PMU：采集 cycles、instructions、L1D/LLC/TLB miss、memory controller、CMN PMU 事件。

### 6.2 计算和内存 kernel

- HPL/DGEMM：确认 FP64/SVE/BLAS 峰值。
- HPCG：确认稀疏、内存和 MPI 归约路径。
- HPCC RandomAccess/FFT/PTRANS：补随机访问和 all-to-all。
- NPB CG/MG/FT/BT/SP/LU：补应用型 kernel。

### 6.3 通信、PCIe、SMMU、GIC

- OSU latency/bw/allreduce/alltoall：区分小消息延迟、大消息带宽、collective。
- 开关 IOMMU/SMMU 或改变 DMA mapping 策略时，需要记录安全和可比性边界。
- 设置 NIC IRQ affinity、RPS/XPS、NUMA 绑定，观察小消息和 high message rate。
- 对 RDMA 路径，应结合 `ib_*`/UCX/libfabric 工具和 OSU one-sided。

### 6.4 存储和系统稳定性

- IOR/mdtest/fio：测 NVMe、本地盘、并行文件系统。
- 长时间 HPL/HPCG/STREAM loop：测热稳定、功耗、RAS、ECC 错误。
- 所有结果记录编译器、MPI、BLAS、kernel、firmware、BIOS/UEFI、NUMA/hugepage 配置。

### 6.5 专项测试补充

经典 HPC 计算 benchmark 不能单独覆盖所有 Arm SoC IP。尤其是 CMN/NoC、PCIe、SMMU 和 GIC，需要用专项工具和 A/B 配置把问题拆开。

#### 6.5.1 CMN/NoC 和 NUMA

CMN/NoC 没有一个统一的“标准跑分项”。它通常通过跨 NUMA、跨 cache slice、一致性流量和 PMU 计数间接分析。

| 方法 | 测试对象 | 原理 | 适合回答的问题 |
|---|---|---|---|
| 跨 NUMA STREAM | CMN/NoC、SLC、内存控制器、NUMA | 线程绑定在一个 NUMA node，数据放在本地、远端或 interleave 内存，对比带宽差异 | 本地/远端内存带宽损失是多少，互连是否限制扩展 |
| STREAM/HPCG 扫线程数 | core、SLC、CMN、DDR | 从少核到全核逐步增加线程，观察带宽或 GFLOP/s 是否线性增长 | 多核扩展瓶颈在 core、内存还是互连 |
| cache-to-cache 微基准 | L1/L2/SLC、CMN 一致性 | 一个核写 cache line，另一个核读，测 cache line 迁移延迟 | 核间一致性传输延迟和拓扑差异 |
| false-sharing 微基准 | CMN 一致性、cache line ownership | 多核频繁写同一 cache line 的不同字段，制造一致性流量 | 一致性风暴下 CMN 和 cache coherence 是否成为瓶颈 |
| CMN PMU/perf | CMN XP/HN-F、SLC slice、memory node | 采集 mesh crosspoint、home node、SLC/memory traffic 事件 | benchmark 分数低时，互连内部哪个方向或节点拥塞 |

注意：CMN PMU 事件名称依赖具体平台、kernel、firmware 和 perf 支持，报告结果时必须记录事件名、socket/die/NUMA 拓扑和绑定方式。

#### 6.5.2 GIC/ITS/MSI 和中断抖动

GIC 不是 HPL/HPCG/STREAM 的直接测试对象。要分析中断路径，应使用实时/调度/网络中断类测试。

| 方法 | 测试对象 | 原理 | 适合回答的问题 |
|---|---|---|---|
| `cyclictest` | GIC、timer interrupt、scheduler、CPU idle wakeup | 周期性唤醒线程，统计期望唤醒和实际唤醒之间的 min/avg/max latency | 中断和调度尾延迟是否满足实时性要求 |
| `hackbench` | scheduler、IPI、核间唤醒 | 大量进程/线程通过 pipe/socket 交换消息，制造调度和 IPI 压力 | 高并发调度下是否出现延迟抖动 |
| OSU 小消息 + IRQ affinity | NIC MSI/MSI-X、ITS、GIC、MPI progress | 固定网卡 IRQ 到不同 CPU/NUMA，跑 `osu_latency`、message rate、small allreduce | 小消息通信瓶颈是网络、MPI 还是中断/亲和性 |
| netperf/iperf 小包 | NIC、GIC、kernel network stack | 高 PPS 网络收发，观察软中断、IRQ、丢包和尾延迟 | 网络收包路径和中断分发是否稳定 |

分析 GIC 时要同时记录 `irqbalance`、IRQ affinity、CPU idle governor、`isolcpus`/`nohz_full`、网卡队列数和 NUMA 亲和性。

#### 6.5.3 PCIe、CXL、SMMU/IOMMU

PCIe/SMMU 主要出现在网卡、NVMe、GPU、CXL 设备和虚拟化/设备直通路径中。经典 CPU 计算 benchmark 只能间接触发这些路径。

| 方法 | 测试对象 | 原理 | 适合回答的问题 |
|---|---|---|---|
| OSU MPI/RDMA | NIC、PCIe root complex、SMMU、NUMA | 使用 point-to-point、one-sided PUT/GET 和 RDMA 路径，对比消息大小和进程/网卡绑定 | NIC 与 CPU/NUMA/PCIe 亲和性是否正确 |
| `fio` NVMe | NVMe、PCIe、SMMU、page cache/block layer | 用 direct I/O、不同 block size、queue depth 测吞吐和 IOPS | 存储 DMA 路径和 PCIe 带宽是否达标 |
| DPDK `testpmd` | NIC、PCIe、SMMU、IOTLB、hugepage | 绕过内核网络栈跑小包/大包转发，比较 IOMMU on/off 或 passthrough | SMMU 地址翻译对高 PPS 或 DMA 的开销 |
| IOMMU A/B 实验 | SMMU/IOMMU、ATS/PRI、IOTLB | 固定应用、网卡、NUMA、hugepage 后，仅改变 IOMMU/SMMU 策略 | DMA 翻译开销和安全隔离代价 |
| 厂商 PCIe bandwidth 工具 | PCIe link、P2P、device DMA | 主机内存到设备、设备到设备大块传输 | PCIe lane、Gen 速率、拓扑和 P2P 是否正常 |

IOMMU/SMMU on/off 对比会改变安全隔离语义，不应直接作为生产配置建议；它主要用于定位性能损失来源。

#### 6.5.4 稳定性、功耗和热

稳定性测试不等同公开 benchmark 排名，但在 Arm HPC CPU bring-up 中非常关键。

| 方法 | 测试对象 | 原理 | 适合回答的问题 |
|---|---|---|---|
| HPL 长跑 | FP64 core/SVE、供电、散热 | 长时间 dense FP64 计算，持续压浮点和功耗 | 峰值算力下是否降频、报错或不稳定 |
| HPCG 长跑 | DDR/SLC/CMN/MPI | 长时间稀疏访存和通信 | 内存/互连/网络压力下是否稳定 |
| STREAM loop | DDR/HBM、内存控制器、热 | 多轮流式读写压内存带宽 | 内存带宽是否随温度或时间下降 |
| `stress-ng` | CPU、cache、VM、I/O、scheduler | 选择不同 stressor 组合压指定子系统 | 快速复现热、功耗、调度、内存压力问题 |
| RAS/ECC 日志采集 | ECC、SError、AER、firmware | 采集 EDAC、RAS daemon、kernel log、BMC SEL | 长时间压力下是否出现硬件纠错或链路错误 |

### 6.6 按问题选工具

| 想搞清楚的问题 | 推荐组合 | 关键判读 |
|---|---|---|
| FP64 算力天花板 | HPL + DGEMM microbenchmark + perf/PMU | HPL 接近库和硬件预期，说明 core/SVE/BLAS 基本健康 |
| 混合精度能力 | HPL-MxP + 低精度 GEMM/BLAS microbenchmark | 关注低精度吞吐、迭代次数和最终残差 |
| 内存带宽上限 | STREAM + 线程数 sweep + NUMA local/remote/interleave | STREAM 是带宽上限参照，HPCG/Graph500 不应高于这个物理边界 |
| cache/TLB 层次 | lmbench/tinymembench/pointer-chasing + perf TLB/cache events | 通过 working set 拐点定位 L1/L2/SLC/DDR 延迟阶梯 |
| 稀疏科学计算有效性能 | HPCG + STREAM + TLB/cache/CMN PMU | STREAM 高但 HPCG 低时，优先看 SpSV、TLB、SLC/CMN 和 MPI |
| CMN/NUMA 扩展 | 跨 NUMA STREAM + HPCG rank/thread sweep + CMN PMU | 本地/远端差异和全核扩展曲线是关键 |
| 节点间通信 | OSU latency/bw/allreduce/alltoall + UCX/MPI counters | 小消息看 latency/GIC/IRQ，大消息看 NIC/PCIe/NUMA |
| PCIe/NIC 路径 | OSU RDMA + PCIe bandwidth 工具 + NUMA 绑定 | 设备离哪个 NUMA 近、是否跨 root complex，会直接影响结果 |
| SMMU/IOMMU 开销 | DPDK/fio/OSU one-sided + IOMMU A/B + hugepage | 小包、小 I/O、内存注册和 IOTLB miss 最敏感 |
| GIC/中断抖动 | cyclictest + hackbench + IRQ affinity + OSU 小消息 | max/tail latency 比平均值更重要 |
| 存储/并行文件系统 | IOR/mdtest/fio + NUMA/PCIe/SMMU 记录 | 区分本地 NVMe、网络文件系统、metadata 和 page cache |
| 整机稳定性 | HPL/HPCG/STREAM 长跑 + stress-ng + RAS/温度/频率日志 | 关注降频、ECC/RAS 事件和性能随时间漂移 |

## 7. 常见误区

1. HPL 高不代表 HPC 应用高。HPL 高说明 dense FP64 和 BLAS 强，但无法代表稀疏、内存、随机访问和通信。
2. STREAM 高不代表 HPCG 高。STREAM 是顺序流访问，HPCG 有稀疏索引、TLB、halo 和 Allreduce。
3. OSU MPI 好不代表应用扩展好。OSU 是微通信，应用还有计算/通信重叠、负载均衡和进程网格。
4. SMMU/GIC 不能靠 HPL/HPCG 单独验收。它们需要 I/O、RDMA、IRQ、DMA 和系统计数配合。
5. Arm CPU 结果高度依赖编译器和库。SVE/SVE2、OpenMP runtime、BLAS/FFT/Sparse 库和 `-mcpu` 选择会显著改变结果。

## 8. 专业术语与英文全称

### 8.1 Benchmark 和测试套件

| 缩写/名称 | 英文全称 | 中文解释 |
|---|---|---|
| HPL | High Performance LINPACK | 高性能 LINPACK，求解双精度稠密线性方程组，是 TOP500 的核心指标。 |
| TOP500 | TOP500 Supercomputer Sites | 全球超级计算机性能排名，主要按 HPL `Rmax` 排序。 |
| HPL-MxP | High Performance LINPACK Mixed Precision | 混合精度 LINPACK，用低精度加速主要计算，再用迭代修正恢复精度。 |
| HPL-AI | HPL-AI Benchmark, AI = Artificial Intelligence | HPL-MxP 的前身/相关参考实现名称；官方通常直接使用 HPL-AI 作为专名，强调低精度硬件在科学计算中的使用。 |
| HPCG | High Performance Conjugate Gradients | 高性能共轭梯度，使用稀疏矩阵、多重网格和全局归约，更接近内存/通信受限科学计算。 |
| STREAM | Sustainable Memory Bandwidth benchmark | 可持续内存带宽 benchmark，通过 Copy、Scale、Add、Triad 测顺序内存带宽。 |
| HPCC | HPC Challenge Benchmark | HPC Challenge 套件，组合 HPL、DGEMM、STREAM、PTRANS、RandomAccess、FFT 和通信测试。 |
| OMB / OSU Micro-Benchmarks | OSU Micro-Benchmarks | 俄亥俄州立大学微基准，常用于 MPI 点对点、collective、one-sided、startup 通信诊断。 |
| NPB | NAS Parallel Benchmarks | NASA 并行 benchmark，包含 CG、MG、FT、BT、SP、LU 等 kernel/pseudo-app。 |
| SPEC CPU | Standard Performance Evaluation Corporation CPU benchmark | SPEC CPU 评测，偏 CPU、内存子系统和编译器的应用级性能。 |
| SPEChpc | SPEC High Performance Computing benchmark | SPEC HPC 评测，覆盖 MPI/OpenMP/offload 的 HPC 应用和 mini-app。 |
| Graph500 | Graph500 benchmark | 数据密集型图计算 benchmark，使用 BFS/SSSP 和 TEPS/GTEPS 指标。 |
| LIKWID | Like I Knew What I’m Doing | Linux 性能工具集，包含 topology、pin、perfctr、bench，适合解释硬件计数和绑定效果。 |
| IOR | Interleaved Or Random I/O benchmark | 并行 I/O benchmark，测试 HPC 文件系统读写吞吐。 |
| mdtest | metadata test | 文件系统 metadata benchmark，测试创建、删除、stat 等元数据操作。 |

### 8.2 Arm CPU 架构和系统 IP

| 缩写/名称 | 英文全称 | 中文解释 |
|---|---|---|
| SIMD | Single Instruction, Multiple Data | 单指令多数据，向量计算基础模式。 |
| NEON | Arm Advanced SIMD extension | Arm 传统 128-bit SIMD 扩展。 |
| SVE | Scalable Vector Extension | Arm 可伸缩向量扩展，向量长度由实现决定，适合高性能数学库。 |
| SVE2 | Scalable Vector Extension version 2 | SVE 的第二代扩展，增强整数、DSP、密码等向量能力。 |
| SME | Scalable Matrix Extension | Arm 可伸缩矩阵扩展，面向矩阵/AI 类计算。 |
| FPU | Floating-Point Unit | 浮点执行单元。 |
| L1/L2 | Level-1 / Level-2 Cache | 一级/二级 cache，通常 L1 私有、L2 私有或簇内共享。 |
| LLC | Last Level Cache | 最后一级 cache，通常是访问内存前的最大共享 cache。 |
| SLC | System Level Cache | 系统级 cache，Arm 服务器 SoC 常用于跨核/设备共享数据缓存。 |
| CMN | Coherent Mesh Network | Arm 一致性网格互连 IP，例如 CMN-600/650/700。 |
| CHI | Coherent Hub Interface | Arm AMBA CHI 一致性互连接口协议。 |
| DSU | DynamIQ Shared Unit | Arm DynamIQ 共享单元，常用于核心簇内共享资源。 |
| RN-F/RN-I | Request Node Fully coherent / Request Node I/O coherent | CHI 中的请求节点类型，分别面向全一致性 CPU 侧和 I/O 一致性设备侧。 |
| SN-F | Slave Node Fully coherent | CHI 中承载内存或系统级目标的全一致性从节点。 |
| Home Node | Home Node | 一致性系统中的归属节点，负责 cache line 一致性和目录/状态管理。 |
| DVM | Distributed Virtual Memory | 分布式虚拟内存消息机制，常用于 TLB/cache 维护广播。 |
| TLB | Translation Lookaside Buffer | 地址翻译缓存，缓存虚拟地址到物理地址的转换。 |
| MMU | Memory Management Unit | 内存管理单元，负责地址翻译、权限和属性检查。 |
| THP | Transparent Huge Pages | Linux 透明大页机制，可减少 TLB miss，但也可能带来抖动。 |
| PMU | Performance Monitoring Unit | 性能监控单元，用硬件计数器统计 cycles、cache miss、TLB miss 等事件。 |
| RAS | Reliability, Availability, and Serviceability | 可靠性、可用性、可维护性，包含 ECC、错误上报、恢复和日志。 |

### 8.3 I/O、通信和中断

| 缩写/名称 | 英文全称 | 中文解释 |
|---|---|---|
| PCIe | Peripheral Component Interconnect Express | 高速外设互连，用于 NIC、NVMe、GPU、CXL 设备等。 |
| CXL | Compute Express Link | 基于 PCIe 物理层的缓存/内存扩展互连协议。 |
| SMMU | System Memory Management Unit | Arm 系统内存管理单元，负责设备 DMA 地址翻译和隔离。 |
| IOMMU | Input-Output Memory Management Unit | 通用 I/O MMU 名称，功能与 SMMU 类似。 |
| ATS | Address Translation Service | PCIe/CXL 设备侧地址翻译服务，可减少 IOMMU 往返。 |
| PRI | Page Request Interface | PCIe 设备页请求接口，用于设备缺页/地址翻译协作。 |
| DMA | Direct Memory Access | 设备绕过 CPU 直接访问内存。 |
| RDMA | Remote Direct Memory Access | 远端直接内存访问，常见于 InfiniBand/RoCE 网络。 |
| NIC | Network Interface Controller | 网络接口控制器。 |
| GIC | Generic Interrupt Controller | Arm 通用中断控制器。 |
| ITS | Interrupt Translation Service | GICv3/v4 中用于 MSI/MSI-X 转换和路由的服务。 |
| MSI/MSI-X | Message Signaled Interrupts / Extended Message Signaled Interrupts | 消息信号中断，PCIe 设备通过内存写触发中断。 |
| IRQ | Interrupt Request | 中断请求。 |
| RPS/XPS | Receive Packet Steering / Transmit Packet Steering | Linux 网络收/发包 CPU steering 机制。 |
| UCX | Unified Communication X | HPC 通信框架，常被 MPI 用于选择共享内存、RDMA、TCP 等传输。 |
| libfabric | OpenFabrics Interfaces library | OFI 通信库，抽象多种高性能网络。 |

### 8.4 数学库、并行模型和算法 kernel

| 缩写/名称 | 英文全称 | 中文解释 |
|---|---|---|
| MPI | Message Passing Interface | 消息传递接口，多进程/多节点 HPC 的主流通信标准。 |
| OpenMP | Open Multi-Processing | 共享内存并行编程模型，常用于单节点多线程。 |
| BLAS | Basic Linear Algebra Subprograms | 基础线性代数子程序，包含 GEMM、TRSM、AXPY、DOT 等。 |
| LAPACK | Linear Algebra Package | 高级线性代数库，构建在 BLAS 之上。 |
| FFT | Fast Fourier Transform | 快速傅里叶变换。 |
| DGEMM | Double-precision General Matrix-Matrix multiplication | 双精度通用矩阵乘，HPL 中最重要的高重用计算 kernel。 |
| TRSM | Triangular Solve with Multiple right-hand sides | 三角矩阵多右端求解，HPL panel/update 路径常用。 |
| LU | Lower-Upper factorization | LU 分解，把矩阵分解为下三角和上三角。 |
| GMRES | Generalized Minimal Residual method | 广义最小残量迭代法，常用于线性系统迭代修正。 |
| PCG | Preconditioned Conjugate Gradient | 预条件共轭梯度法，HPCG 的主迭代框架。 |
| MG | Multigrid | 多重网格方法，用粗细网格层级加速收敛。 |
| SpMV | Sparse Matrix-Vector multiplication | 稀疏矩阵向量乘，低算术强度、强依赖内存层次。 |
| SpSV | Sparse Triangular Solve | 稀疏三角求解，数据依赖强，通常难以完全并行。 |
| SYMGS | Symmetric Gauss-Seidel | 对称 Gauss-Seidel 平滑器，HPCG 预条件中的关键部分。 |
| DDOT | Double-precision Dot Product | 双精度向量点积，多节点时通常跟 `MPI_Allreduce` 绑定。 |
| WAXPBY | Weighted A X Plus B Y | 加权向量更新，常写作 `w = alpha*x + beta*y`。 |
| CSR | Compressed Sparse Row | 压缩稀疏行格式。 |
| SELL | Sliced ELLPACK | 分片 ELLPACK 稀疏格式，改善向量化和连续访问。 |
| PTRANS | Parallel Matrix Transpose | 并行矩阵转置，常用于测试成对通信和全局数据重排。 |

### 8.5 精度和指标

| 缩写/名称 | 英文全称 | 中文解释 |
|---|---|---|
| FP64/FP32/FP16 | 64/32/16-bit Floating Point | 64/32/16 位浮点格式。FP64 是传统科学计算核心指标。 |
| BF16 | Brain Floating Point 16-bit | 16 位 brain floating point，指数位较多，常用于 AI/HPC 混合精度。 |
| INT8 | 8-bit Integer | 8 位整数，常用于 AI 推理或量化计算。 |
| FLOP/s | Floating-point Operations Per Second | 每秒浮点运算次数。 |
| GFLOP/s | Giga Floating-point Operations Per Second | 每秒十亿次浮点运算。 |
| GB/s | Gigabytes Per Second | 每秒 GB 级数据吞吐，常用于内存/网络/存储带宽。 |
| IPC | Instructions Per Cycle | 每周期指令数，衡量 core 执行效率。 |
| TEPS/GTEPS | Traversed Edges Per Second / Giga Traversed Edges Per Second | 图遍历每秒访问边数/十亿边数，Graph500 指标。 |
| V&V | Verification and Validation | 验证与确认，确认结果正确性和测试有效性。 |

## 9. 需要 Boss 后续确认的问题

为了把报告转成可执行测试计划，后续建议确认：

- 目标平台是 NVIDIA Grace、Ampere Altra/One、Fujitsu A64FX、AWS Graviton、国产 Arm，还是自研 Arm HPC CPU？
- 重点是芯片 bring-up、系统架构分析、采购对比，还是对外 benchmark 结果？
- 测试对象是单 socket、双 socket、单节点多 NUMA，还是多节点集群？
- 是否包含 GPU/加速器，还是严格 CPU-only？
- 是否允许运行长时间压力测试并生成构建/运行产物？

## 10. 参考资料

- HPL 官方说明：https://netlib.org/benchmark/hpl/
- HPL GitHub 参考源码：https://github.com/icl-utk-edu/hpl
- HPL-MxP 官方说明：https://hpl-mxp.org/
- HPL-AI/HPL-MxP 参考源码：https://bitbucket.org/icl/hpl-ai
- HPCG 官方软件页：https://www.hpcg-benchmark.org/software/index.html
- STREAM 官方页：https://www.cs.virginia.edu/stream/
- STREAM 官方源码目录：https://www.cs.virginia.edu/stream/FTP/Code/
- HPCC 官方页：https://hpcchallenge.org/hpcc/
- OSU Micro-Benchmarks：https://mvapich.cse.ohio-state.edu/benchmarks/
- Graph500 官方页：https://graph500.org/
- NAS Parallel Benchmarks：https://www.nas.nasa.gov/software/npb.html
- SPEC CPU 2026：https://www.spec.org/cpu2026/
- SPEC CPU 2017：https://www.spec.org/cpu2017/
- SPEChpc 2021：https://www.spec.org/hpc2021/
- LIKWID：https://github.com/RRZE-HPC/likwid
- lmbench：https://lmbench.sourceforge.net/
- IOR/mdtest：https://github.com/hpc/ior
