//Author: Chance Ball
//Date: 4/11/18
//Project 4 : Simple File System - Virtual Disk

#include "disk.h"

/*----------------------------------------------------------------------------------------------*/
int create_disk(char *filename, size_t nbytes)
{ 
  int fd;
  char *buf[BLOCK_SIZE];
     

  if(!filename) 
  {
    printf("Invalid file name (create_disk)\n");
    return -1;
  }

  if((fd = open(filename, O_RDWR | O_CREAT | O_EXCL, (mode_t)0777)) < 0) 
  {
    perror("Cannot open/create file, already exists(create_disk)");
    return -1;
  }

  lseek(fd, (nbytes-1), SEEK_SET);

  if((write(fd, "\0", 1)) < 0) //Expand disk by writing \0 to it
  {					
		printf("Error writing null terminal to disk (create_disk).\n");
		close(fd);
    return -1;
  }

  lseek(fd, 0, SEEK_SET); //Reposition pointer in file
  close(fd);
  return 0;
}
/*----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
int open_disk(char *filename)
{
  int fd;
  if(!filename)
  {
    printf("Invalid file name(open_disk)\n");
    return -1;
  }  
  
  if(isOpen) 
  {
    printf("Error, disk is already open (open_disk)\n");
    return -1;
  }
  
  if((fd = open(filename, O_RDWR)) < 0)
  {
    printf("Failed to open disk (open_disk)\n");
    return -1;
  }  

  isOpen = true;

  return disk;
}
/*----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
int read_block(int disk, int block_num, char *buf)
{
  if(!isOpen) 
  {
    printf("Error, disk not open (read_block)\n");
    return -1;
  }

  if(block_num < 0 || block_num >= MAX_BLOCKS) 
  {
    printf("Error, index out of bounds (read_block)\n");
    return -1;
  }

  if(lseek(disk, (block_num * BLOCK_SIZE), SEEK_SET) < 0)
  {
    perror("Failed to reposition in file (read_block)\n");
    return -1;
  }

  if(read(disk, buf, BLOCK_SIZE) < 0) 
  {
    perror("Error, unable to read file (read_block)\n");
    return -1;
  }

  return 0;
}
/*----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
int write_block(int disk, int block_num, char* buf)
{
  if(!isOpen) 
  {
    printf("Error, disk not open (write_block)\n");
    return -1;
  }

  if(block_num < 0 || block_num >= MAX_BLOCKS) 
  {
    printf("Error, index out of bounds (write_block)\n");
    return -1;
  }

  if(lseek(disk, (block_num * BLOCK_SIZE), SEEK_SET) < 0) 
  {
    perror("Failed to reposition in file (write_block)\n");
    return -1;
  }

  if(write(disk, buf, BLOCK_SIZE) < 0) 
  {
    perror("Error, unable to write to file (write_block)\n");
    return -1;
  }

  return 0;
}
/*----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
int close_disk(int disk)
{
  if (!isOpen) 
  {
    printf("Error, no disk open (close_disk)\n");
    return -1;
  }
  
  close(disk);

  isOpen = false;
  disk = 0;

  return 0;
}
/*----------------------------------------------------------------------------------------------*/
