output: disk.o sfs.o
	gcc disk.o sfs.o -o output

disk.o: disk.c
	gcc -c disk.c

sfs.o: sfs.c
	gcc -c sfs.c


