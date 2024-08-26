#pragma once
#include <stdint.h>
#ifndef _MSC_VER
#include <stddef.h>
#endif

namespace codegen {
    typedef uint64_t    u64;
    typedef int64_t     i64;
    typedef uint32_t    u32;
    typedef int32_t     i32;
    typedef uint16_t    u16;
    typedef int16_t     i16;
    typedef uint8_t     u8;
    typedef int8_t      i8;
    typedef float       f32;
    typedef double      f64;
    typedef void*       ptr;
    typedef u32         vreg_id;
    typedef u32         label_id;
    typedef u32         stack_id;

    constexpr vreg_id NullRegister = 0;
    constexpr label_id NullLabel = 0;
    constexpr stack_id NullStack = 0;
};