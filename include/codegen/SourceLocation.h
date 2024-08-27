#pragma once
#include <codegen/types.h>

namespace codegen {
    struct SourceLocation {
        /**
         * @brief User-defined value used to uniquely identify the file or
         * resource that this location refers to.
         */
        u32 resourceId;
        
        /**
         * @brief Starting position in the file or resource referred to by
         * resourceId
         */
        u32 startBufferPosition;

        /**
         * @brief Ending position in the file or resource referred to by
         * resourceId
         */
        u32 endBufferPosition;

        /**
         * @brief Starting line number in the file or resource referred to
         * by resourceId
         */
        u32 startLine;
        
        /**
         * @brief Ending line number in the file or resource referred to
         * by resourceId
         */
        u32 endLine;

        /**
         * @brief Starting column number in the file or resource referred to
         * by resourceId
         */
        u32 startColumn;

        /**
         * @brief Ending column number in the file or resource referred to
         * by resourceId
         */
        u32 endColumn;
    };
};