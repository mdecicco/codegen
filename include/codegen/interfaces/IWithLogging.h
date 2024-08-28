#pragma once
#include <codegen/types.h>

namespace codegen {
    class ILogHandler;

    class IWithLogging {
        public:
            IWithLogging();

            void setLogHandler(ILogHandler* handler);
            ILogHandler* getLogHandler() const;

            void logDebug(const char* msg, ...);
            void logInfo(const char* msg, ...);
            void logWarn(const char* msg, ...);
            void logError(const char* msg, ...);
        
        protected:
            ILogHandler* m_logger;
    };
};