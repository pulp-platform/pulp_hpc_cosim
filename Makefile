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

#Simulation
SRCS 		= System_Simulation/main.c
INC_PATH 	= System_Simulation/
SRCS 		+= System_Simulation/src/*.c
INC_PATH 	+= System_Simulation/include/

#Libraries
#MQTT
SRCS 		+= Libraries/MQTT-lib/src/*.c
INC_PATH 	+= Libraries/MQTT-lib/include/
#MySem
SRCS 		+= Libraries/MySemaphore/*.c
INC_PATH 	+= Libraries/MySemaphore/
#JSON
SRCS 		+= cJSON/cJSON.c
INC_PATH 	+= cJSON/
#CSV
SRCS 		+= csv-fast-reader/csv.c
INC_PATH 	+= csv-fast-reader/

#.PHONY: all
#all: linux_simulation

#linux_simulation: $(APP_SRCS) $(CXX) -lDIR -Wall -IINC_PATH $< -o $@

#.PHONY: clean
#clean:
#	rm -f linux_simulation

# Personalize
#MY_FLAGS += -DDEBUG_ACTIVE
MY_FLAGS += -DUSE_INSTRUCTIONS_COMPOSITION
MY_FLAGS += -DSCMI_WL_TIME_S=0.1 -DSCMI_TOP_FREQ=3.4 -DSCMI_CORE=2 -DHLC_TS_S=0.001 -DSCMI_SIM_TIME_S=10
MY_FLAGS += -UUSE_SCMI

MY_FLAGS += -DUSE_MQTT_SEND -UUSE_FILE_DB

MY_FLAGS += -UUSE_EXAMON

#Code
MY_FLAGS += -DUSE_MYSEM

NAME=linux_simulation

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
OB=$(CROSS_COMPILE)objdump

CFLAGS += -Wall -O2 -DPLATFORM=${PLATFORM}

### Mosquitto
INC_PATH += mosquitto/include
INC_PATH += mosquitto/lib
INC_PATH += compiled/lib
#INC_PATH += compiled/include/openssl
LIBS = -Lmosquitto/lib/ -Lcompiled/lib

#REALLY IMPORTANT FLAGS!
CFLAGS += -lpthread -lrt -lm #-lmosquitto -lm -L/usr/lib #-lhdf5_hl -lhdf5
# #define _GNU_SOURCE
CFLAGS += -D_GNU_SOURCE

CFLAGS += $(MY_FLAGS) #-Wall -g

CFLAGS += -DEXT_PWR_ACTIVE

INCLUDES = $(foreach f, $(INC_PATH), -I$f)
OBJS = $(addsuffix .o, $(basename $(SRCS)))
CFLAGS += $(INCLUDES) $(LIBS) mosquitto/lib/libmosquitto.so.1 compiled/lib/libcrypto.so.3 compiled/lib/libssl.so.3

#linker mapping:
#CFLAGS += -Map=output.map
CFLAGS += -fcommon
NETFLAGSL += -DMQTT_BROKER_IP=\"127.0.0.1\" -DMQTT_USERNAME=\"a\" -DMQTT_PASSWD=\"b\" -DMQTT_BROKER_PORT=1111
NETFLAGS += -DMQTT_BROKER_IP=\"127.0.0.1\" -DMQTT_USERNAME=\"a\" -DMQTT_PASSWD=\"b\" -DMQTT_BROKER_PORT=1111


.PHONY: all build clean deploy dis

all: build #deploy

$(NAME): $(SRCS)
	$(CC) $^ -o $@ $(CFLAGS) $(MAKE_FLAGS) $(ADD_COMP_FLAGS) $(NETFLAGS)

build: $(NAME) local_simulation

local_simulation: $(SRCS)
	$(CC) $^ -o $@ $(CFLAGS) $(MAKE_FLAGS) $(ADD_COMP_FLAGS) $(NETFLAGSL)

deploy: build
	scp $(NAME) $(HERO_TARGET_HOST):$(HERO_TARGET_PATH_APPS)/

dis: build
	$(OB) -dS $(NAME) > $(NAME).s

clean:
	-rm -f $(NAME)
	-rm -f local_simulation
