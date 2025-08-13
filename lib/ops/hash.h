#pragma once

#include "primitives/primitives.h"
#include "alloc/array.h"

typedef struct Hasher {
    u64 state;
} Hasher;

Hasher new_hasher(void);
u64 finish_hash(Hasher hasher);

#define hash(T) hash_ ## T

void hash_u8(           Hasher* hasher, u8          val);
void hash_u16(          Hasher* hasher, u16         val);
void hash_u32(          Hasher* hasher, u32         val);
void hash_u64(          Hasher* hasher, u64         val);
void hash_usize(        Hasher* hasher, usize       val);
void hash_uptr(         Hasher* hasher, uptr        val);
void hash_umax(         Hasher* hasher, umax        val);

void hash_i8(           Hasher* hasher, i8          val);
void hash_i16(          Hasher* hasher, i16         val);
void hash_i32(          Hasher* hasher, i32         val);
void hash_i64(          Hasher* hasher, i64         val);
void hash_isize(        Hasher* hasher, isize       val);
void hash_iptr(         Hasher* hasher, iptr        val);
void hash_imax(         Hasher* hasher, imax        val);

void hash_bool(         Hasher* hasher, bool        val);

void hash_char(         Hasher* hasher, char        val);
void hash_String(       Hasher* hasher, const char* val);
void hash_StringView(   Hasher* hasher, StringView  val);
