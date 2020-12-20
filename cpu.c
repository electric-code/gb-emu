#include "cpu.h"

static uint8_t op;

#define ZF 0x80
#define NF 0x40
#define HF 0x20
#define CF 0x10

static void dump(void) {
  FILE *fp = fopen("ram.bin", "wb");
  for (uint16_t pos = 0; pos != USHRT_MAX; ++pos) {
    fputc(gb_rb(pos), fp);
  }
  fclose(fp);
}

static void die(void) {
  printf("%.2x\n", op);
  dump();
  exit(EXIT_FAILURE);
}

/* general registers */
static uint16_t af;
static uint16_t bc;
static uint16_t de;
static uint16_t hl;
static uint8_t *a, *b;
static uint8_t *c, *d;
static uint8_t *e, *f;
static uint8_t *h, *l;

/* special registers */
static uint16_t pc = 0x100; /* program counter */
static uint16_t sp; /* stack pointer */

/* condition functions */
static bool cc_nz(void) { return !(*f & ZF); }
static bool cc_z(void) { return *f & ZF; }
static bool cc_nc(void) { return !(*f & CF); }
static bool cc_c(void) { return *f & CF; }

static void zero(uint8_t arg) {
  if (arg == 0x00) {
    *f |= ZF;
  } else {
    *f &= ~ZF;
  }
}

static void carry(uint8_t arg1, uint8_t arg2) {
  if ((uint16_t)(arg1 & 0xff) + (uint16_t)(arg2 & 0xff) > 0xff) {
    *f |= CF;
  } else {
    *f &= ~CF;
  }
}

static void half_carry(uint8_t arg1, uint8_t arg2) {
  if ((arg1 & 0x0f) + (arg2 & 0x0f) > 0x0f) {
    *f |= HF;
  } else {
    *f &= ~HF;
  }
}

static void borrow(uint8_t arg1, uint8_t arg2) {
  if (arg1 < arg2) {
    *f |= CF;
  } else {
    *f &= ~CF;
  }
}

static void half_borrow(uint8_t arg1, uint8_t arg2) {
  if ((arg1 & 0x0f) < (arg2 & 0x0f)) {
    *f |= HF;
  } else {
    *f &= ~HF;
  }
}

/* alu functions */

static void a_add(uint8_t arg) {
  carry(*a, arg);
  half_carry(*a, arg);
  *a += arg;
  zero(*a);
  *f &= ~NF;
}

static void a_adc(uint8_t arg) {
  uint16_t c = (uint16_t)(*f & CF ? 1 : 0);
  if ((uint16_t)*a + (uint16_t)arg + c > 0xff) {
    *f |= CF;
  } else {
    *f &= ~CF;
  }
  if ((uint16_t)(*a & 0x0f) + (uint16_t)(arg & 0x0f) + c > 0x0f) {
    *f |= HF;
  } else {
    *f &= ~HF;
  }
  *a = ((uint16_t)*a + (uint16_t)arg + c) & 0xff;
  zero(*a);
  *f &= ~NF;
}

static void a_sub(uint8_t arg) {
  borrow(*a, arg);
  half_borrow(*a, arg);
  *a -= arg;
  zero(*a);
  *f |= NF;
}

static void a_sbc(uint8_t arg) {
  uint16_t c = (uint16_t)(*f & CF ? 1 : 0);
  if ((uint16_t)*a < (uint16_t)arg + c) {
    *f |= CF;
  } else {
    *f &= ~CF;
  }
  if ((uint16_t)(*a & 0x0f) < (uint16_t)(arg & 0x0f) + c) {
    *f |= HF;
  } else {
    *f &= ~HF;
  }
  *a = ((uint16_t)*a - (uint16_t)arg - c) & 0xff;
  zero(*a);
  *f |= NF;
}

static void a_and(uint8_t arg) {
  *a &= arg;
  zero(*a);
  *f &= ~NF;
  *f |= HF;
  *f &= ~CF;
}

static void a_xor(uint8_t arg) {
  *a ^= arg;
  zero(*a);
  *f &= ~NF;
  *f &= ~HF;
  *f &= ~CF;
}

static void a_or(uint8_t arg) {
  *a |= arg;
  zero(*a);
  *f &= ~NF;
  *f &= ~HF;
  *f &= ~CF;
}

static void a_cp(uint8_t arg) {
  borrow(*a, arg);
  half_borrow(*a, arg);
  zero(*a - arg);
  *f |= NF;
}

