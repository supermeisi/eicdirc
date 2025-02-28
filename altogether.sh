#!/bin/bash
#SBATCH --job-name=simulation
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --mem=2gb
#SBATCH --time=48:00:00
#SBATCH --output=simulation_%j.log

. /beegfs/schmidt/software/geant4-v11.2.0-install/share/Geant4/geant4make/geant4make.sh
. /beegfs/schmidt/software/root_install/bin/thisroot.sh

VAL=$(bc <<< "scale=3; $1*0.0001")
SHIFTX=$(bc <<< "scale=3; $1*0.001")
SHIFTY=$(bc <<< "scale=3; $1*0.001")
DATA="data_$1"

mkdir $DATA
./eicdirc -o $DATA/lut.root -r 1 -g 1 -c 2031 -l 3 -v 0 -ev 0 -x "opticalphoton" -p "3.18 eV" -e 1000000 -b 1 -rotz $VAL -shiftx $SHIFTX -shifty $SHIFTY

cd ../macro
root -q -b "lutmean.C(\"../build/$DATA/lut.root\")"
cd ../build

./eicdirc -r 0 -o $DATA/sim.root -theta 30 -x "mix_pik" -p 6 -w 0 -g 1 -c 2031 -l 3 -trackingres 0.0005 -e 20000 -b 1 -rotz $VAL -shiftx $SHIFTX -shifty $SHIFTY

./eicdirc -r 2 -i $DATA/sim.root -u $DATA/lut.avr.root -o $DATA/reco.root -trackingres 0.0005 -timeres 0.1 -timecut 0.2 -e 20000 -v 2 -rotz $VAL -shiftx $SHIFTX -shifty $SHIFTY

./eicdirc -r 2 -i $DATA/sim.root -u $DATA/lut.avr.root -o $DATA/reco.root -trackingres 0.0005 -timeres 0.1 -timecut 0.2 -e 20000 -v 2 -rotz $VAL -shiftx $SHIFTX -shifty $SHIFTY
