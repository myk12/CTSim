#!/bin/bash

function usage {
    echo "Usage: $0 [OPTION]..."
    echo "Run the FISIM mobility test"
    echo ""
    echo "  -h, --help              Print this help"
    echo "  -v, --verbose           Print debug output"
    echo ""
    echo "Example: $0"
    exit 1
}

function Init {
     # prepare the environment
     echo "******* 0 - Prepare the environment *******\n"
     OUTPUT_DIR="/tmp/mobility-test/"
     OUTPUT_DIR_ON="$OUTPUT_DIR/Routing_On/"
     OUTPUT_DIR_OFF="$OUTPUT_DIR/Routing_Off/"
     rm -rf $OUTPUT_DIR
     mkdir -p $OUTPUT_DIR
     mkdir -p $OUTPUT_DIR_ON
     mkdir -p $OUTPUT_DIR_OFF
     # change working directory to the root of the project
     cd ../
}

function RunSimulation {
     echo "******* 1 - Run the simulation *******\n"
     ./ns3 clean
     # run the simulation
     if [ $1 == "ON" ]; then
          echo "******* 1 - Run the simulation with ID First Routing ON *******\n"
          sed -i 's/option(FISIM_NAME_FIRST_ROUTING "Use FISIM Name First Routing" OFF)/option(FISIM_NAME_FIRST_ROUTING "Use FISIM Name First Routing" ON)/g' CMakeLists.txt
          ./ns3 configure --disable-examples --disable-tests --disable-python --disable-mpi
          ./ns3 build
          ./ns3 run scratch/mobility_test.cc -- --outpath="$OUTPUT_DIR_ON" --random_seed=177
     else
          echo "******* 2 - Run the simulation with ID First Routing OFF *******\n"
          sed -i 's/option(FISIM_NAME_FIRST_ROUTING "Use FISIM Name First Routing" ON)/option(FISIM_NAME_FIRST_ROUTING "Use FISIM Name First Routing" OFF)/g' CMakeLists.txt
          ./ns3 configure --disable-examples --disable-tests --disable-python --disable-mpi
          ./ns3 build
         ./ns3 run scratch/mobility_test.cc -- --outpath="$OUTPUT_DIR_OFF" --random_seed=147
     fi
}

############# STEP [0] #############
echo "===================== FISIM Mobility TEST =====================\n"
echo ""
if [ $1 == "Sim" ]; then
     Init
     RunSimulation "OFF"
     RunSimulation "ON"
fi

############# STEP [3] #############
echo "******* 3 - Process the results *******\n"

python3 -c '
import os
import pandas as pd
import matplotlib.pyplot as plt

time_interval = 0.07

#first process the results with ID First Routing off
output_dir_off = "/tmp/mobility-test/Routing_Off"
df_off_1 = pd.read_csv(output_dir_off + "/receiver_2.log", sep=",")
df_off_2 = pd.read_csv(output_dir_off + "/receiver_3.log", sep=",")
df_off = pd.concat([df_off_1, df_off_2], ignore_index=True)
print(df_off)
df_off_plt = df_off.groupby(pd.cut(df_off["time"], bins=pd.interval_range(start=0, end=df_off["time"].max(), freq=time_interval)))["packet_size"].sum().reset_index()

#then process the results with ID First Routing on
output_dir_on = "/tmp/mobility-test/Routing_On"
df_on_1 = pd.read_csv(output_dir_on + "/receiver_2.log", sep=",")
df_on_2 = pd.read_csv(output_dir_on + "/receiver_3.log", sep=",")
df_on = pd.concat([df_on_1, df_on_2], ignore_index=True)
print(df_on)
df_on_plt = df_on.groupby(pd.cut(df_on["time"], bins=pd.interval_range(start=0, end=df_on["time"].max(), freq=time_interval)))["packet_size"].sum().reset_index()

df_off_plt["time"] = df_off_plt["time"].apply(lambda x: x.left)
df_on_plt["time"] = df_on_plt["time"].apply(lambda x: x.left)
df_off_plt["Throughput"] = df_off_plt["packet_size"] * 8 / 1000000 / time_interval
df_on_plt["Throughput"] = df_on_plt["packet_size"] * 8 / 1000000 / time_interval

# save time, throughput to csv file
df_off_plt.to_csv("mobility_test_off.csv", index=False)
df_on_plt.to_csv("mobility_test_on.csv", index=False)
'

############# STEP [4] #############
# plot using gnuplot
echo "******* 4 - Plot the results *******\n"
#!/bin/bash

# 直接嵌入gnuplot脚本
gnuplot << EOF
# plot_script.gp

# 设置绘图样式
set terminal pdfcairo enhanced color size 8cm, 4cm font 'Arial, 10, bold' 
#set object 1 rectangle from graph 0,0 to graph 1,1 behind fillcolor rgb '#EEEEEE' fillstyle solid noborder
set output 'plot.pdf'
unset border

set datafile separator ","

set yrange [0:2]
set xrange [0:4]

# 设置轴标签
set xlabel 'Time (s)'
set ylabel 'Data rate (Mbps)'
set grid

# set bold line

# 绘制折线图
# plot a line in black
set object 2 rect from 1, 0 to 1.1, 2 fc rgb "#9E9E9E" fs solid border lw 1
set object 3 rect from 2, 0 to 2.1, 2 fc rgb "#9E9E9E" fs solid border lw 1
set object 4 rect from 3, 0 to 3.1, 2 fc rgb "#9E9E9E" fs solid border lw 1

set label "movement" at 0.5, 1.7 center tc rgb "#555555" font "Arial, 10, bold"
set arrow from 0.7, 1.6 to 1, 1.5 lc rgb "#555555" lw 2

plot 'mobility_test_off.csv' using 1:3 with lines  lw 3 lc "#0072BB" title 'W/o ID-aware Routing', \
     'mobility_test_on.csv' using 1:3 with lines dashtype '-' lt 2 lw 3 lc "#4CAF50" title 'W/ ID-aware Routing'
EOF

