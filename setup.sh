#!/bin/sh

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

E_OPTERR=65

args=$(getopt -l "compiler:,legrev" -o "c,h" -- "$@")
eval set -- "$args"

while [ $# -ge 1 ]; do
        case "$1" in
                --)
                    # No more options left.
                    shift
                    break
                   ;;
                -h)
					printf "Usage: $0\n\r\t [-c] \t\t --> to cross compile for arm64\n\r\t [--compiler] \t --> to cross-compile with a certain compiler (/bin/.../-linux-gnu-)\n\r"
					exit
					;;
				-c)
					echo "Cross-compile for arm64"
					CROSSCOMPILE=true
					;;
				--compiler)
					echo "Cross-compile using $2"
					CROSSCOMPILE=true
					COMPILER_PATH="$2"
					shift
					;;
				--legrev)
					echo "Cross-compile for legrev"
					CROSSCOMPILE=true
					COMPILER_PATH="/usr/scratch2/gransasso/scratch_from_maipo/mettleti/hero/install/bin/aarch64-none-linux-gnu-"
        esac

        shift
done

shift $(($OPTIND - 1)) # $OPTIND is the number of options found by getopts

if [ "${CROSSCOMPILE,,}" = "true" ]; then
	COMPILER_PATH=${COMPILER_PATH:="aarch64-linux-gnu-"}
	#echo "$COMPILER_PATH"
fi


git submodule update --init

mkdir -p compiled/
INSTALL_DIR="$(pwd)/compiled"
cd openssl/

# if test ! -f "$(which h5dump)"; then
#    printf "\n\033[0;31mYou have to install hdf5 to continue with the compilation\e[0m\n"
#    exit
# fi

if [ "${CROSSCOMPILE,,}" = "true" ]; then
#    if test ! -f "$(which aarch64-linux-gnu-gcc)" ;then
#        printf "\n\033[0;31mYou have to install \"aarch64-linux-gnu-gcc\" to continue with the cross-compilation\e[0m\n";
#        exit
#    fi
    ./Configure linux-generic32 shared --prefix="$INSTALL_DIR" \
        --openssldir="$INSTALL_DIR/openssl" --cross-compile-prefix=$COMPILER_PATH
	touch .cross_compiled
else
    ./Configure linux-generic32 shared --prefix="$INSTALL_DIR" \
        --openssldir="$INSTALL_DIR/openssl"
	rm -f .cross_compiled
fi
make depend
make
make install

cd ../
cd mosquitto/
cd lib/
if [ "${CROSSCOMPILE,,}" = "true" ]; then
    #make clean binary CROSS_COMPILE=$COMPILER_PATH CC=gcc LIB_CFLAGS="-I../openssl/include/ -fPIC" LIB_LIBADD=-L../compiled/lib WITH_STATIC_LIBRARIES=yes WITH_CJSON=no &&
    make all CROSS_COMPILE=$COMPILER_PATH CC=gcc LIB_CFLAGS="-I../../openssl/include/ -fPIC" LIB_LIBADD=-L../../compiled/lib WITH_STATIC_LIBRARIES=yes &&
        printf "\n\033[0;32m----- DONE CROSSCOMPILE -----\e[0m\n\n"
        touch .cross_compiled
else
    #make clean binary CC=gcc LIB_CFLAGS="-I../openssl/include/ -fPIC" LIB_LIBADD=-L../compiled/lib WITH_STATIC_LIBRARIES=yes WITH_CJSON=no &&
    make clean all CC=gcc LIB_CFLAGS="-I../../openssl/include/ -fPIC" LIB_LIBADD=-L../../compiled/lib WITH_STATIC_LIBRARIES=yes &&
        printf "\n\033[0;32m----- DONE -----\e[0m\n\n"
        rm -f .cross_compiled
fi
cd ../../
