build: main

test: main
	seq 10000 | xargs -P128 -n1 ./main | sort | uniq -c