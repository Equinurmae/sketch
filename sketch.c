//prints out the bytes in a file in hex
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "display.h"

struct state {
  int x, y, xp, yp;
  bool pen_down;
};
typedef struct state state;
typedef enum {DX, DY, DT, PEN};

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

int opcode(unsigned char x) {
    return (x >> 6);
}

int param(unsigned char x) {
    unsigned int y = x & 0x3f;
    signed int c = y;
    int msb = (x >> 5) & 1;
    if (msb == 1) {
      c = -32 + (y-32);
    }
    return c;
}

//does magic
void loop(FILE *f) {
    state s = {0, 0, 0, 0, false};
    int i = 0;
    int scalar = 10;
    display *d = newDisplay("Window", 1280, 960);
    unsigned char ch = fgetc(f);

    while (! feof(f)) {
      int op = opcode(ch);
      if (op == DX) {
        s.x += param(ch);
        printf("Set x to %+d.\n", s.x);
      } else if (op == DY) {
        s.y += param(ch);
        printf("Set y to %+d.\n", s.y);
        if (s.pen_down) line(d, s.xp, s.yp, scalar*s.x, scalar*s.y);
        printf("Draw a line.\n");
        s.xp = scalar*s.x;
        printf("Set xp to %+d\n", s.xp/scalar);
        s.yp = scalar*s.y;
        printf("Set yp to %+d\n", s.yp/scalar);
      } else if (op == DT) {
        pause(d, param(ch));
        printf("Pause for %d ms.\n", param(ch));
      } else if (op == PEN) {
        s.pen_down = ! s.pen_down;
        printf("Changed pen!\n");
      }
      ch = fgetc(f);
      i++;
    }
    end(d);
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
