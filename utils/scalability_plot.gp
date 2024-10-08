set terminal pdfcairo enhanced color size 9cm, 6cm linewidth 2

set datafile separator ","

unset border
set grid

# 设置柱状图参数
set style data histograms
set style fill solid
set boxwidth 0.5
set xtic rotate by -45 scale 0

set style histogram cluster gap 1
set style fill pattern 1

set xlabel "Number of Nodes"
# 绘制第二列柱状图
set ylabel "Runtime (s)"
set output 'runtime.pdf'
set arrow from graph 0, 0.04 to graph 1,0.04 nohead lw 1 lt 3 lc rgb "#9A9A9A"
plot 'output.csv' every ::1 using 2:xtic(1) notitle with histograms lc rgb "#0072BB"


set output 'memory.pdf'
set yrange [0:250]
set ylabel "Memory Usage (MB)"
unset arrow
plot 'output.csv' every ::1 using (column(3)/1024):xtic(1)  notitle with histograms lc rgb "#4CAF50"
