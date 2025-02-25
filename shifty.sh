#!/bin/bash
#SBATCH --job-name=simulation        # Job name
#SBATCH --nodes=1                    # Run on 1 node
#SBATCH --ntasks=1                   # Run on 32 CPUs
#SBATCH --mem=2gb                    # Job memory request
#SBATCH --time=48:00:00              # Time limit hrs:min:sec
#SBATCH --output=simulation_%j.log   # Standard output and error log

##SBATCH --mail-user=muschmidt@uni-wuppertal.de          # Send E-Mails
##SBATCH --mail-type=BEGIN,END,FAIL,REQUEUE,ARRAY_TASKS

. /beegfs/schmidt/software/geant4-v11.2.0-install/share/Geant4/geant4make/geant4make.sh
. /beegfs/schmidt/software/root_install/bin/thisroot.sh

VAL=$(bc <<< "scale=3; $1*0.001")
echo $VAL
DATA="shifty_$1"
mkdir $DATA
echo $DATA
./eicdirc -o $DATA/lut.root -r 1 -g 1 -c 2031 -l 3 -v 0 -ev 0 -x "opticalphoton" -p "3.18 eV"  -e 1000000 -b 1 -shifty $VAL
cd ../macro
root -q -b "lutmean.C(\"../build/$DATA/lut.root\")"
cd ../build
./eicdirc -r 0 -o $DATA/sim.root -theta 30 -x "mix_pik" -p 6 -w 0 -g 1 -c 2031 -l 3 -trackingres 0.0005 -e 20000 -b 1 -rotz $VAL
./eicdirc -r 2 -i $DATA/sim.root -u $DATA/lut.avr.root -o $DATA/reco.root -trackingres 0.0005 -timeres 0.1 -timecut 0.2 -e 20000 -v 2 -shifty $VAL
./eicdirc -r 2 -i $DATA/sim.root -u $DATA/lut.avr.root -o $DATA/reco.root -trackingres 0.0005 -timeres 0.1 -timecut 0.2 -e 20000 -v 2 -shifty $VAL
