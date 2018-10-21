for i in {1..10}; do
    for j in 8 32 128 512; do
	for k in 1000 4000 16000 64000; do
	    ./run.sh ./out/unbounded_regs $j $k
	done;
    done;
done
