
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
import os
import matplotlib
import tkinter
# matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.animation import FuncAnimation
import time
import argparse
import pandas as pd
import numpy as np


# parser = argparse.ArgumentParser(description= """Plot control policy output data stream from PMS emulation on FPGA in real time""")
#
# parser.add_argument(
#     '--inp',
#     '-i',
#     required=True,
#     help='Location of the input file where data are collected over MQTT.')
#
# args = parser.parse_args()
# file_path = args.inp
# _, data = os.path.split(file_path)

def parse_csv():
    # Prepare to write some analysis on data
    f = open('core_group_analysis.csv', 'w')
    writer = csv.writer(f)
    writer.writerow(["Core", "Tmax_t", "Tmean_t"])

    for idx, col in enumerate(df.columns):
        if idx > 0 and idx < num_cores+1:
            temp.append(df[col])
            temp_max.append(df[col].max())
            temp_mean.append(df1[col].mean())
            row = [f"{idx-1}", f"{temp_max[idx-1]}", f"{temp_mean[idx-1]}"]
            writer.writerow(row)

    f.close()

def print_temp_subplt(i):
    line1, = axs[i].plot(time, temp[i], color='b', linewidth=0.8)
    line2 = axs[i].axhline(y = temp_max[i], color = 'red', linestyle = 'dashed', linewidth=0.8)
    line3 = axs[i].axhline(y = temp_target, color = 'green', linestyle = 'dashed', linewidth=0.8)
    line4 = axs[i].axhline(y = temp_mean[i], color = 'black', linestyle = 'dashed', linewidth=0.8)
    axs[i].legend(['T[{}]'.format(i), 'T[{}]_max'.format(i), 'T_target', 'T[{}]_mean'.format(i)], prop={"size":6})
    axs[i].set_xlabel('Time')
    axs[i].set_ylabel('T[{}]'.format(i))
    axs[i].ticklabel_format(axis="x", style="sci", scilimits=(0,0)) # scientific notation for time, in processor clock cycles
    plt.subplots_adjust(hspace=0.45, wspace=0.45)

def print_temp_plt(ax, i):
    line1, = ax.plot(time, temp[i], color='b', linewidth=0.8)
    line2 = ax.axhline(y = temp_max[i], color = 'red', linestyle = 'dashed', linewidth=0.8)
    line3 = ax.axhline(y = temp_target, color = 'green', linestyle = 'dashed', linewidth=0.8)
    line4 = ax.axhline(y = temp_mean[i], color = 'black', linestyle = 'dashed', linewidth=0.8)
    ax.legend(['T[{}]'.format(i), 'T[{}]_max'.format(i), 'T_target', 'T[{}]_mean'.format(i)])
    ax.set_xlabel('Time')
    ax.set_ylabel('T[{}]'.format(i))
    plt.show()

def animate_subplots(i):
    parse_csv()
    for i in range(h_plots*v_plots):
        print_temp_subplt(i)

def animate_multi(nfigs):
    anims = []

    for n in range(nfigs):
        fig = plt.figure(num=nfigs)
        ax = fig.add_subplot(111)

        # animation function. This is called sequentially
        def animate_plot(i):
            print_temp_plt(ax, n)

        anim = FuncAnimation(fig, animate_plot, interval=1000)
        anims.append(anim)

    return anims


# Increase number of rows of the dataframe
pd.options.display.max_rows = 9999

# Open dataframe
df = pd.read_csv('output.csv')

# Convert df to float appropriately
df1 = df.apply(pd.to_numeric, errors='coerce')

# Variables definition
multi = 0
num_cores = 36
h_plots = 6
v_plots = int(num_cores/h_plots)
time = df['MId']
temp = []
temp_max = []     # T_max over time
temp_mean = []    # T_mean over time
temp_target = 348 # T_target

# Parse and manipulate dataframe
# Save some important results into a csv file
parse_csv()

# Real-time plots

# Subplots
if multi == 0:
    fig, axs = plt.subplots(h_plots, v_plots, figsize=(100,150))
    axs = axs.ravel()

while True:
    # Wait for file creation
    if(os.path.isfile('output.csv')):
        if multi == 0:
            plt.ion()
            # blit = True means the function re-draws only the parts that have changed -> speedup
            # TODO: fix blit option!
            ani = FuncAnimation(fig, animate_subplots, interval=1000)
            plt.pause(1) # Number of seconds you wait to update the plot
            #fig.tight_layout()

        else:
            plt.ioff()
            my_anims = animate_multi(nfigs=num_cores)
            plt.show()

            # TODO: save animation once program is ended wih KeyBoard exception
            #for idx, animation in enumerate(my_anims):
            #    animation.save('./save/T_{}.gif'.format(idx), writer='imagemagick', fps=60)
    else:
        print("Waiting for file creation")
