#pragma once

#include "primitives/primitives.h"
#include "alloc/array.h"

#define eq(T) eq_##T

bool eq_u8(         u8          a,  u8          b);
bool eq_u16(        u16         a,  u16         b);
bool eq_u32(        u32         a,  u32         b);
bool eq_u64(        u64         a,  u64         b);
bool eq_usize(      usize       a,  usize       b);
bool eq_uptr(       uptr        a,  uptr        b);
bool eq_umax(       umax        a,  umax        b);

bool eq_i8(         i8          a,  i8          b);
bool eq_i16(        i16         a,  i16         b);
bool eq_i32(        i32         a,  i32         b);
bool eq_i64(        i64         a,  i64         b);
bool eq_isize(      isize       a,  isize       b);
bool eq_iptr(       iptr        a,  iptr        b);
bool eq_imax(       imax        a,  imax        b);

bool eq_bool(       bool        a,  bool        b);

bool eq_f32(        f32         a,  f32         b);
bool eq_f64(        f64         a,  f64         b);

bool eq_char(       char        a,  char        b);
bool eq_String(     String      a,  String      b);
bool eq_StringView( StringView  a,  StringView  b);
