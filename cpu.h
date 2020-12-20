#ifndef CPU_H
#define CPU_H

#include "gb.h"

extern void gb_wb(uint16_t, uint8_t); /* write byte */
extern uint8_t *gb_rb_ptr(uint16_t); /* read byte, fetch pointer */
extern uint8_t gb_rb(uint16_t); /* read byte */

extern void gb_cpu_tick(void);

#endif /* end of include guard: CPU_H */
