#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdint.h>
#include "anidump.h"

void *do_mmap(char *filename)
{
  int i;
  int fd;

  unsigned char *retval;

  struct stat status;
  off_t filesize;

  if (-1 == (i = stat(filename, &status))) {
    perror("stat");
    return NULL;
  }
  
  if (-1 == (fd = open(filename, O_RDONLY))) {
    perror("open");
    return NULL;
  }
  
  retval = mmap(NULL, filesize, PROT_READ, MAP_SHARED, fd, 0);

  if (NULL == retval) {
    perror("mmap");
    return NULL;
  }
 
  return retval;
}

int print_single_frame(char *template, unsigned char *buf, int len, int i)
{
  int fd;
  int max_fn;
  char *filename;
  if(template == NULL)
    template = "frame";

  max_fn = strlen(template)+10;
  filename = malloc(max_fn+1);

  snprintf(filename, max_fn, "%s%04d", template, i);
  fd = open(filename, O_CREAT|O_RDWR, S_IRWXU);
  
  if(fd == -1) {
    perror("open");
    exit(-1);
  }
  
  write(fd, buf, len);
  close(fd);

  return 0;
}

char *ani_get_string(unsigned char *buf, int len)
{
  char *retval = malloc(len+1);
  memcpy(retval, buf, len);
  retval[len] = 0x00;
  return retval;
}

int ani_check_string(unsigned char *buf, char *s)
{
  char *b = ani_get_string(buf, strlen(s));
  int k = strncasecmp(b, s, strlen(s));
  free(b);
  return k;
}

struct ani_header_t *read_header(struct ani_header_t *retval, unsigned char *buf)
{
  int pos;

  if(!retval)
    if(NULL == (retval = malloc(sizeof(struct ani_header_t))))
      return NULL;

  pos = 0;

  retval->size = ani_get32(buf+pos);
  pos += 4;
  retval->frames = ani_get32(buf+pos);
  pos += 4;
  retval->steps = ani_get32(buf+pos);
  pos += 4;
  retval->xy_zero = ani_get32(buf+pos);
  pos += 4;
  retval->bitcount_zero = ani_get32(buf+pos);
  pos += 4;
  retval->planes_zero = ani_get32(buf+pos);
  pos += 4;
  retval->jifrate = ani_get32(buf+pos);
  pos += 4;
  retval->flags = ani_get32(buf+pos);
  pos += 4;

  return retval;
}

struct ani_riff_t *icon_list_parser(struct ani_riff_t *retval, unsigned char *buf, uint32_t len) 
{
  int pos = 0;
  int cur_frame_no = 0;

  if(ani_check_string(buf+pos, "fram")) {
    printf("not a framelist, sorry\n");
    exit(-1);
  }
  pos += 4;

  for(; len > pos;) {
    uint32_t framelen = 0;

    if(ani_check_string(buf+pos, "icon")) {
      printf("expected an icon entry, but didn't get one at %d\n", pos);
      exit(-1);
    }
    pos += 4;

    framelen = ani_get32(buf+pos);
    pos += 4;

    retval->frames[cur_frame_no].len = framelen;
    retval->frames[cur_frame_no].buf = buf+pos;

    print_single_frame("test", buf+pos, framelen, cur_frame_no);

    cur_frame_no++;
    pos += framelen;

  }

  return retval;
}
  

struct ani_riff_t *read_riff(struct ani_riff_t *retval, unsigned char *buf)
{
  int pos;

  if(NULL == retval)
    if(NULL == (retval = malloc(sizeof(struct ani_riff_t))))
      return NULL;
 
  pos = 0;
  if(ani_check_string(buf+pos, "RIFF")) {
    printf("bad RIFF header at pos %d\n", pos);
    exit(-1);
  }
  pos += 4;

  retval->file_length = ani_get32(buf+pos);
  pos += 4;

  if(ani_check_string(buf+pos, "acon")) {
    printf("bad RIFF header at pos %d\n", pos);
    exit(-1);
  }
  pos += 4;

  if(ani_check_string(buf+pos, "list")) {
    printf("bad RIFF header at pos %d\n", pos);
    exit(-1);
  }
  pos += 4;

  retval->list_length = ani_get32(buf+pos);
  pos += 4;

  /* XXX parse list */

  pos += retval->list_length;

  if(ani_check_string(buf+pos, "anih")) {
    printf("bad RIFF header at pos %d\n", pos);
    exit(-1);
  }
  pos += 4;

  read_header(&(retval->header), buf+pos);
  pos += 4 + retval->header.size;

  for( ; retval->file_length > pos; ) {
    char *curheader = ani_get_string(buf+pos, 4);
    uint32_t curlen;
    pos += 4;
    curlen = ani_get32(buf+pos);
    pos += 4;

    if(0 == strncasecmp(curheader, "LIST", 4))
      if(0 == ani_check_string(buf+pos, "fram"))
	icon_list_parser(retval, buf+pos, curlen);
    
    pos += curlen;
  }
  return retval;
}

int print_frames(struct ani_riff_t* riff, char *template)
{
  int i;
  for(i = 0; i < riff->header.frames; i++) {
    printf("frame %d(%d/%p).", i, riff->frames[i].len, riff->frames[i].buf);
    print_single_frame(template, riff->frames[i].buf, riff->frames[i].len, i);
    printf(".\n");
    
  }
  return 0;
}

int main(int argc, char *argv[]) 
{
  unsigned char *data = (unsigned char *)do_mmap(argv[1]);
  struct ani_riff_t riff;

  read_riff(&riff, data);

  print_frames(&riff, NULL);
  return 0;
}
