//Author: Chance Ball
//Date: 4/29/18
//Project 4 : Simple File System - Simple File System

#include "disk.h"
#include "sfs.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int currentBlock = 0; //Holds the current block we're on
int currentFile = 0; //Holds the current file we're on
int fd = 0; //File descriptor for reading/writing
int i = 0; //Iterator
int nextBlock = 0; //Points to the next block
int numBlocks = 0; //How many blocks

dir directory[MAX_FILES]; //Directory entries

int make_sfs(char *disk_name)
{
	strcpy(superBlock, "placeholder\0"); //Placeholder into super block

	if((create_disk(disk_name, MAX_BLOCKS*BLOCK_SIZE)) < 0) //Creates the disk
		{
			printf("Error, failed to create disk (make_sfs).\n");
			return -1;
		}

	if((fd = open_disk(disk_name)) < 0) //Opens the disk
		{
			printf("Error, failed to open disk (make_sfs).\n");
			return -1;
		}

	FATFill(); // fill FAT blocks
	write_block(fd, 0, superBlock);	//Writes the super block onto the disk
	write_block(fd, 1, (char*)FAT);	//Writes the FAT to disk

	close_disk(fd);
	return 0;
} 

int mount_sfs(char *disk_name)
{
	if((fd = open_disk(disk_name)) < 0) //Open disk
		{
			printf("Error, failed to open disk (mount_sfs).\n");
			return -1;
		}

	fileNameArray[fd] = disk_name; //Store the disk name associated with this file descriptor
	read_block(fd, 0, superBlock); //Read super block from disk
	read_block(fd, 1, (char*)FAT); //Read FAT from disk
	return 0;
}

int unmount_sfs(char *disk_name)
{

	for(i = 0; i < MAX_OPEN_FILES; i++)
	{
		if(fileNameArray[i]) //Get file descriptor from a matching name
			fd = i;
	}

	write_block(fd, 0, (char*)superBlock); //Write super block to the disk
	for(i = 1; i < 5; i++)
	{
		write_block(fd, i, (char*)FAT);	//Write FAT to the disk
	}

	fileNameArray[fd] = NULL; //Free file name index associated with file descriptor
	fdPointers[fd] = -1; //Free pointer of file descriptor
	close_disk(fd);	//Close disk
	printf("Disk %s with file descriptor %d successfully unmounted (unmount_sfs).\n", disk_name, fd);
	return 0;
}

int sfs_open(char *file_name)
{
	if(countDir() > MAX_FILES)
		{
			printf("Error, too many files open already (sfs_open).\n");
			return -1;
		}

	if((fd = open(file_name, O_RDWR, (mode_t)0777)) < 0) //Open file with full permissions
	{	
	    printf("Error, could not open file (sfs_open).\n");
		return -1;
	}

	fileNameArray[fd] = file_name;	//Associate file descriptor and file name
	fdPointers[fd] = 0;	//Initialize pointer to start
	currentFile = getIndex(file_name);	//Get file descriptor by using the file name
	directory[currentFile].numOpened++;	//Add number to open instances of file
	printf("File %s with file descriptor %d successfully opened (sfs_open).\n", directory[currentFile].fileName, fd);
	return fd;
}

int sfs_close(int fd)
{
	if(close(fd) < 0)
	{
		printf("Error closing file (sfs_close).\n");
		return -1;
	}
	currentFile = getIndex(fileNameArray[fd]); //Get file directory from name
	directory[currentFile].numOpened--;	//Reduce file instances	
	fdPointers[fd] = -1; //Free file directory pointer
	fileNameArray[fd] = NULL; //Free index holding the name

	printf("File %s with file descriptor %d successfully closed (sfs_close).\n", directory[currentFile].fileName, fd);
	return 0;
}

int sfs_create(char *file_name)
{
	currentFile = getOpenDir(); //Gets next open directory index

	if(currentFile > MAX_FILES)
	{
		printf("Error, too many files open already (sfs_create).\n");
		return -1;
	}

	if((fd = open(file_name, O_RDWR | O_CREAT | O_EXCL, (mode_t)0777)) < 0) //Open file with full permissions
	{	
	    printf("Error could not open file (sfs_create).\n");
		return -1;
	}

	strcpy(directory[currentFile].fileName, file_name);	//Put file name into directory
	directory[currentFile].size = 0; //Set file size in directory
	directory[currentFile].startBlock = 0;	//Set first block of the file in the directory
	close(fd);
	return 0;
}

