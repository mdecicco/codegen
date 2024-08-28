#include <codegen/interfaces/IWithLogging.h>
#include <codegen/interfaces/ILogHandler.h>
#include <stdarg.h>
#include <stdio.h>

namespace codegen {
    IWithLogging::IWithLogging() : m_logger(nullptr) {}

    void IWithLogging::setLogHandler(ILogHandler* handler) {
        m_logger = handler;
    }

    ILogHandler* IWithLogging::getLogHandler() const {
        return m_logger;
    }

    void IWithLogging::logDebug(const char* msg, ...) {
        if (!m_logger) return;

        char buf[1024] = { 0 };

        va_list l;
        va_start(l, msg);
        vsnprintf(buf, 1024, msg, l);
        va_end(l);

        m_logger->onInfo(buf);
    }

    void IWithLogging::logInfo(const char* msg, ...) {
        if (!m_logger) return;

        char buf[1024] = { 0 };

        va_list l;
        va_start(l, msg);
        vsnprintf(buf, 1024, msg, l);
        va_end(l);

        m_logger->onInfo(buf);
    }

    void IWithLogging::logWarn(const char* msg, ...) {
        if (!m_logger) return;

        char buf[1024] = { 0 };

        va_list l;
        va_start(l, msg);
        vsnprintf(buf, 1024, msg, l);
        va_end(l);

        m_logger->onWarn(buf);
    }

    void IWithLogging::logError(const char* msg, ...) {
        if (!m_logger) return;

        char buf[1024] = { 0 };

        va_list l;
        va_start(l, msg);
        vsnprintf(buf, 1024, msg, l);
        va_end(l);

        m_logger->onError(buf);
    }
};