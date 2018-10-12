#include "file_system.h"
#include "utils.h"
#include <ctype.h>
#include <string.h>

static unsigned int fs_mounted = 0, fs_errors = 0;
static struct FS fs;

int fs_getcopy(void) {
   return fs.copy;
}

unsigned int fs_geterrors(void) {
   return fs_errors;
}

void fs_clearerrors(void) {
   fs_errors = 0;
}

void fs_assert(int value, int error) {
   if(!value)
      fs_errors |= (1 << error);
}

int fs_ismounted(void) {
   return fs_mounted == 1;
}

int fs_mount(void) {
   unsigned char t[4];
   int i, p;
   unsigned int v, max_v;

   //randama paskutine failu sistemos kopija
   for(i = 0; i < FS_COPY_COUNT; i++) {
      flash_read(FS_START_ADDRESS + i * FLASH_PAGE_SIZE, t, 4);
      v = *((unsigned int *)t);

      if(i == 0 || v > max_v) {
         p = i;
         max_v = v;
      }
   }

   flash_read(FS_START_ADDRESS + p * FLASH_PAGE_SIZE, (unsigned char *)&fs, FLASH_PAGE_SIZE);
   fs_mounted = 1;
   return p;
}

int fs_checkdisk(void) {
   int i;
   for(i = 0; i < DIRECTORY_ENTRIES
              &&
              fs_filenamevalid(fs.dir[i].file_name)
              &&
              fs.dir[i].max_sectors >= 0
              &&
              fs.dir[i].max_sectors < SECTORS
              &&
              fs.dir[i].start_sector >= 0
              &&
              fs.dir[i].start_sector < SECTORS
              &&
              fs.dir[i].header_size < MAX_HEADER_SIZE
              &&
              fs.dir[i].record_size < SECTOR_SIZE; i++);
   return i == DIRECTORY_ENTRIES ? STATUS_OK : STATUS_ERROR;
}

void fs_format(void) {
   int i;
   memset(&fs, 0, FLASH_PAGE_SIZE);
   for(i = 0; i < FS_COPY_COUNT; i++)
      flash_write(FS_START_ADDRESS + i * FLASH_PAGE_SIZE, (unsigned char *)&fs, FLASH_PAGE_SIZE);
   fs_clearerrors();
}

int fs_flush(void) {
   fs.copy += 1;
   flash_write(FS_START_ADDRESS + (fs.copy % FS_COPY_COUNT) * FLASH_PAGE_SIZE, (unsigned char *)&fs, FLASH_PAGE_SIZE);
   return fs.copy % FS_COPY_COUNT;
}

int fs_direntryempty(int file) {
   int i;
   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   for(i = 0; i < MAX_FILE_NAME_SIZE && fs.dir[file].file_name[i] == 0; i++); //direktorijos irasas [failas] laikomas tusciu, jei nera failo vardo
   return i == MAX_FILE_NAME_SIZE;
}

int fs_filenamevalid(char *file_name) {
   int i;
   for(i = 0; i < MAX_FILE_NAME_SIZE - 1
              &&
              (  isalpha(file_name[i])
                 ||
                 isdigit(file_name[i])
                 ||
                 file_name[i] == '.'
                 ||
                 file_name[i] == '_'
                 ||
                 file_name[i] == '-'
                 ||
                 file_name[i] == '('
                 ||
                 file_name[i] == ')'
               ); i++);

   return file_name[i] == 0;
}

int fs_freesectorfind(void) {
   unsigned char sector_mask[SECTORS / 8] = { 0 };
   int i, size, sector;
   for(i = 0; i < DIRECTORY_ENTRIES; i++)
      if(!fs_direntryempty(i)) //pazymimi visi failo uzimami sektoriai; failas visada uzima bent viena sektoriu [net jei jo dydis yra 0, vienas sektorius jam yra isskiriamas kuriant faila]
         for(sector = fs.dir[i].start_sector, size = fs.dir[i].data_size; size == fs.dir[i].data_size || size > 0; sector_mask[sector / 8] |= (1 << (sector % 8)), sector = fs.fat[sector], size -= (SECTOR_SIZE / fs.dir[i].record_size) * fs.dir[i].record_size);
   for(i = 0; i < SECTORS && (sector_mask[i / 8] & (1 << (i % 8))) != 0; i++); //ieskau nepanaudoto sektoriaus
   return i < SECTORS ? i : STATUS_ERROR;
}

