setupATLAS
lsetup CMake

cmake ..
make -j48

./eicdirc -o lut.root -r 1 -g 1 -c 2031 -l 3 -v 0 -ev 0 -x "opticalphoton" -p "3.18 eV"  -e 1000000 -b 1
cd ../macro
root -q -b "lutmean.C(\"../build/lut.root\")"
cd ../build

for i in $(seq 0 100);
do
    sbatch rotz.sh $i
    sbatch shiftx.sh $i
    sbatch shifty.sh $i
    #sbatch altogether.sh $i
done
