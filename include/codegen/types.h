#pragma once
#include <utils/types.h>
#include <bind/types.h>

namespace codegen {
    using namespace utils;
    using namespace bind;
    
    typedef void*       ptr;
    typedef u32         vreg_id;
    typedef u32         label_id;
    typedef u32         stack_id;

    constexpr vreg_id NullRegister = 0;
    constexpr label_id NullLabel = 0;
    constexpr stack_id NullStack = 0;
};