int fs_freesectors(void) {
   unsigned char sector_mask[SECTORS / 8] = { 0 };
   int i, k, sector, size;
   for(i = 0; i < DIRECTORY_ENTRIES; i++)
      if(!fs_direntryempty(i))
         for(sector = fs.dir[i].start_sector, size = fs.dir[i].data_size; size == fs.dir[i].data_size || size > 0; sector_mask[sector / 8] |= (1 << (sector % 8)), sector = fs.fat[sector], size -= (SECTOR_SIZE / fs.dir[i].record_size) * fs.dir[i].record_size);
   for(k = i = 0; i < SECTORS; i++)
      if((sector_mask[i / 8] & (1 << (i % 8))) == 0)
         k += 1;
   return k;
}

int fs_filesearch(char *file_name) {
   int i;
   for(i = 0; i < DIRECTORY_ENTRIES && strcmp(file_name, fs.dir[i].file_name) != 0; i++);
   return i == DIRECTORY_ENTRIES ? STATUS_ERROR : i;
}

int fs_filesearch_tail(char *file_tail) {
   int i;
   for(i = 0; i < DIRECTORY_ENTRIES && strcmp(file_tail, fs.dir[i].file_name + strlen(fs.dir[i].file_name) - strlen(file_tail)) != 0; i++);
   return i == DIRECTORY_ENTRIES ? STATUS_ERROR : i;
}

int fs_filenew(char *file_name, short max_sectors, short record_size) {
   int i, s, f, l = strlen(file_name);
   for(i = 0; i < DIRECTORY_ENTRIES && !fs_direntryempty(i); i++); //ieskau ar yra laisvas irasas direktorijoje
   if(   i < DIRECTORY_ENTRIES
         &&
         fs_filenamevalid(file_name)
         &&
         l > 0
         &&
         l <= MAX_FILE_NAME_SIZE - 1
         &&
         max_sectors < SECTORS
         &&
         record_size > 0
         &&
         record_size < SECTOR_SIZE
         &&
         (s = fs_freesectorfind()) != STATUS_ERROR)
   {
      strcpy(fs.dir[i].file_name, file_name);
      fs.dir[i].max_sectors = max_sectors;
      fs.dir[i].start_sector = s;
      fs.dir[i].header_size = 0;
      fs.dir[i].record_size = record_size;
      fs.dir[i].data_size = 0;
      f = i;
   }
   else
      f = STATUS_ERROR;
   return f;
}

void fs_filerename(int file, char *file_name) {
   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   strcpy(fs.dir[file].file_name, file_name);
}

void fs_filedelete(int file) {
   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   memset(&fs.dir[file], 0, sizeof(struct DirectoryEntry));
}

int fs_fileheadersize(int file) {
   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   return fs.dir[file].header_size;
}

int fs_filerecordsize(int file) {
   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   return fs.dir[file].record_size;
}

int fs_filedatasize(int file) {
   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   return fs.dir[file].data_size;
}

int fs_filesize(int file) {
   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   return fs.dir[file].header_size + fs.dir[file].data_size;
}

char *fs_filename(int file) {
   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   return fs.dir[file].file_name;
}

char *fs_fileheader(int file) {
   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   return fs.dir[file].header;
}

void fs_fileheaderset(int file, char *header, int len) {
   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   len = MIN2(len, MAX_HEADER_SIZE);
   fs.dir[file].header_size = len;
   memcpy(fs.dir[file].header, header, len);
}

