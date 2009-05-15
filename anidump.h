/*
"riff" {length of file}
"acon"
"list" {length of list}
"inam" {length of title} {data}
"iart" {length of author} {data}
"fram"
"icon" {length of icon} {data} ; 1st in list
...
"icon" {length of icon} {data} ; last in list (1 to cframes)
"anih" {length of ani header (36 bytes)} {data} ; (see ani header typedef )
"rate" {length of rate block} {data} ; ea. rate is a long (length is 1 to csteps)
"seq " {length of sequence block} {data} ; ea. seq is a long (length is 1 to csteps)
-end-
*/

#include <inttypes.h>

struct ani_frame_t {
  uint32_t len;
  unsigned char *buf;
};

struct ani_header_t {
  uint32_t size;
  uint32_t frames;
  uint32_t steps;
  uint32_t xy_zero;
  uint32_t bitcount_zero;
  uint32_t planes_zero;
  uint32_t jifrate;
  uint32_t flags;
};

struct ani_riff_t {
  uint32_t file_length;
  uint32_t list_length;
  char *title;
  char *author;
  struct ani_frame_t frames[256];
  struct ani_header_t header;
};

#define ani_get32(x) (  ((*((x)+0)) & 0xff) + (((*((x)+1)) << 8) & 0xff00) +  (((*((x)+2)) << 16) & 0xff0000) + (((*((x)+3)) << 24) & 0xff000000)  )


/*


- any of the blocks ("acon", "anih", "rate", or "seq ") can appear in any
order. i've never seen "rate" or "seq " appear before "anih", though. you
need the csteps value from "anih" to read "rate" and "seq ". the order i
usually see the frames is: "riff", "acon", "list", "inam", "iart", "anih",
"rate", "seq ", "list", "icon". you can see the "list" tag is repeated and
the "icon" tag is repeated once for every embedded icon. the data pulled
from the "icon" tag is always in the standard 766-byte .ico file format.

- all {length of...} are 4byte dwords.

- ani header typedef:

struct taganiheader {
dword cbsizeof; // num bytes in aniheader (36 bytes)
dword cframes; // number of unique icons in this cursor
dword csteps; // number of blits before the animation cycles
dword cx, cy; // reserved, must be zero.
dword cbitcount, cplanes; // reserved, must be zero.
dword jifrate; // default jiffies (1/60th of a second) if rate chunk not present.
dword flags; // animation flag (see af_ constants)
} aniheader;
#define af_icon =3d 0x0001l // windows format icon/cursor animation


r. james houghtaling

*/


