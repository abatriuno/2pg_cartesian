#!/bin/bash

path="./1VII_MC_temp_309_1/monte_carlo_solutions.pdb"
save_files="monte_carlo_solutions"
mkdir -p $save_files

grep -n 'MODEL\|ENDMDL' $path | cut -d: -f 1 | \
 awk '{if(NR%2) printf "sed -n %d,",$1+1; else printf "%dp '$path' > ./'$save_files'/PROT_IND_%03d.pdb\n", $1-1,NR/2;}' |  bash -sf

python compute_rmsd_pdb_files.py ./$save_files

mkdir -p $1 
mv $save_files all_rmsd.txt $1