#include <codegen/Immediate.h>

namespace codegen {
    Immediate::Immediate() : u(0) {}
    Immediate::Immediate(bool rhs) { u = rhs; }
    Immediate::Immediate(u8   rhs) { u = rhs; }
    Immediate::Immediate(u16  rhs) { u = rhs; }
    Immediate::Immediate(u32  rhs) { u = rhs; }
    Immediate::Immediate(u64  rhs) { u = rhs; }
    Immediate::Immediate(i8   rhs) { u = rhs; }
    Immediate::Immediate(i16  rhs) { i = rhs; }
    Immediate::Immediate(i32  rhs) { i = rhs; }
    Immediate::Immediate(i64  rhs) { i = rhs; }
    Immediate::Immediate(f32  rhs) { f = rhs; }
    Immediate::Immediate(f64  rhs) { d = rhs; }
    Immediate::Immediate(ptr  rhs) { p = rhs; }

    bool Immediate::operator =(bool  rhs) { return bool(u = rhs); }
    u8   Immediate::operator =(u8  rhs) { return u8 (u = rhs); }
    u16  Immediate::operator =(u16 rhs) { return u16(u = rhs); }
    u32  Immediate::operator =(u32 rhs) { return u32(u = rhs); }
    u64  Immediate::operator =(u64 rhs) { return u64(u = rhs); }
    i8   Immediate::operator =(i8  rhs) { return i8 (i = rhs); }
    i16  Immediate::operator =(i16 rhs) { return i16(i = rhs); }
    i32  Immediate::operator =(i32 rhs) { return i32(i = rhs); }
    i64  Immediate::operator =(i64 rhs) { return i64(i = rhs); }
    f32  Immediate::operator =(f32 rhs) { return f = rhs;}
    f64  Immediate::operator =(f64 rhs) { return d = rhs; }
    ptr  Immediate::operator =(ptr rhs) { return p = rhs; }

    Immediate::operator bool() const { return bool(u); }
    Immediate::operator u8  () const { return u8  (u); }
    Immediate::operator u16 () const { return u16 (u); }
    Immediate::operator u32 () const { return u32 (u); }
    Immediate::operator u64 () const { return u64 (u); }
    Immediate::operator i8  () const { return i8  (i); }
    Immediate::operator i16 () const { return i16 (i); }
    Immediate::operator i32 () const { return i32 (i); }
    Immediate::operator i64 () const { return i64 (i); }
    Immediate::operator f32 () const { return f; }
    Immediate::operator f64 () const { return d; }
    Immediate::operator ptr () const { return p; }
};