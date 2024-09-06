#include "Common.h"
#include <bind/Registry.h>

void setupTest() {
    bind::Registry::Reset();
    bind::type<void>("void");
    bind::type<bool>("bool");
    bind::type<i8>("i8");
    bind::type<i16>("i16");
    bind::type<i32>("i32");
    bind::type<i64>("i64");
    bind::type<u8>("u8");
    bind::type<u16>("u16");
    bind::type<u32>("u32");
    bind::type<u64>("u64");
    bind::type<f32>("f32");
    bind::type<f64>("f64");
}