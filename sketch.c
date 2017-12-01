//prints out the bytes in a file in hex
#include <stdio.h>
#include <stdlib.h>
#include "display.h"

//prints the contents of thee file
void print(FILE *f) {
    int i = 0;
    unsigned char ch = fgetc(f);
    while (! feof(f)) {
      if ((i % 16) == 0) printf("%08o ", i);
      if (((i+1) % 16) == 0) printf("%02x\n", ch);
      else printf("%02x ", ch);
      ch = fgetc(f);
      i++;
    }
    if ((i % 16) != 0) printf("\n");
    printf("%08o\n",i);
}

//prints the contents of th efile
void loop(FILE *f) {
    int i = 0;
    unsigned char ch = fgetc(f);
    while (! feof(f)) {
      printf("%02x\n", ch);
      ch = fgetc(f);
      i++;
    }
}

//checks to see if the file can be opened
FILE *fopenCheck(char *file, char *mode) {
  FILE *p = fopen(file, mode);
  if (p != NULL) return p;
  fprintf(stderr, "Can't open %s: ", file);
  fflush(stderr);
  perror("");
  exit(1);
}

int main(int n, char *args[n]) {
    if (n > 1) {
      fopenCheck(args[1], "rb");
      FILE *in = fopen(args[1], "rb");
      loop(in);
      fclose(in);
    }
    return 0;
}
