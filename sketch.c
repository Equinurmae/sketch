//prints out the bytes in a file in hex
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "display.h"

struct state {
  int x, y, xp, yp;
  bool pen_down;
};

typedef struct state state;
typedef enum baseop {DX, DY, DT, EXT} baseop;
typedef enum extop {DXL, DYL, DTL, PEN, CLEAR, KEY, COL} extop;

int opcode(unsigned char x) {
    return (x >> 6);
}

int oplength(unsigned char x) {
    return ((x >> 4) & 0x3);
}

int lopcode(unsigned char x) {
    return (x & 0x0f);
}

signed int convert(int n, unsigned int y) {
    signed int c = y;
    int msb = (y >> (n-1)) & 1;
    if (msb == 1) {
      c = -pow(2, n-1) + (y-pow(2, n-1));
    }
    return c;
}

//sign = true if the operand is supposed to be signed
int param(unsigned char x, bool sign) {
    unsigned int y = x & 0x3f;
    return (sign) ? convert(6, y) : y;
}

//sign = true if the operand is supposed to be signed
int lparam (FILE *f, unsigned char x, int n, bool sign) {
    if (n == 0) return 0;
    else {
      unsigned int y = fgetc(f);
      if (n == 3) n = 4;
      for (int i = 0; i < n-1; i++) {
        unsigned int m = fgetc(f);
        y = (y << 8) | m;
      }
    return (sign) ? convert(n*8, y) : y;
    }
}

void opDX(state *s, signed int x) {
    s->x += x;
}

void opDY(display *d, state *s, signed int x, int scalar) {
    s->y += x;
    if (s->pen_down) line(d, s->xp, s->yp, scalar*s->x, scalar*s->y);
    s->xp = scalar*s->x;
    s->yp = scalar*s->y;
}

void opDT(display *d, signed int x) {
    pause(d, x*10);
}

void opPEN(state *s) {
    s->pen_down = ! s->pen_down;
}

void opCLEAR(display *d) {
    clear(d);
}

void opKEY(display *d) {
    key(d);
}

void opCOL(display *d, unsigned int x) {
    colour(d, x);
}

void extra(FILE *f, unsigned char ch, display *d, state *s, extop lop, int length, int scalar) {
    switch (lop) {
      case DXL:
        opDX(s, lparam(f, ch, length, true));
        break;
      case DYL:
        opDY(d, s, lparam(f, ch, length, true), scalar);
        break;
      case DTL:
        opDT(d, lparam(f, ch, length, false));
        break;
      case PEN:
        opPEN(s);
        break;
      case CLEAR:
        opCLEAR(d);
        break;
      case KEY:
        opKEY(d);
        break;
      case COL:
        opCOL(d, lparam(f, ch, length, false));
        break;
    }
}

//does magic
void loop(FILE *f) {
    state s = {0, 0, 0, 0, false};
    int i = 0;
    int scalar = 1;
    display *d = newDisplay("lawn.sketch", 1280, 960);
    unsigned char ch = fgetc(f);
    while (! feof(f)) {
      baseop op = opcode(ch);
      int length = oplength(ch);
      extop lop = lopcode(ch);
      switch(op) {
        case DX:
          opDX(&s, param(ch, true));
          break;
        case DY:
          opDY(d, &s, param(ch, true), scalar);
          break;
        case DT:
          opDT(d, param(ch, false));
          break;
        case EXT:
          extra(f, ch, d, &s, lop, length, scalar);
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
