#!/usr/bin/env python3

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

import csv
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.animation import FuncAnimation
import pandas as pd
import numpy as np

import io
import tailer

from itertools import count

x_vals = []
y_vals = []
num_cores = 36
n_instrs = 4
h_plots = 6
v_plots = int(num_cores/h_plots)
temp_target = 348 # T_target
temp_amb = 273.1 + 25.0
colorI = ['green', 'cyan', 'b', 'red']
#temp_mean = np.empty((num_cores, 2)) #[temp_amb]*num_cores
#for i in range(num_cores):
#	temp_mean[i,0] = temp_amb
    #app = [y1.mean(), x1[-1]]
    #np.append(temp_mean[i], app)
figT, axsT = plt.subplots(h_plots, v_plots, figsize=(100,150))
axsT = axsT.ravel()
figF, axsF = plt.subplots(h_plots, v_plots, figsize=(100,150))
axsF = axsF.ravel()
figI, axsI = plt.subplots(h_plots, v_plots, figsize=(100,150))
axsI = axsI.ravel()

rT = lambda x: x if float(x) > 0.0 else 298.1 #float(x.replace('0', '298.1'))
sR = lambda x: x in [1, 2, 3, 4, 5, 6, 7]

avoid0_dict = {'Temp[0]' : rT}

def draw_subplotsT(x1, y1, i):
    axsT[i].cla() #clear axis
    line1, = axsT[i].plot(x1, y1, color='b', linewidth=0.8)
    temp_max = y1.max()
    temp_mean = y1.mean()
    line2 = axsT[i].axhline(y = temp_max, color = 'red', linestyle = 'dashed', linewidth=0.8)
    line3 = axsT[i].axhline(y = temp_target, color = 'green', linestyle = 'dashed', linewidth=0.8)
    line4 = axsT[i].axhline(y = temp_mean, color = 'black', linestyle = 'dashed', linewidth=0.8)
    #axsT[i].legend(['T[{}]'.format(i), 'T[{}]_max'.format(i), 'T_target', 'T[{}]_mean'.format(i)], prop={"size":6}, loc='lower left')
    axsT[i].set_xlabel('Time')
    axsT[i].set_ylabel('T[{}]'.format(i))
    axsT[i].ticklabel_format(axis="x", style="sci", scilimits=(0,0)) # scientific notation for time, in processor clock cycles
    plt.subplots_adjust(hspace=0.45, wspace=0.45)
    
def draw_subplotsF(x1, y1, y2, i):
    axsF[i].cla() #clear axis
    line1, = axsF[i].plot(x1, y1, color='b', linewidth=0.8)
    freq_max = y1.max()
    freq_mean = y1.mean()
    line2 = axsF[i].axhline(y = freq_max, color = 'red', linestyle = 'dashed', linewidth=0.8)
    line3 = axsF[i].plot(x1, y2, color = 'green', linestyle = 'dashed', linewidth=0.8)
    line4 = axsF[i].axhline(y = freq_mean, color = 'black', linestyle = 'dashed', linewidth=0.8)
    #axsF[i].legend(['F[{}]'.format(i), 'F[{}]_max'.format(i), 'F_target', 'F[{}]_mean'.format(i)], prop={"size":6}, loc='lower left')
    axsF[i].set_xlabel('Time')
    axsF[i].set_ylabel('F[{}]'.format(i))
    axsF[i].ticklabel_format(axis="x", style="sci", scilimits=(0,0)) # scientific notation for time, in processor clock cycles
    plt.subplots_adjust(hspace=0.45, wspace=0.45)
    
def draw_subplotsI(x1, y1, i, j):
    #axsI[i].cla() #clear axis
    #line1 = []
    #for j in range(n_instrs):
        #line1[j] = axsI[i].plot(x1, y1[j], color='b', linewidth=0.8)
    line1 = axsI[i].plot(x1, y1, color=colorI[j], linewidth=0.8)
    line2 = axsI[i].axhline(y = y1.mean(), color=colorI[j], linestyle = 'dashed', linewidth=0.8)
    
    #axsI[i].legend(['I[{}]'.format(i), 'I[{}]_max'.format(i), 'I_target', 'I[{}]_mean'.format(i)], prop={"size":6}, loc='lower left')
    axsI[i].set_xlabel('Time')
    axsI[i].set_ylabel('I[{}]'.format(i))
    axsI[i].ticklabel_format(axis="x", style="sci", scilimits=(0,0)) # scientific notation for time, in processor clock cycles
    plt.subplots_adjust(hspace=0.45, wspace=0.45)


def animateT(i):
    try:
        data = pd.read_csv('output.csv', skipinitialspace=True, skiprows=sR) #, dtype=float) # converters=avoid0_dict) #engine='c',
        x_vals = data['MId']
        for i in range(h_plots*v_plots):
            y_vals = data['Temp[' + str(i) + ']']
            draw_subplotsT(x_vals, y_vals, i)
		  
    except pd.errors.EmptyDataError:
        print("Waiting for file")
        
def animateF(i):
    try:
        data = pd.read_csv('output.csv', skipinitialspace=True, skiprows=sR) #, dtype=float) # converters=avoid0_dict) #engine='c',
        x_vals = data['MId']
        for i in range(h_plots*v_plots):
            y1_vals = data['Core_freq[' + str(i) + ']']
            y2_vals = data['Target_freq[' + str(i) + ']']
            draw_subplotsF(x_vals, y1_vals, y2_vals, i)
		  
    except pd.errors.EmptyDataError:
        print("Waiting for file")
        
def animateI(i):
    try:
        data = pd.read_csv('output.csv', skipinitialspace=True, skiprows=sR) #, dtype=float) # converters=avoid0_dict) #engine='c',
        x_vals = data['MId']
        #y_vals = []
        for i in range(h_plots*v_plots):
            axsI[i].cla() #clear axis
            for j in range(n_instrs):
                y_vals = data['Instr[' + str(i) + '-' + str(j) + ']']
                draw_subplotsI(x_vals, y_vals, i, j)
		  
    except pd.errors.EmptyDataError:
        print("Waiting for file")


aniT = FuncAnimation(figT, animateT, interval=2000) #, blit=True, init_
aniF = FuncAnimation(figF, animateF, interval=2000) 
aniI = FuncAnimation(figI, animateI, interval=2000) 

plt.tight_layout()
plt.show()
