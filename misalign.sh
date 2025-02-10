for i in $(seq 0 10);
do
    VAL=$(bc <<< "scale=3; $i*0.001")
    echo $VAL
    DATA="data_$i"
    mkdir $DATA
    echo $DATA
    ./eicdirc -o $DATA/lut.root -r 1 -g 1 -c 2031 -l 3 -v 0 -ev 0 -x "opticalphoton" -p "3.18 eV"  -e 1000000 -b 1 -rotz 0.001
    cd ../macro
    root -q -b lutmean.C'("../build/$DATA/lut.root")'
    cd ../build
    ./eicdirc -r 0 -o $DATA/sim.root -theta 30 -x "mix_pik" -p 6 -w 0 -g 1 -c 2031 -l 3 -trackingres 0.0005 -e 2000 -b 1 -rotz 0.001
    ./eicdirc -r 2 -i $DATA/sim.root -u $DATA/lut.avr.root -o $DATA/reco.root -trackingres 0.0005 -timeres 0.1 -timecut 0.2 -e 2000 -v 2 -rotz 0.001
    ./eicdirc -r 2 -i $DATA/sim.root -u $DATA/lut.avr.root -o $DATA/reco.root -trackingres 0.0005 -timeres 0.1 -timecut 0.2 -e 2000 -v 3 -rotz 0.001
done