static void r_rlc(uint8_t *arg) {
  uint8_t t = *arg << 1;
  if (*arg & 0x80) {
    t |= 1;
    *f |= CF;
  } else {
    t &= ~1;
    *f &= ~CF;
  }
  *f &= ~HF;
  *f &= ~NF;
  zero(t);
  *arg = t;
}

static void r_rrc(uint8_t *arg) {
  uint8_t t = *arg >> 1;
  if (*arg & 0x01) {
    t |= 0x80;
    *f |= CF;
  } else {
    *f &= ~CF;
  }
  *f &= ~HF;
  *f &= ~NF;
  zero(t);
  *arg = t;
}

static void r_rl(uint8_t *arg) {
  uint8_t t = (*arg << 1);
  if (*f & CF) {
    t |= 1;
  }
  if (*arg & 0x80) {
    *f |= CF;
  } else {
    *f &= ~CF;
  }
  *f &= ~NF;
  *f &= ~HF;
  zero(t);
  *arg = t;
}

static void r_rr(uint8_t *arg) {
  uint8_t t = *arg >> 1;
  if (*f & CF)
    t |= 0x80;
  else
    t &= ~0x80;
  *f &= ~NF;
  *f &= ~HF;
  if (*arg & 1)
    *f |= CF;
  else
    *f &= ~CF;
  *arg = t;
  zero(*arg);
}

static void r_sla(uint8_t *arg) {
  uint8_t t = *arg << 1;
  if (*arg & 0x80) {
    *f |= CF;
  } else {
    *f &= ~CF;
  }
  *f &= ~NF;
  *f &= ~HF;
  zero(t);
  *arg = t;
}

static void r_sra(uint8_t *arg) {
  uint8_t c = *arg & 0x01;
  uint8_t t = (*arg >> 1) | (*arg & 0x80);
  if (c) {
    *f |= CF;
  } else {
    *f &= ~CF;
  }
  *f &= ~HF;
  *f &= ~NF;
  zero(t);
  *arg = t;
}

static void r_swap(uint8_t *arg) {
  uint8_t t = *arg;
  *arg ^= *arg;
  *arg |= (t >> 4) & 0xf;
  *arg |= (t & 0xf) << 4;
  zero(*arg);
  *f &= ~NF;
  *f &= ~HF;
  *f &= ~CF;
}

static void r_srl(uint8_t *arg) {
  if (*arg & 1) {
    *f |= CF;
  } else {
    *f &= ~CF;
  }
  *arg >>= 1;
  zero(*arg);
  *f &= ~NF;
  *f &= ~HF;
}

/* register tables */
#define HL_IND_IDX 6
static uint8_t *r[8];
static uint16_t *rp[4] = { &bc, &de, &hl, &sp };
static uint16_t *rp2[4] = { &bc, &de, &hl, &af };
static bool (*cc[4])(void) = { &cc_nz, &cc_z, &cc_nc, &cc_c };
static void (*alu[8])(uint8_t) = { &a_add, &a_adc, &a_sub, &a_sbc, &a_and, &a_xor, &a_or, &a_cp };
static void (*rot[8])(uint8_t *) = { &r_rlc, &r_rrc, &r_rl, &r_rr, &r_sla, &r_sra, &r_swap, &r_srl };

static void __attribute__((__constructor__)) gb_cpu_init(void) {
  a = &((uint8_t *) &af)[1];
  f = &((uint8_t *) &af)[0];
  b = &((uint8_t *) &bc)[1];
  c = &((uint8_t *) &bc)[0];
  d = &((uint8_t *) &de)[1];
  e = &((uint8_t *) &de)[0];
  h = &((uint8_t *) &hl)[1];
  l = &((uint8_t *) &hl)[0];
  r[0] = b;
  r[1] = c;
  r[2] = d;
  r[3] = e;
  r[4] = h;
  r[5] = l;
  r[7] = a;
}

/* synthetic registers */
#define a (*a)
#define f (*f)
#define b (*b)
#define c (*c)
#define d (*d)
#define e (*e)
#define h (*h)
#define l (*l)

static uint8_t x, y, z, p, q;
static bool inc = true;

static void push8(uint8_t arg) { gb_wb(sp--, arg); }
static uint8_t pop8(void) { return gb_rb(++sp); }

static uint16_t pop16(void) {
  uint16_t nn = gb_rb(sp++);
  nn |= gb_rb(sp++) << 8;
  return nn;
}

