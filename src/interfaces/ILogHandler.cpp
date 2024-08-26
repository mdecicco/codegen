#include <codegen/interfaces/ILogHandler.h>

namespace codegen {
    void ILogHandler::onInfo(const char* msg) { }
    void ILogHandler::onWarn(const char* msg) { }
    void ILogHandler::onError(const char* msg) { }
};