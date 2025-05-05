set terminal svg enhanced size 800, 600 background rgb 'white';
set datafile separator ',';
set title 'Speedup vs. Number of Threads';
set xlabel 'Number of Threads';
set ylabel 'Speedup';
set output 'speedup.svg'
plot \
'speedup-1000.csv' using 1:2 with linespoints lw 2 pt 7 lc rgb 'red' title 'N Value: 10^{3}', \
'speedup-1000000.csv' using 1:2 with linespoints lw 2 pt 7 lc rgb 'blue' title 'N Value: 10^{6}'
