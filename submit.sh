for i in $(seq 0 100);
do
    #sbatch rotz.sh $i
    #sbatch shiftx.sh $i
    #sbatch shifty.sh $i
    sbatch altogether.sh $i
done
