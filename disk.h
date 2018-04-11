#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BLOCKS  4096  //Max number of blocks on the disk                
#define BLOCK_SIZE   4096 //Block size on disk                        

//Global Variables
int disk; //File Descriptor for virtual disk
bool isOpen = false; //Boolean to see if disk is open

//Function Prototypes
int create_disk(char *filename, size_t nbytes); //Create an empty, virtual disk file
int open_disk(char *filename); //Open a virtual disk
int read_block(int disk ,int block_num, char *buf); //Read a block of size BLOCK_SIZE from disk
int write_block(int disk, int block, char *buf); //Write a block of size BLOCK_SIZE to disk
int close_disk(int disk); //Close an opened disk    
