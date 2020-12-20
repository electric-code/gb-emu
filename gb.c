#include "cpu.h"

uint8_t mem[USHRT_MAX];

uint8_t gb_rb(uint16_t addr) { return mem[addr]; }
uint8_t *gb_rb_ptr(uint16_t addr) { return &mem[addr]; }
void gb_wb(uint16_t addr, uint8_t val) {
  mem[addr] = val;
  if (addr == 0xff02) fprintf(stderr, "%c", mem[0xff01]);
}

int main(int argc, char const *argv[]) {
  FILE *fp = fopen(argv[1], "rb");
  fread(mem, sizeof(uint8_t), sizeof(mem), fp);
  fclose(fp);
  for (;;) {
    gb_cpu_tick();
  }
  return 0;
}
