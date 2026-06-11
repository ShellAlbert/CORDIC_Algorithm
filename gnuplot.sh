#!/bin/bash

rm -rvf combined_plot.png
# gnuplot <<EOF
# # 1. 设置终端为 pngcairo，并指定输出文件名
# set terminal pngcairo size 800,600 enhanced font "Arial,12"
# set output "combined_plot.png"

# # 2. 设置标题和标签
# set title "Cos and Sin Waves"
# set xlabel "Angle"
# set ylabel "Value"
# set grid

# # 3. 合并绘图命令
# plot "cos0_360.dat" using 1:2 with linespoints title "cos", \
#      "sin0_360.dat" using 1:2 with linespoints title "sin"
# EOF

# # 4. 确认文件已生成
# if [ -f "combined_plot.png" ]; then
#     echo "Plot successfully saved to combined_plot.png"
# else
#     echo "Error: Plot file was not created."
# fi

#-p, --persist lets plot windows survive after main gnuplot program exits.
#-e "command list" executes the requested commands before loading the next input file.
#plot 'Rx.DAT.TXT' using 1:2 linespoints pointsize 1 linewidth 1 title 'NO\_HV'"
gnuplot -persist \
        -e "set term wxt font 'Arial,12'; \
        set border lw 1.5;
        set grid lc rgb '#b0b0b0' lw 1;
        set title 'CORDIC Alogirithm (Cos and Sin)'; \
        set xrange [0:500]; set yrange [-65535:65535]; \
        set xlabel 'Time/Samples'; set ylabel 'Voltage'; \
        set label 'Comparison' at 100,1000 tc rgb 'blue' center; \
        plot 'cos0_360.dat' using 1:2 with linespoints linewidth 1 title 'cos',\
        'sin0_360.dat' using 1:2 with linespoints linewidth 1 title 'sin' "

