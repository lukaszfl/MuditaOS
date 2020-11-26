set terminal svg size 1800,1000 dynamic enhanced font 'arial,16' mousing name "Mem_Usage" butt dashlength 1.0 
set output '@PLOT_OUT@'
set key bmargin left horizontal Right noreverse enhanced autotitle box lt black linewidth 1.000 dashtype solid
set style increment default
set samples 1800, 1200
set title "Mem usage" 
set title  font ",20" norotate
set ylabel "Bytes"
set y2label "Ussage"
set autoscale y
set y2range [0:100]
set autoscale y2
set y2tics
set xtics rotate by -90
show xtics
set grid x y2
plot "@WIKI_DATA_PATH@" using 0:2:xtic(1) title "mem" axes x1y1 with line, "@WIKI_DATA_PATH@" using 4 title "%" axes x1y2 with line
