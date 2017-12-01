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
typedef enum {DX, DY, DT, EXT};
typedef enum {PEN = 3, CLEAR, KEY, COL};

int opcode(unsigned char x) {
    return (x >> 6);
}

int oplength(unsigned char x) {
    return ((x >> 4) & 0x3);
}

int lopcode(unsigned char x) {
    return (x & 0x0f);
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

void opDX(state *s, unsigned char x) {
    s->x += param(x);
}

void opDY(display *d, state *s, unsigned char x, int scalar) {
    s->y += param(x);
    if (s->pen_down) line(d, s->xp, s->yp, scalar*s->x, scalar*s->y);
    s->xp = scalar*s->x;
    s->yp = scalar*s->y;
}

void opDT(display *d, unsigned char x) {
    pause(d, param(x));
}

void opPEN(state *s) {
    s->pen_down = ! s->pen_down;
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
      switch(op) {
        case DX:
          opDX(&s, ch);
          break;
        case DY:
          opDY(d, &s, ch, scalar);
          break;
        case DT:
          opDT(d, ch);
          break;
        case EXT:
          opPEN(&s);
          int length = oplength(ch);
          int lop = lopcode(ch);
          printf("Length is %d and lopcode is %d\n", length, lop);
          break;
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
