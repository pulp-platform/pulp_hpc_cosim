#!/bin/bash

##########################################################################
#
# Copyright 2023 ETH Zurich and University of Bologna
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
# Author: Giovanni Bambini (gv.bambini@gmail.com)
#
##########################################################################

if test -f "./mosquitto/lib/.cross_compiled" ;then
	printf "\n\033[0;32m----- Compiling CrossPlatform -----\e[0m\n\n"
	case $1 in
		legrev)
			#export FPGA_COMP_PATH=/usr/scratch2/gransasso/scratch_from_maipo/aottaviano/cpulp-linux-trial-1/install/bin/
			COMPILER_PATH=/usr/scratch2/gransasso/scratch_from_maipo/mettleti/hero/install/bin/aarch64-none-linux-gnu-
			COMP_FLAGS="-std=c99"
			;;
		local)
			COMPILER_PATH=arm-linux-gnueabi-
			;;
		*)
			COMPILER_PATH=aarch64-linux-gnu-
			;;
	esac

	#COMP_FLAGS=${COMP_FLAGS:=""}
	make clean all CROSS_COMPILE=$COMPILER_PATH MAKE_FLAGS=$COMP_FLAGS
else
    make clean all
	LD_PRELOAD="./mosquitto/lib/libmosquitto.so.1 ./compiled/lib/libssl.so.3 ./compiled/lib/libcrypto.so.3" ./linux_simulation
fi
