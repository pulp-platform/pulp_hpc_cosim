#!/bin/bash

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
	LD_PRELOAD="./mosquitto/lib/libmosquitto.so.1 ./compiled/lib/libssl.so.3 ./compiled/lib/libcrypto.so.3" ./local_simulation
fi