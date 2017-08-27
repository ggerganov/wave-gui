/*! \file cg_logger.h
 *  \brief Logging options for debugging purposes
 *  \author Georgi Gerganov
*/

#ifndef __CG_LOGGER_H__
#define __CG_LOGGER_H__

#include <memory>
#include <string>

namespace CG {

// Default logging macros
#ifndef CG_LOGGING
    #define CG_LOGGING -1
#endif

#if defined(_WIN32) || defined(WIN32)
    #define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#ifdef __GNUC__
#define CG_FORMAT_ATTRIBUTE(fmt, args) __attribute__((format(printf, fmt, args)))
#else
#define CG_FORMAT_ATTRIBUTE(fmt, args)
#endif

constexpr int kLoggerDefaultDebugVebosity = 10;

#define CG_INFO(V, ...)        if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::INFO, NULL, -1, -1, NULL, __VA_ARGS__)
#define CG_INFOC(V, ...)       if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::INFOC, NULL, -1, -1, __FUNCTION__, __VA_ARGS__)
#define CG_IDBG(V, T, ...)     if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::DEBUG, NULL, -1, (T), __FUNCTION__, __VA_ARGS__)
#define CG_DBG_TAG(V, T, ...)  if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::DEBUG, __FILE__, __LINE__, (T), __FUNCTION__, __VA_ARGS__)
#define CG_DBGD_TAG(V, T, ...) if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::DEBUG, __FILE__, __LINE__, (T), __PRETTY_FUNCTION__, __VA_ARGS__)
#define CG_DBG(V, ...)         if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::DEBUG, __FILE__, __LINE__, -1,  __FUNCTION__, __VA_ARGS__)
#define CG_DBGD(V, ...)        if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::DEBUG, __FILE__, __LINE__, -1,  __PRETTY_FUNCTION__, __VA_ARGS__)
#define CG_DEBUG(...)          if (CG::kLoggerDefaultDebugVebosity <= CG_LOGGING) \
                                                                    CG::Logger::getInstance().logMessage(CG::Logger::DEBUG, __FILE__, __LINE__, -1, __FUNCTION__, __VA_ARGS__)
#define CG_DEBUGD(...)         if (CG::kLoggerDefaultDebugVebosity <= CG_LOGGING) \
                                                                    CG::Logger::getInstance().logMessage(CG::Logger::DEBUG, __FILE__, __LINE__, -1, __PRETTY_FUNCTION__, __VA_ARGS__)
#define CG_FATAL(V, ...)       if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::FATAL, __FILE__, __LINE__, -1,  __FUNCTION__, __VA_ARGS__)
#define CG_FATALD(V, ...)      if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::FATAL, __FILE__, __LINE__, -1,  __PRETTY_FUNCTION__, __VA_ARGS__)
#define CG_WARN(V, ...)        if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::WARNING, __FILE__, __LINE__, -1, __FUNCTION__, __VA_ARGS__)
#define CG_WARND(V, ...)       if (V <= CG_LOGGING) CG::Logger::getInstance().logMessage(CG::Logger::WARNING, __FILE__, __LINE__, -1, __PRETTY_FUNCTION__, __VA_ARGS__)

class Logger {
public:
    enum TypeMessage {
        INFO = 0,
        INFOC,
        DEBUG,
        WARNING,
        FATAL,
    };

    ~Logger();

    static inline Logger & getInstance() {
        static Logger instance;
        return instance;
    }

    void logMessage(TypeMessage type, const char *file, long line, const int   tag, const char *func, const char *msg, ...) CG_FORMAT_ATTRIBUTE(7, 8);
    void logMessage(TypeMessage type, const char *file, long line, const char *tag, const char *func, const char *msg, ...) CG_FORMAT_ATTRIBUTE(7, 8);

    void configure(const std::string &fnameXMLConfig, const std::string &xmlNode);

    void disableInstance();

private:
    class LoggerImplementation;

    Logger();

    std::unique_ptr<LoggerImplementation> impl_;
};
}

#endif
