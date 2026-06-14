# -*- Makefile -*-

# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# by default, "arch" is unknown, should be specified in the command line
arch = UNKNOWN

setup_file = setup/Make.$(arch)
include $(setup_file)

bin_name='bin/xhpcg'

ifeq ($(USE_CUDA), 0)
     ifneq ($(USE_GRACE), 0)
          bin_name='bin/xhpcg-cpu'
     endif
endif

HPCG_DEPS = src/CG.o src/CG_ref.o src/TestCG.o src/ComputeResidual.o \
         src/ExchangeHalo.o src/GenerateGeometry.o src/GenerateProblem.o \
         src/GenerateProblem_ref.o src/CheckProblem.o \
	 src/OptimizeProblem.o src/ReadHpcgDat.o src/ReportResults.o \
	 src/SetupHalo.o src/SetupHalo_ref.o src/TestSymmetry.o src/TestNorms.o src/WriteProblem.o \
         src/YAML_Doc.o src/YAML_Element.o src/ComputeDotProduct.o \
         src/ComputeDotProduct_ref.o src/finalize.o src/init.o src/mytimer.o src/ComputeSPMV.o \
         src/ComputeSPMV_ref.o src/ComputeSYMGS.o src/ComputeSYMGS_ref.o src/ComputeWAXPBY.o src/ComputeWAXPBY_ref.o \
         src/ComputeMG_ref.o src/ComputeMG.o src/ComputeProlongation_ref.o src/ComputeRestriction_ref.o src/GenerateCoarseProblem.o \
	 src/ComputeOptimalShapeXYZ.o src/MixedBaseCounter.o src/CheckAspectRatio.o src/OutputFile.o \
     src/ComputeProlongation.o src/ComputeRestriction.o

$(bin_name): src/main.o $(HPCG_DEPS)
	$(LINKER) $(LINKFLAGS) src/main.o $(HPCG_DEPS) -o $(bin_name) $(HPCG_LIBS)
	@[ "$(STRIP_SYMBOLS)" != "1" ] || $(STRIP) --strip-all $(bin_name)

install:
	cp build/bin/xhpcg* bin/

clean:
	rm -f $(HPCG_DEPS) $(bin_name) src/main.o

.PHONY: clean

