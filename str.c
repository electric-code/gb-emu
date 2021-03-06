#include "str.h"

char *str[UCHAR_MAX + 1] = {
    "nop",
    "ld bc,u16",
    "ld (bc),a",
    "inc bc",
    "inc b",
    "dec b",
    "ld b,u8",
    "rlca",
    "ld (u16),sp",
    "add hl,bc",
    "ld a,(bc)",
    "dec bc",
    "inc c",
    "dec c",
    "ld c,u8",
    "rrca",
    "stop",
    "ld de,u16",
    "ld (de),a",
    "inc de",
    "inc d",
    "dec d",
    "ld d,u8",
    "rla",
    "jr i8",
    "add hl,de",
    "ld a,(de)",
    "dec de",
    "inc e",
    "dec e",
    "ld e,u8",
    "rra",
    "jr nz,i8",
    "ld hl,u16",
    "ld (hl+),a",
    "inc hl",
    "inc h",
    "dec h",
    "ld h,u8",
    "daa",
    "jr z,i8",
    "add hl,hl",
    "ld a,(hl+)",
    "dec hl",
    "inc l",
    "dec l",
    "ld l,u8",
    "cpl",
    "jr nc,i8",
    "ld sp,u16",
    "ld (hl-),a",
    "inc sp",
    "inc (hl)",
    "dec (hl)",
    "ld (hl),u8",
    "scf",
    "jr c,i8",
    "add hl,sp",
    "ld a,(hl-)",
    "dec sp",
    "inc a",
    "dec a",
    "ld a,u8",
    "ccf",
    "ld b,b",
    "ld b,c",
    "ld b,d",
    "ld b,e",
    "ld b,h",
    "ld b,l",
    "ld b,(hl)",
    "ld b,a",
    "ld c,b",
    "ld c,c",
    "ld c,d",
    "ld c,e",
    "ld c,h",
    "ld c,l",
    "ld c,(hl)",
    "ld c,a",
    "ld d,b",
    "ld d,c",
    "ld d,d",
    "ld d,e",
    "ld d,h",
    "ld d,l",
    "ld d,(hl)",
    "ld d,a",
    "ld e,b",
    "ld e,c",
    "ld e,d",
    "ld e,e",
    "ld e,h",
    "ld e,l",
    "ld e,(hl)",
    "ld e,a",
    "ld h,b",
    "ld h,c",
    "ld h,d",
    "ld h,e",
    "ld h,h",
    "ld h,l",
    "ld h,(hl)",
    "ld h,a",
    "ld l,b",
    "ld l,c",
    "ld l,d",
    "ld l,e",
    "ld l,h",
    "ld l,l",
    "ld l,(hl)",
    "ld l,a",
    "ld (hl),b",
    "ld (hl),c",
    "ld (hl),d",
    "ld (hl),e",
    "ld (hl),h",
    "ld (hl),l",
    "halt",
    "ld (hl),a",
    "ld a,b",
    "ld a,c",
    "ld a,d",
    "ld a,e",
    "ld a,h",
    "ld a,l",
    "ld a,(hl)",
    "ld a,a",
    "add a,b",
    "add a,c",
    "add a,d",
    "add a,e",
    "add a,h",
    "add a,l",
    "add a,(hl)",
    "add a,a",
    "adc a,b",
    "adc a,c",
    "adc a,d",
    "adc a,e",
    "adc a,h",
    "adc a,l",
    "adc a,(hl)",
    "adc a,a",
    "sub a,b",
    "sub a,c",
    "sub a,d",
    "sub a,e",
    "sub a,h",
    "sub a,l",
    "sub a,(hl)",
    "sub a,a",
    "sbc a,b",
    "sbc a,c",
    "sbc a,d",
    "sbc a,e",
    "sbc a,h",
    "sbc a,l",
    "sbc a,(hl)",
    "sbc a,a",
    "and a,b",
    "and a,c",
    "and a,d",
    "and a,e",
    "and a,h",
    "and a,l",
    "and a,(hl)",
    "and a,a",
    "xor a,b",
    "xor a,c",
    "xor a,d",
    "xor a,e",
    "xor a,h",
    "xor a,l",
    "xor a,(hl)",
    "xor a,a",
    "or a,b",
    "or a,c",
    "or a,d",
    "or a,e",
    "or a,h",
    "or a,l",
    "or a,(hl)",
    "or a,a",
    "cp a,b",
    "cp a,c",
    "cp a,d",
    "cp a,e",
    "cp a,h",
    "cp a,l",
    "cp a,(hl)",
    "cp a,a",
    "ret nz",
    "pop bc",
    "jp nz,u16",
    "jp u16",
    "call nz,u16",
    "push bc",
    "add a,u8",
    "rst 00h",
    "ret z",
    "ret",
    "jp z,u16",
    "prefix cb",
    "call z,u16",
    "call u16",
    "adc a,u8",
    "rst 08h",
    "ret nc",
    "pop de",
    "jp nc,u16",
    "unused",
    "call nc,u16",
    "push de",
    "sub a,u8",
    "rst 10h",
    "ret c",
    "reti",
    "jp c,u16",
    "unused",
    "call c,u16",
    "unused",
    "sbc a,u8",
    "rst 18h",
    "ld (ff00+u8),a",
    "pop hl",
    "ld (ff00+c),a",
    "unused",
    "unused",
    "push hl",
    "and a,u8",
    "rst 20h",
    "add sp,i8",
    "jp hl",
    "ld (u16),a",
    "unused",
    "unused",
    "unused",
    "xor a,u8",
    "rst 28h",
    "ld a,(ff00+u8)",
    "pop af",
    "ld a,(ff00+c)",
    "di",
    "unused",
    "push af",
    "or a,u8",
    "rst 30h",
    "ld hl,sp+i8",
    "ld sp,hl",
    "ld a,(u16)",
    "ei",
    "unused",
    "unused",
    "cp a,u8",
    "rst 38h",
};