static void push16(uint16_t arg) {
  gb_wb(--sp, (arg >> 8) & 0xff);
  gb_wb(--sp, arg & 0xff);
}

#define NN \
  uint16_t nn = gb_rb(pc + 1) | (gb_rb(pc + 2) << 8);

static void x0_z0(void) {
  switch (y) {
    case 0:
      /* nop */
      break;
    case 1:
      {
        /* ld (nn), sp */
        NN;
        gb_wb(nn, sp & 0xff);
        gb_wb(nn + 1, (sp >> 8) & 0xff);
        break;
      }
    case 2:
      /* TODO: stop */
      break;
    case 3:
    {
      /* jr d */
      int8_t disp = (int8_t) gb_rb(pc + 1);
      pc += 2;
      pc += disp;
      inc = false;
      break;
    }
    default:
    {
      /* jr cc[y-4],d */
      int8_t disp = (int8_t) gb_rb(pc + 1);
      pc += 2;
      if ((*cc[y - 4])()) {
        pc += disp;
      }
      inc = false;
      break;
    }
  }
}

static void carry_16(uint16_t arg1, uint16_t arg2) {
  if ((uint32_t)(hl) + (uint32_t)(arg2) > 0xffff)
    f |= CF;
  else
    f &= ~CF;
}

static void half_carry_16(uint16_t arg1, uint16_t arg2) {
  if ((arg1 & 0xfff) + (arg2 & 0xfff) > 0xfff)
    f |= HF;
  else
    f &= ~HF;
}

static void x0_z1(void) {
  switch (q) {
    case 0:
      {
        NN;
        *rp[p] = nn;
        break;
      }
    case 1:
      carry_16(hl, *rp[p]);
      half_carry_16(hl, *rp[p]);
      hl += *rp[p];
      f &= ~NF;
      break;
  }
}

static void x0_z2(void) {
  switch (q) {
    case 0:
      switch (p) {
        case 0: gb_wb(bc, a); break;
        case 1: gb_wb(de, a); break;
        case 2: gb_wb(hl, a); hl += 1; break;
        case 3: gb_wb(hl, a); hl -= 1; break;
      }
      break;
    case 1:
      switch (p) {
        case 0: a = gb_rb(bc); break;
        case 1: a = gb_rb(de); break;
        case 2: a = gb_rb(hl); hl += 1; break;
        case 3: a = gb_rb(hl); hl -= 1; break;
      }
      break;
  }
}

static void daa(void) {
  if (!(f & NF)) {
    if (f & CF || a > 0x99) {
      a += 0x60;
      f |= CF;
    }
    if (f & HF || (a & 0x0f) > 0x09) {
      a += 0x06;
    }
  } else {
    if (f & CF) {
      a -= 0x60;
    }
    if (f & HF) {
      a -= 0x06;
    }
  }
  zero(a);
  f &= ~HF;
}

static void x0_z7(void) {
  switch (y) {
    case 0:
      r_rlc(&a);
      f &= ~ZF;
      break;
    case 1:
      r_rrc(&a);
      f &= ~ZF;
      break;
    case 2:
      r_rl(&a);
      f &= ~ZF;
      break;
    case 3:
      r_rr(&a);
      f &= ~ZF;
      break;
    case 4:
      daa();
      break;
    case 5:
      /* cpl */
      a = ~a;
      f |= NF;
      f |= HF;
      break;
    case 6:
      /* scf */
      f &= ~NF;
      f &= ~HF;
      f |= CF;
      break;
    case 7:
      /* ccf */
      if (f & CF)
        f &= ~CF;
      else
        f |= CF;
      f &= ~NF;
      f &= ~HF;
      break;
  }
}

static void x0(void) {
  switch (z) {
    case 0:
      x0_z0();
      break;
    case 1:
      x0_z1();
      break;
    case 2:
      x0_z2();
      break;
    case 3:
      switch (q) {
        case 0: *rp[p] += 1; break;
        case 1: *rp[p] -= 1; break;
      }
      break;
    case 4:
      half_carry(*r[y], 1);
      *r[y] += 1;
      zero(*r[y]);
      f &= ~NF;
      break;
    case 5:
      half_borrow(*r[y], 1);
      *r[y] -= 1;
      zero(*r[y]);
      f |= NF;
      break;
    case 6:
      *r[y] = gb_rb(pc + 1);
      break;
    case 7:
      x0_z7();
      break;
  }
}

