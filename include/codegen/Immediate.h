#pragma once
#include <codegen/types.h>

namespace codegen {
    union Immediate {
        Immediate();
        Immediate(bool rhs);
        Immediate(u8 rhs);
        Immediate(u16 rhs);
        Immediate(u32 rhs);
        Immediate(u64 rhs);
        Immediate(i8 rhs);
        Immediate(i16 rhs);
        Immediate(i32 rhs);
        Immediate(i64 rhs);
        Immediate(f32 rhs);
        Immediate(f64 rhs);
        Immediate(ptr rhs);

        bool operator =(bool rhs);
        u8   operator =(u8 rhs);
        u16  operator =(u16 rhs);
        u32  operator =(u32 rhs);
        u64  operator =(u64 rhs);
        i8   operator =(i8 rhs);
        i16  operator =(i16 rhs);
        i32  operator =(i32 rhs);
        i64  operator =(i64 rhs);
        f32  operator =(f32 rhs);
        f64  operator =(f64 rhs);
        ptr  operator =(ptr rhs);

        operator bool() const;
        operator u8  () const;
        operator u16 () const;
        operator u32 () const;
        operator u64 () const;
        operator i8  () const;
        operator i16 () const;
        operator i32 () const;
        operator i64 () const;
        operator f32 () const;
        operator f64 () const;
        operator ptr () const;
        
        u64  u;
        i64  i;
        f32  f;
        f64  d;
        ptr  p;
    };
};