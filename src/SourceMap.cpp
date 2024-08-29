#include <codegen/SourceMap.h>
#include <utils/Array.hpp>

namespace codegen {
    void SourceMap::add(u32 codeIndex, const SourceLocation& src) {
        if (entries.size() == 0) {
            entries.push({ src, codeIndex, codeIndex });
            return;
        }

        Entry& last = entries.last();
        if (
            last.src.resourceId == src.resourceId &&
            last.src.startBufferPosition == src.startBufferPosition &&
            last.src.endBufferPosition == src.endBufferPosition
        ) {
            last.lastCodeIndex = codeIndex;
            return;
        }

        entries.push({ src, codeIndex, codeIndex });
    }

    const SourceMap::Entry* SourceMap::get(u32 codeIndex) const {
        for (u32 i = 0;i < entries.size();i++) {
            if (entries[i].firstCodeIndex <= codeIndex && entries[i].lastCodeIndex >= codeIndex) {
                return &entries[i];
            }
        }

        return nullptr;
    }
};