static void x3_z0(void) {
  switch (y) {
    case 4:
      gb_wb(0xff00 + gb_rb(pc + 1), a);
      break;
    case 5:
      carry(sp & 0xff, gb_rb(pc + 1));
      half_carry(sp & 0xff, gb_rb(pc + 1));
      sp += (int8_t) gb_rb(pc + 1);
      f &= ~ZF;
      f &= ~NF;
      break;
    case 6:
      a = gb_rb(0xff00 + gb_rb(pc + 1));
      break;
    case 7:
      carry(sp & 0xff, gb_rb(pc + 1));
      half_carry(sp & 0xff, gb_rb(pc + 1));
      hl = sp + (int8_t) gb_rb(pc + 1);
      f &= ~ZF;
      f &= ~NF;
      break;
    default:
      if ((*cc[y])()) {
        pc = pop16();
        inc = false;
      }
      break;
  }
}

static void x3(void) {
  switch (z) {
    case 0:
      x3_z0();
      break;
    case 1:
      switch (q) {
        case 0: *rp2[p] = pop16(); break;
        case 1:
          switch (p) {
            case 0: pc = pop16(); inc = false; break;
            case 1: pc = pop16(); inc = false; break;
            case 2: pc = hl; inc = false; break;
            case 3: sp = hl; break;
          }
          break;
      }
      break;
    case 2:
      switch (y) {
        case 4:
          gb_wb(0xff00 + c, a);
          break;
        case 5:
        {
          NN;
          gb_wb(nn, a);
          break;
        }
        case 6:
          a = gb_rb(0xff00 + c);
          break;
        case 7:
        {
          NN;
          a = gb_rb(nn);
          break;
        }
        default:
        {
          /* jp cc[y],nn */
          NN;
          pc += 3;
          if ((*cc[y])()) {
            pc = nn;
          }
          inc = false;
          break;
        }
      }
      break;
    case 3:
      switch (y) {
        case 0:
        {
          NN;
          pc += 3;
          pc = nn;
          inc = false;
          break;
        }
        case 6: /* TODO: di */ break;
        case 7: /* TODO: ei */ break;
        default:
          die();
          break;
      }
      break;
    case 4:
      switch (y) {
        case 0:
        case 1:
        case 2:
        case 3:
        {
          /* call cc[y],nn */
          if ((*cc[y])()) {
            NN;
            push16(pc + 3);
            pc = nn;
            inc = false;
          }
          break;
        }
        default:
          die();
          break;
      }
      break;
    case 5:
      switch (q) {
        case 0:
          push16(*rp2[p]);
          break;
        case 1:
          switch (p) {
            case 0:
            {
              NN;
              push16(pc + 3);
              pc = nn;
              inc = false;
              break;
            }
            default:
              die();
              break;
          }
          break;
      }
      break;
    case 6:
      (*alu[y])(gb_rb(pc + 1));
      break;
    case 7:
      push16(pc + 1);
      pc = y << 3;
      inc = false;
      break;
  }
}

uint8_t len[UCHAR_MAX + 1] = {
/*
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
*/
  1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1,
  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
  2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
  1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 1, 2, 1,
  2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1,
  2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1,
};

#include "str.h"

#define DECODE \
  x = (op >> 6) & 0b11; \
  y = (op >> 3) & 0b111; \
  z = op & 0b111; \
  p = (op >> 4) & 0b11; \
  q = (op >> 3) & 0b1;

void gb_cpu_tick(void) {
  f &= 0xf0;
  r[HL_IND_IDX] = gb_rb_ptr(hl);
  op = gb_rb(pc);
  if (op != 0xcb) {
    DECODE;
    inc = true;
    switch (x) {
      case 0:
        x0();
        break;
      case 1:
        if (z == 6 && y == 6) {
          /* halt */
        } else {
          *r[y] = *r[z];
        }
        break;
      case 2:
        (*alu[y])(*r[z]);
        break;
      case 3:
        x3();
        break;
    }
    if (inc) pc += len[op];
    if (!len[op]) {
      printf("%.2x: invalid opcode!", op);
      die();
    }
  } else {
    pc += 1;
    op = gb_rb(pc);
    DECODE;
    switch (x) {
      case 0:
        (*rot[y])(r[z]);
        break;
      case 1:
        if (*r[z] & (1 << y))
          f &= ~ZF;
        else
          f |= ZF;
        f &= ~NF;
        f |= HF;
        break;
      case 2:
        *r[z] &= ~(1 << y);
        break;
      case 3:
        *r[z] |= 1 << y;
        break;
    }
    pc += 1;
  }
}
