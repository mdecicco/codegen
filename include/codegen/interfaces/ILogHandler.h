#pragma once
#include <codegen/types.h>

namespace codegen {
    class ILogHandler {
        public:
            virtual void onInfo(const char* msg);
            virtual void onWarn(const char* msg);
            virtual void onError(const char* msg);
    };
};