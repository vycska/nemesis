#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include "at45db161d.h"

#define flash_read AT45DB161D_read
#define flash_write AT45DB161D_write

#define FS_START_ADDRESS (208*FLASH_PAGE_SIZE)
#define FS_END_ADDRESS (FLASH_TOTAL_SIZE-1)
#define FS_TOTAL_SIZE (FS_END_ADDRESS-FS_START_ADDRESS+1)

#define DIRECTORY_SIZE 252
#define FAT_SIZE 256
#define FS_COPY_COUNT 48

#define DIRECTORY_ENTRIES (DIRECTORY_SIZE/sizeof(struct DirectoryEntry))

#define SECTORS 256
#define SECTOR_SIZE (FS_TOTAL_SIZE-FS_COPY_COUNT*FLASH_PAGE_SIZE)/SECTORS //7680 = 15 * 512

#define MAX_FILE_NAME_SIZE 21
#define MAX_HEADER_SIZE 30

#define STATUS_OK 0
#define STATUS_ERROR -1

#define ERROR_FLASH 0
#define ERROR_FILEHANDLE 1
#define ERROR_DATALEN 2
#define ERROR_ADRLEN 3

struct DirectoryEntry { //sizeof = 63 [63 * 4 = 252]
   int data_size;
   short max_sectors, //kiek daugiausiai sektoriu gali sudaryti faila, reiksme 0 reiskia neribota kieki
         start_sector,
         header_size,
         record_size; //reikalingas jei failas sudarytas is fiksuoto dydzio irasu, kurie negali buti perskirti, kas gali atsitikti ties sektoriu riba
   char file_name[MAX_FILE_NAME_SIZE],
        header[MAX_HEADER_SIZE];
} __attribute__((packed));

struct FS { //sizeof = 512 (vienas page'as)
   unsigned int copy;
   unsigned char fat[FAT_SIZE];
   struct DirectoryEntry dir[DIRECTORY_ENTRIES];
} __attribute__((packed));

int fs_getcopy(void);
unsigned int fs_geterrors(void);
void fs_clearerrors(void);
void fs_assert(int value, int error);
int fs_ismounted(void);
int fs_mount(void);
int fs_checkdisk(void);
void fs_format(void);
int fs_flush(void);
int fs_direntryempty(int);
int fs_filenamevalid(char *file_name);
int fs_freesectorfind(void);
int fs_freesectors(void);
int fs_filesearch(char *file_name);
int fs_filesearch_tail(char *file_tail);
int fs_filenew(char *file_name, short max_sectors, short record_size);
void fs_filerename(int file, char *file_name);
void fs_filedelete(int file);
int fs_fileheadersize(int file);
int fs_filerecordsize(int file);
int fs_filedatasize(int file);
int fs_filesize(int file);
char *fs_filename(int file);
char *fs_fileheader(int file);
void fs_fileheaderset(int file, char *header, int len);
void fs_fileappend(int file, char *buf, int len);
int fs_fileread_seq(int file, char *buf, int len);
void fs_fileread_datapart(int file, int adr, int len, unsigned char *buf);

#endif