int sfs_delete(char *file_name)
{	
	
	if(access(file_name, F_OK) < 0) //Check if file exists
	{
    	printf("Deletion failed. File does not exist (sfs_delete).\n"); //File does not exist, so we can't delete
    	return -1;
	}

	currentFile = getIndex(file_name);	//Get file descriptor using file name

	if(directory[currentFile].numOpened > 1) //Check if multiple instances are opened
	{
		printf("Deletion of file %s failed due to %d open instances existing (sfs_delete).\n", file_name, directory[currentFile].numOpened);
		return -1;
	}

	else //No duplicates open
	{																
		unlink(file_name);	//Delete file
	
		currentBlock = directory[currentFile].startBlock; //Set current block to start block of file
		nextBlock = currentBlock;

		while(FAT[currentBlock] > 0)
		{
			nextBlock = FAT[currentBlock];
			FAT[currentBlock] = 0;	//Update File Allocation Table
			currentBlock = nextBlock; //Move to next block
		}
		if(FAT[currentBlock] == -1)	//Hit final FAT block, update
		{
			FAT[currentBlock] = 0;
		}

		strcpy(directory[currentFile].fileName, ""); //Clear out the directory index
		directory[currentFile].size = 0; //Reset variables
		directory[currentFile].startBlock = 0;
		directory[currentFile].numOpened = 0;
		printf("File %s deleted successfully. (sfs_delete).\n", file_name);
	}
	return 0;
}

int sfs_read(int fd, void *buf, size_t count)
{
	numBlocks = ((count / BLOCK_SIZE) - 1); //Find how many blocks need to be read
	currentBlock = directory[currentFile].startBlock; //Update current block in file
	currentFile = getIndex(fileNameArray[fd]);	//Find file in directory by using associated file descriptor

	for(numBlocks; numBlocks >= 0; numBlocks--) //Go through all blocks
	{
		if((read_block(fd, fdPointers[fd], (char*)buf)) < 0)
		{
			printf("Error, could not read block (sfs_read).\n");
			return -1;
		}
		nextBlock = currentBlock; //Move to next block

		while(FAT[currentBlock] > 0) //Move through all blocks of FAT and update
		{
			nextBlock = FAT[currentBlock];
			currentBlock = nextBlock;
		}

		fdPointers[fd] += lseek(fd, BLOCK_SIZE, SEEK_SET);	//Update file pointer and move it up
	}
	return (int)count;
}

int sfs_write(int fd, void *buf, size_t count)
{
	numBlocks = ((count / BLOCK_SIZE) - 1); //Find how many blocks need to be written to

	if((currentBlock = getOpenBlock()) == -4348) //If disk is full arbitrary number -4348 will be in block
	{
		printf("Error writing to disk, disk is full (sfs_write).\n");
		return 0;
	}

	if(directory[currentFile].startBlock != 0)	//File already has some blocks written to, move to unwritten blocks
	{
		currentBlock = getFinalBlock(fd);
	}
	currentFile = getIndex(fileNameArray[fd]); //Find file in directory
	directory[currentFile].startBlock = currentBlock;	
	
	for(numBlocks; numBlocks >= 0; numBlocks--) //Loop through all blocks
	{
		if((write_block(fd, (fdPointers[fd] / BLOCK_SIZE), (char*)buf)) < 0)
		{
			printf("Error, could not write to block (sfs_write).\n");
			return -1;
		}
		FAT[currentBlock] = -1; //Hold place of the block writing to							

		if(numBlocks > 0)
		{
			if((nextBlock = getOpenBlock()) == -4348) //If no free blocks, arbitrary number 4348 will be pointed to at the end
			{
				printf("Error writing to disk, disk is full (sfs_write).\n");
				return -1;
			} 
			else
			{
				FAT[currentBlock] = nextBlock;	//Update FAT blocks
			}
		}
		currentBlock = nextBlock;	//Move to next block and set to current block
		fdPointers[fd] += BLOCK_SIZE;	//Move to next file descriptor
	}

	directory[currentFile].size += count; //Update directory with file's new size
	FAT[currentBlock] = -1;	//Update last block in the FAT
	return (int)count;
}

