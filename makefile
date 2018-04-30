output: disk.o sfs.o
	gcc disk.o sfs.o -o output

disk.o: disk.c
	gcc -std=c99 -c disk.c

sfs.o: sfs.c
	gcc -std=c99 -c sfs.c
