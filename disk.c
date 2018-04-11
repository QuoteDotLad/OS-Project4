//Author: Chance Ball
//Date: 4/11/18
//Project 4 : Simple File System - Virtual Disk

#include "disk.h"

/*----------------------------------------------------------------------------------------------*/
int create_disk(char *filename, size_t nbytes)
{ 
  //int disk;
  char buf[nbytes];

  if(!filename) 
  {
    fprintf(stderr, "Invalid file name (create_disk)\n");
    return -1;
  }

  if((disk = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) 
  {
    perror("Cannot open/create file (create_disk)");
    return -1;
  }

  memset(buf, 0, nbytes);
  for(int i = 0; i < nbytes; i++)
    {
        write(disk, buf, nbytes);
    }
  close(disk);
  return 0;
}
/*----------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
int open_disk(char *filename)
{

  if(!filename) 
  {
    fprintf(stderr, "Invalid file name (open_disk)\n");
    return -1;
  }  
  
  if(isOpen) 
  {
    fprintf(stderr, "Error, disk is already open (open_disk)\n");
    return -1;
  }
  
  if((disk = open(filename, O_RDWR, 0644)) < 0) 
  {
    perror("Error, cannot open file (open_disk)");
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
    fprintf(stderr, "Error, disk not open (read_block)\n");
    return -1;
  }

  if(block_num < 0 || block_num >= MAX_BLOCKS) 
  {
    fprintf(stderr, "Error, index out of bounds (read_block)\n");
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
int write_block(int disk, int block_num, char *buf)
{
  if(!isOpen) 
  {
    fprintf(stderr, "Error, disk not open (write_block)\n");
    return -1;
  }

  if(block_num < 0 || block_num >= MAX_BLOCKS) 
  {
    fprintf(stderr, "Error, index out of bounds (write_block)\n");
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
    fprintf(stderr, "Error, no disk open (close_disk)\n");
    return -1;
  }
  
  close(disk);

  isOpen = false;
  disk = 0;

  return 0;
}
/*----------------------------------------------------------------------------------------------*/

