set terminal svg enhanced size 800, 600 background rgb 'white';
set datafile separator ',';
set title 'Efficiency vs. Number of Threads';
set xlabel 'Number of Threads';
set ylabel 'Efficiency';
set output 'efficiency.svg'
plot \
'efficiency-1000.csv' using 1:2 with linespoints lw 2 pt 7 lc rgb 'red' title 'N Value: 10^{3}', \
'efficiency-1000000.csv' using 1:2 with linespoints lw 2 pt 7 lc rgb 'blue' title 'N Value: 10^{6}'
