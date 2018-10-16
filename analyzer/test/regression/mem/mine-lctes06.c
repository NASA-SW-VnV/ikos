// SAFE
typedef unsigned char uint8;
typedef unsigned short uint16;

// LCTES'06 example

static union {
  struct {
    uint8 al;
    uint8 ah;
    uint8 bl;
    uint8 bh;
  } b;
  struct {
    uint16 ax;
    uint16 bx;
  } w;
} regs;

extern void __ikos_assert(int);

int main() {
  regs.w.ax = 5;           //(1)
  if (!regs.b.ah)          /*(2)*/
    regs.b.bl = regs.b.al; /*(3)*/
  else                     /*(4)*/
    regs.b.bh = regs.b.al; /*(5)*/

  __ikos_assert(regs.w.ax == 5);
  __ikos_assert(regs.w.bx == 5);
  return 0;
}
