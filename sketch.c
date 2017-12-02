//the sketch assignment
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "display.h"

//the structure for the state of the display
//xp and yp are the previous coordinates
struct state {
  int x, y, xp, yp;
  bool pen_down;
};

typedef struct state state;
typedef enum baseop {DX, DY, DT, EXT} baseop;
typedef enum extop {DXL, DYL, DTL, PEN, CLEAR, KEY, COL} extop;

//reads in the opcode
int opcode(unsigned char x) {
    return (x >> 6);
}

//reads in the extended length
int oplength(unsigned char x) {
    return ((x >> 4) & 0x3);
}

//reads in the extended opcode
int lopcode(unsigned char x) {
    return (x & 0x0f);
}

//converts an unsigned int to an int
signed int convert(int n, unsigned int y) {
    signed int c = y;
    int msb = (y >> (n-1)) & 1;
    if (msb == 1) {
      c = -pow(2, n-1) + (y-pow(2, n-1));
    }
    return c;
}

//gets the parameter
//sign = true if the operand is supposed to be signed
int param(unsigned char x, bool sign) {
    unsigned int y = x & 0x3f;
    return (sign) ? convert(6, y) : y;
}

//gets the extended operands by reading in the bytes and joining them
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

//the x instruction
void opDX(state *s, signed int x) {
    s->x += x;
}

//the y instruction
void opDY(display *d, state *s, signed int x, int scalar) {
    s->y += x;
    if (s->pen_down) line(d, s->xp, s->yp, scalar*s->x, scalar*s->y);
    s->xp = scalar*s->x;
    s->yp = scalar*s->y;
}

//the pause instruction
void opDT(display *d, signed int x) {
    pause(d, x*10);
}

//the pen instruction
void opPEN(state *s) {
    s->pen_down = ! s->pen_down;
}

//the clear instruction
void opCLEAR(display *d) {
    clear(d);
}

//the key instruction
void opKEY(display *d) {
    key(d);
}

//the colour instruction
void opCOL(display *d, unsigned int x) {
    colour(d, x);
}

//the cases for the extention opcode
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

//loops through each byte in the file, executing the instructions
void loop(FILE *f) {
    state s = {0, 0, 0, 0, false};
    int i = 0;
    //because my screen has a ridiculously high resolution, I added in this parameter
    //to scale the drawings
    int scalar = 1;
    display *d = newDisplay("CANVAS", 1280, 960);
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

//runs the tests
void testBits() {
    assert(opcode(0x00) == 0);
    assert(opcode(0x40) == 1);
    assert(opcode(0x80) == 2);
    assert(opcode(0xc0) == 3);
    assert(oplength(0xc0) == 0);
    assert(oplength(0xd0) == 1);
    assert(oplength(0xe0) == 2);
    assert(oplength(0xf0) == 3);
    assert(lopcode(0xc0) == 0);
    assert(lopcode(0xc1) == 1);
    assert(lopcode(0xc2) == 2);
    assert(lopcode(0xc3) == 3);
    assert(convert(4, 0x1) == 1);
    assert(convert(4, 0x8) == -8);
    assert(convert(4, 0xf) == -1);
    assert(param(0x00, false) == 0);
    assert(param(0x00, true) == 0);
    assert(param(0x0f, false) == 15);
    assert(param(0x3f, false) == 63);
    assert(param(0x3f, true) == -1);
}

int main(int n, char *args[n]) {
    if (n > 1) {
      //reads in the file
      fopenCheck(args[1], "rb");
      FILE *in = fopen(args[1], "rb");
      loop(in);
      fclose(in);
    } else {
      testBits();
      printf("All tests pass.\n");
    }
    return 0;
}
