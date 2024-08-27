#pragma once
#include <codegen/types.h>
#include <codegen/SourceLocation.h>
#include <utils/Array.h>

namespace codegen {
    class SourceMap {
        public:
            struct Entry {
                SourceLocation src;
                u32 firstCodeIndex;
                u32 lastCodeIndex;
            };

            void add(u32 codeIndex, const SourceLocation& src);
            const Entry* get(u32 codeIndex) const;

            Array<Entry> entries;
    };
};