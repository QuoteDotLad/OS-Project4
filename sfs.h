//Author: Chance Ball
//CS4348.501 Project 4
//Header file for sfs.c
//Date: 4/29/18

#ifndef SFS_H_
#define SFS_H_

//Provided defines
#define BLOCK_SIZE 4096
#define MAX_BLOCKS 4096
#define MAX_DATA_BLOCKS 3500
#define MAX_FILES 64
#define MAX_OPEN_FILES 64
#define FNAME_LENGTH 16

//My defines
#define MAX_DISKNUM 3
#define FIRST_BLOCK 596

//Structure for a directory, dir
typedef struct 
{
	char fileName[FNAME_LENGTH];	//File name
	size_t size;				//File size
	size_t startBlock;			//Points to next block
	size_t numOpened;		//How many instances are open
} 
dir;

//My global variables
char superBlock[BLOCK_SIZE];
int FAT[BLOCK_SIZE];
char *fileNameArray[MAX_OPEN_FILES];
int fdPointers[MAX_OPEN_FILES];

//Provided function prototypes
int make_sfs(char *disk_name);
int mount_sfs(char *disk_name);
int unmount_sfs(char *disk_name);
int sfs_open(char *file_name);
int sfs_close(int fd);
int sfs_create(char *file_name);
int sfs_delete(char *file_name);
int sfs_read(int fd, void *buf, size_t count);
int sfs_write(int fd, void *buf, size_t count);
int sfs_seek(int fd, int offset);

//My function prototypes
int countDir(); //Counts directories
void FATFill(); //Fills File Allocation Table with numbers at the beginnning
int getFinalBlock(int fd); //Gets final block of a file, marked with -1
int getOpenBlock(); //Finds the first free block of a file, otherwise returns -4348
int getOpenDir(); //Finds an open index in the directory by looking for an empty string in the file name
int getIndex(char *name); //Retrieves the index a file is at by using its name and file descriptor to match up
void printFAT(); //Prints the contents of the File Allocation Table
void printfileNameArray(); //Prints the file descriptor's index and name of file 
void printDir(); //Prints the directory's contents, which hold all file names, sizes, and start blocks

#endif