int sfs_seek(int fd, int offset)
{
	currentFile = getIndex(fileNameArray[fd]);	//Get file using its file descriptor

	if(offset > directory[currentFile].size) //Offset too large
		{
			printf("Error, offset too large for file (sfs_seek).\n");
			return -1;
		}
	fdPointers[fd] = lseek(fd, offset, SEEK_SET); //Find and update file pointer
	return 0;
}

//Start of my functions
int countDir() //Counts how many directory entries exist, by checking if a directory index is filled with a null string
{
	int count = 0;
	for(i = 0; i < MAX_FILES; i++)
	{
		if(strcmp(directory[i].fileName, "") != 0)
		{
			count++;
		}
	}
	return count;
}

void FATFill() //Fills blocks of FAT
{
	for (i=0; i < BLOCK_SIZE; i++)
	{
		switch(i) //Filled with random numbers, initially 0 is -1 so that the first write starts at front
		{
			case 0:
				FAT[i] = -1;
			 	break;
			 case 1:
			 	FAT[i] = 2;
			 	break;
			 case 2:
			 	FAT[i] = 3;
			 	break;
			 case 3:
			 	FAT[i] = 4;
			 	break;
			 case 4:
			 	FAT[i] = -1;
			 	break;
			 case 5:
			 	FAT[i] = -1;
			 	break;
			 default:
			 	FAT[i] = 0;
			 	break;
		}
	}
}

int getFinalBlock(int fd)
{
	currentFile = getIndex(fileNameArray[fd]); //Get file directory from associated file name
	int startBlock = directory[currentFile].startBlock; //Initialize at starting block of file
	if(startBlock != 0)
	{
		for(i = startBlock; i < MAX_BLOCKS; i++) //Iterate through all blocks of file
		{
			if(FAT[i] == -1) //Where the final block of the FAT is, the index will be -1
			{
				return i; //Returns index of final block
			}
		}
	}
}

int getOpenBlock()
{
	for(i = FIRST_BLOCK; i < MAX_BLOCKS; i++) //Goes through all blocks from the first block until the last
	{
		if(FAT[i] == 0) //If an index is freed, it has 0. Return that index
			return i;
	}
	return -4348; //Else return -4348, the disk has no open blocks
}

int getOpenDir()
{
	for(i = 0; i < MAX_FILES; i++) //Go through all files in directory
	{
		if(strcmp(directory[i].fileName, "") == 0) //If space is open, the file name will be empty. Return the index of open directory index.
		{
			return i;
		}
	}
}

int getIndex(char *name)
{
	for(i = 0; i < MAX_FILES; i++) //Go through all files
	{
		if(strcmp(directory[i].fileName, name) == 0) //If name in directory matches the file name given
		{
			return i; //Returns index where the file is held
		}
	}
}

void printDir()
{
	for(i = 0; i < MAX_FILES; i++) //Iterate through all files
	{
		if(strcmp(directory[i].fileName, "") != 0) //If file name exists, ie the directory entry is not empty
		{
			printf("\tDirectory index: %d | name: %s | size: %d | first block: %d\n", i, directory[i].fileName, directory[i].size, directory[i].startBlock); //Print directory contents
		}
	}
}

void printFAT()
{
	for(i = 0; i < MAX_BLOCKS; i++) //Iterate through all blocks
	{
		if(FAT[i] != 0) //If FAT index has contents
		{
			printf("\tFAT index: %d | FAT contents: %d\n", i, FAT[i]); //Print contents
		}
	}
}

void printfileNameArray()
{
	for(i = 0; i < MAX_FILES; i++) //Iterate through all files
	{
		if(fileNameArray[i]) //If there is a filename present in array
		{
			printf("\tFile descriptor's index: %d | file name: %s\n", i, fileNameArray[i]); //Print file's contents
		}
	}
}

