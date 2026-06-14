# HPC CPU Benchmark Arm 架构分析报告目录

生成日期：2026-06-14

本目录用于存放面向 HPC CPU，特别是 Arm 架构服务器/HPC CPU 的 benchmark 分析材料。可公开取得的 benchmark 源码快照单独放在项目根目录 `external_sources/`，用于扩展分析；未修改仓库现有 benchmark 源码。

## 文件说明

- `01_hpc_cpu_benchmark_arm_architecture_report.md`：主流 HPC CPU benchmark、测试项、测试对象、测试原理、测试目的，以及 Arm 架构组件映射。
- `02_nvidia_hpc_benchmarks_arm_cpu_source_analysis.md`：NVIDIA HPC-Benchmarks 文档中适用于 Arm CPU/Grace CPU 的部分，以及 NVIDIA HPCG、HPL、HPL-MxP/HPL-AI、STREAM、OSU Micro-Benchmarks 的源码分析。

## 外部源码目录

| 本地目录 | 来源 | 版本/快照 | 结论边界 |
|---|---|---|---|
| `../../external_sources/nvidia-hpcg/` | https://github.com/NVIDIA/nvidia-hpcg | `7dd63cd` | NVIDIA 优化 HPCG 源码，可做 Grace-only CPU 路径源码分析 |
| `../../external_sources/hpl-reference/` | https://github.com/icl-utk-edu/hpl | `9edeab5` | Netlib/ICL HPL 参考实现，用于解释 HPL 原理，不等同 NVIDIA Grace 优化二进制 |
| `../../external_sources/hpl-mxp-reference/` | https://bitbucket.org/icl/hpl-ai | `8c7ab18` | HPL-AI/HPL-MxP 参考实现，用于解释混合精度算法，不等同 NVIDIA HPL-MxP 优化二进制 |
| `../../external_sources/stream-reference/` | https://www.cs.virginia.edu/stream/FTP/Code/ | 官方 `stream.c`/`stream.f` 源码目录，文件时间 2021-11-29 | STREAM 参考源码，用于解释 Copy/Scale/Add/Triad 原理 |
| `../../external_sources/osu-micro-benchmarks-7.5.2/` | https://mvapich.cse.ohio-state.edu/benchmarks/ | OSU Micro-Benchmarks 7.5.2，发布日 2025-12-05；官方 tarball 保留为 `../../external_sources/osu-micro-benchmarks-7.5.2.tar.gz` | OSU 官方源码包，用于解释 MPI latency/bw/collective/one-sided 测试 |

## 关键边界

- 当前仓库的 `src/`、`setup/`、`tools/`、`unittesting/`、`web/` 等 benchmark 源码和工程文件未修改。
- NVIDIA HPC-Benchmarks 文档中，HPL、HPL-MxP、HPCG、STREAM 均有 Arm SBSA/Grace CPU 相关说明；公开的 NVIDIA 优化源码只确认到 `NVIDIA/nvidia-hpcg`。HPL、HPL-MxP、STREAM 的源码级分析基于上游参考实现，不能直接等同 NVIDIA Grace 优化二进制。
- 报告中把 CPU 适用项、GPU/NVIDIA 专用项、MPI/通信辅助项、正确性验证项分开，避免把 GPU 或 CUDA 专属测试误写为通用 Arm CPU 测试。
- 两份分析报告均补充了专业英语名称全称和术语解析；正文后续仍使用常见缩写以保持可读性。
