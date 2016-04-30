path = "./1VII_MC_temp_309_1/"
set term postscript color enhanced eps
set output 'grafico1.eps'
set datafile separator "\t"
set key box
set key left top Left
set autoscale
set ylabel "Potential" offset -2,0
set xlabel "GBSA Solvatation" offset 0,-1.5
plot 1/0 w p lc "black" pt 7 lw 1.5 ps 1.5 t " New solution was dominated",\
1/0 w p lc "gray" pt 7 lw 1.5 ps 1.5 t " New solution dominates it",\
path.'monte_carlo_dominance_objectives.fit' using 3:(stringcolumn(8) eq "NO"? $2 : NaN) pt 7 ps 1.5 lt rgb "black" notitle ,\
path.'monte_carlo_dominance_objectives.fit' using 3:(stringcolumn(8) eq "YES"? $2 : NaN) pt 7 ps 1.3 lt rgb "gray" notitle
set terminal x11
set output
replot
pause -1
