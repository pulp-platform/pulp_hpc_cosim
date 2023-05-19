
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


APP_SRCS1 = ./*.c

APP_SRCS1 += ../Libraries/MQTT-lib/src/*.c
APP_SRCS1 += ../Libraries/MySemaphore/*.c

INC_PATH1 = ./ #../System_Simulation/include/
INC_PATH1 += ../Libraries/MQTT-lib/include/
INC_PATH1 += ../Libraries/MySemaphore/


### Mosquitto
INC_PATH1 += ../mosquitto/include
INC_PATH1 += ../mosquitto/lib
INC_PATH1 += ../compiled/lib
#LIBMOSQ = /home/osboxes/mosquitto-x86-64/lib/libmosquitto.so.1
#MOSQUITTO = /home/osboxes/mosquitto-x86-64/
#INC_PATH += /home/osboxes/mosquitto-x86-64/lib /home/osboxes/mosquitto-x86-64/include
CFLAGS += -Lmosquitto/lib/ -Lcompiled/lib ../mosquitto/lib/libmosquitto.so.1 ../compiled/lib/libcrypto.so.3 ../compiled/lib/libssl.so.3 #-lssl -lcrypto

#Pthread
CFLAGS += -lpthread -lrt
# #define _GNU_SOURCE
CFLAGS += -D_GNU_SOURCE

CFLAGS += #-DUSE_FILE_DB #-UUSE_FILE_DB 
CFLAGS += -DUSE_EXAMON -DUSE_EXAMON_SIM_TIME

CFLAGS += -fcommon #-std=c99

#.PHONY: all
#all: linux_simulation

#linux_simulation: $(APP_SRCS) $(CXX) -lDIR -Wall -IINC_PATH $< -o $@

#.PHONY: clean
#clean:
#	rm -f linux_simulation

# Personalize
#MY_FLAGS += -DDEBUG_ACTIVE
# MY_FLAGS += -DUSE_INSTRUCTIONS_COMPOSITION
#
# #Code
# MY_FLAGS += -DUSE_MYSEM
#
# NAME=linux_simulation
#
# CROSS_COMPILE = $(FPGA_COMP_PATH_JO)/aarch64-none-linux-gnu-
#
# CC=$(CROSS_COMPILE)gcc
# AR=$(CROSS_COMPILE)ar
# OB=$(CROSS_COMPILE)objdump
#
# CFLAGS += -Wall -O2 -DPLATFORM=${PLATFORM}
#
# #REALLY IMPORTANT FLAGS!
# CFLAGS += -lpthread -lrt
# # #define _GNU_SOURCE
# CFLAGS += -D_GNU_SOURCE
#
# CFLAGS += $(MY_FLAGS)

INCLUDES = $(foreach f, $(INC_PATH1), -I$f)
OBJS = $(addsuffix .o, $(basename $(APP_SRCS1)))

#.PHONY: all build clean deploy dis

all: data_retriever data_retriever_local #fpga_publisher
	
# 		$(MAKE) -C $(MOSQUITTO)

data_retriever: $(APP_SRCS1)
	gcc $^ -o $@ $(INCLUDES) $(CFLAGS) -Wall $(MAKE_FLAGS) -DMQTT_BROKER_IP=\"127.0.0.1\" -DMQTT_USERNAME=\"a\" -DMQTT_PASSWD=\"b\" -DMQTT_BROKER_PORT=1111

data_retriever_local: $(APP_SRCS1)
	gcc $^ -o $@ $(INCLUDES) $(CFLAGS) -Wall $(MAKE_FLAGS) -DMQTT_BROKER_IP=\"127.0.0.1\" -DMQTT_USERNAME=\"a\" -DMQTT_PASSWD=\"b\" -DMQTT_BROKER_PORT=1111

clean:
	-rm -f data_retriever data_retriever_local
# all: build #deploy
#
# $(NAME): $(APP_SRCS)
# 	$(CC) $(CFLAGS) $(MAKE_FLAGS) $(INCLUDES) $^ -o $@
#
# build: $(NAME)
#
# deploy: build
# 	scp $(NAME) $(HERO_TARGET_HOST):$(HERO_TARGET_PATH_APPS)/
#
# dis: build
# 	$(OB) -dS $(NAME) > $(NAME).s
#
# clean:
# 	-rm -f $(NAME)
