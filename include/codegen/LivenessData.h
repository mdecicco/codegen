#pragma once
#include <codegen/types.h>
#include <utils/Array.h>
#include <unordered_map>

namespace codegen {
    class Value;
    class CodeHolder;
    
    struct RegisterLifetime {
        vreg_id reg_id;
        address begin;
        address end;
        u16 usage_count;
        bool is_fp;

        bool isConcurrent(const RegisterLifetime& o) const;
    };

    class LivenessData {
        public:
            LivenessData();
            LivenessData(CodeHolder* ch);
            void rebuild(CodeHolder* ch);

            Array<RegisterLifetime*> rangesOf(const Value& v);
            Array<RegisterLifetime*> rangesOf(u32 reg_id);
            bool isLive(const Value& v, address at);
            bool isLive(u32 reg_id, address at);
            RegisterLifetime* getLiveRange(const Value& v, address at);
            RegisterLifetime* getLiveRange(u32 reg_id, address at);

            Array<RegisterLifetime> lifetimes;
            std::unordered_map<u32, Array<u32>> regLifetimeMap;
    };
};