void fs_fileappend(int file, char *buf, int len) {
   int k_sector, last_sector, new_sector, size, l;

   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);
   fs_assert(len % fs.dir[file].record_size == 0, ERROR_DATALEN);

   //surandamas paskutinis failo uzimamas sektorius ir jame uzpildytu baitu kiekis
   for(k_sector = 1, last_sector = fs.dir[file].start_sector, size = fs.dir[file].data_size; size - (SECTOR_SIZE / fs.dir[file].record_size) * fs.dir[file].record_size > 0;
      k_sector += 1, last_sector = fs.fat[last_sector], size -= (SECTOR_SIZE / fs.dir[file].record_size) * fs.dir[file].record_size);

   do {
      //paskaiciuojam kiek pilnu irasu baitais telpa i dabartini sektoriu
      l = ((SECTOR_SIZE - size) / fs.dir[file].record_size) * fs.dir[file].record_size;

      if(l > 0) { //kazkiek irasu/baitu telpa i dabartini sektoriu
         l = MIN2(l, len);
         flash_write(FS_START_ADDRESS + FS_COPY_COUNT * FLASH_PAGE_SIZE + last_sector * SECTOR_SIZE + size, (unsigned char *)buf, l);
         fs.dir[file].data_size += l;
         size += l;
         buf += l;
         len -= l;
      }
      else { //reikalingas naujas sektorius
         if((fs.dir[file].max_sectors > 0 && k_sector + 1 > fs.dir[file].max_sectors) || (new_sector = fs_freesectorfind()) == STATUS_ERROR) { //failas pasieke jam skirtu sektoriu kieki arba nera tuscio sektoriaus -- rasysime i pirma failo sektoriu
            new_sector = fs.dir[file].start_sector;
            fs.dir[file].start_sector = fs.fat[fs.dir[file].start_sector];
            fs.dir[file].data_size -= (SECTOR_SIZE / fs.dir[file].record_size) * fs.dir[file].record_size;
         }
         size = 0;
         fs.fat[last_sector] = new_sector;
         last_sector = new_sector;
      }
   } while(len > 0);
}

int fs_fileread_seq(int file, char *buf, int len) {
   static int current_file = -1, state, size_returned;
   int sector, size, l = 0, r = 0;

   if(current_file != file) {
      current_file = file;
      size_returned = 0;
      state = 1;
   }

   if(current_file == STATUS_ERROR)
      state = 0;

   if(state == 1) { //nuskaitomas header'is
      l = MIN2(len, fs.dir[file].header_size - size_returned); //pasirinkimas tarp kiek liko issiusti ir buferio dydzio

      if(l > 0) {
         memcpy(buf, fs.dir[file].header + size_returned, l);
         len -= l;
         buf += l;
         size_returned += l;
         r += l;
      }

      if(size_returned == fs.dir[file].header_size) {
         size_returned = 0;
         state = 2;
      }
   }

   if(state == 2) { //nuskaitomi failo duomenys
      //is pradziu reikia rasti sektoriu, kur prasideda dar negrazinti duomenys
      for(sector = fs.dir[file].start_sector, size = size_returned; size - (SECTOR_SIZE / fs.dir[file].record_size) * fs.dir[file].record_size > 0; sector = fs.fat[sector], size -= (SECTOR_SIZE / fs.dir[file].record_size) * fs.dir[file].record_size);

      while(len > 0 && size_returned != fs.dir[file].data_size) {
         //cia sector yra dabartinis sektorius, size - kiek jame baitu jau grazinta
         l = MIN3(len, fs.dir[file].data_size - size_returned, (SECTOR_SIZE / fs.dir[file].record_size) * fs.dir[file].record_size - size);

         if(l > 0) {
            flash_read(FS_START_ADDRESS + FS_COPY_COUNT * FLASH_PAGE_SIZE + sector * SECTOR_SIZE + size, (unsigned char *)buf, l);
            len -= l;
            buf += l;
            size_returned += l;
            r += l;
         }

         size = 0;
         sector = fs.fat[sector];
      }

      if(size_returned == fs.dir[file].data_size)
         current_file = -1;
   }

   return r;
}

void fs_fileread_datapart(int file, int adr, int len, unsigned char *buf) {
   int l, sector;

   fs_assert(file >= 0 && file < DIRECTORY_ENTRIES, ERROR_FILEHANDLE);

   if(len < 0) { //jei len<0, adr skaiciuojamas nuo failo pabaigos
      len = -len;
      adr = fs.dir[file].data_size - adr - len;
   }

   fs_assert(adr + len <= fs.dir[file].data_size, ERROR_ADRLEN);

   for(sector = fs.dir[file].start_sector; adr - (SECTOR_SIZE / fs.dir[file].record_size) * fs.dir[file].record_size > 0; sector = fs.fat[sector], adr -= (SECTOR_SIZE / fs.dir[file].record_size) * fs.dir[file].record_size);

   while(len > 0) {
      l = MIN2(len, (SECTOR_SIZE / fs.dir[file].record_size) * fs.dir[file].record_size - adr);

      if(l > 0) {
         flash_read(FS_START_ADDRESS + FS_COPY_COUNT * FLASH_PAGE_SIZE + sector * SECTOR_SIZE + adr, buf, l);
         len -= l;
         buf += l;
      }

      adr = 0;
      sector = fs.fat[sector];
   }
}
