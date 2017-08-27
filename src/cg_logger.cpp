/*! \file cg_logger.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "cg_logger.h"

#ifdef __MINGW32__
    #include <cstdio>
    #include "mingw.thread.h"
#endif

#include <mutex>
#ifdef __MINGW32__
    #include "mingw.mutex.h"
    #include "mingw.condition_variable.h"
#endif

#include <map>
#include <thread>
#include <queue>
#include <condition_variable>
#include <cinttypes>

#ifdef CG_COLOR_OUTPUT
    #define COL_DEFAULT "\033[0m"
    #define COL_BOLD "\033[1m"
    #define COL_RED "\033[01;31m"
    #define COL_YELLOW "\033[01;33m"
    #define COL_GREEN "\033[01;32m"
    #define COL_BLUE "\033[01;34m"
    #define COL_MAGENTA "\033[01;35m"
    #define COL_CYAN "\033[01;36m"
#else
    #define COL_DEFAULT ""
    #define COL_BOLD ""
    #define COL_RED ""
    #define COL_YELLOW ""
    #define COL_GREEN ""
    #define COL_BLUE ""
    #define COL_MAGENTA ""
    #define COL_CYAN ""
#endif

#include <cstring>
#include <cstdarg>
#include <cstdlib>

constexpr size_t kMaxLogPrefixSize = 64*1024;
constexpr size_t kMaxLogMessageSize = 64*1024;

namespace CG {

class Logger::LoggerImplementation {
public:
    LoggerImplementation() {
        running_ = true;
        useColors_ = true;
        showTags_ = true;
        showTimestamps_ = false;
        showSignatures_ = false;

        maxLogPrefixSize_  = kMaxLogPrefixSize;
        maxLogMessageSize_ = kMaxLogMessageSize;

        worker_ = std::thread(&Logger::LoggerImplementation::run, this);
    }

    ~LoggerImplementation() {
        disableInstance();
    }

    void disableInstance() {
        running_ = false;
        if (worker_.joinable()) {
            std::vector<char> a(128, 0);
#if (defined(_WIN32) || defined(WIN32)) && defined(_MSC_VER)
            sprintf_s(a.data(), 128, "[End of Log]\n");
#else
            sprintf(a.data(), "[End of Log]\n");
#endif
            push(INFO, -1, a, a);
            worker_.join();
        }
    }

    void pause() {
        {
            std::lock_guard<std::mutex> lock(mutexResume_);
            if (!running_) return;
            running_ = false;
        }

        if (worker_.joinable()) {
            std::vector<char> a(128, 0);
#if (defined(_WIN32) || defined(WIN32)) && defined(_MSC_VER)
            sprintf_s(a.data(), 128, "[Pausing Logger]\n");
#else
            sprintf(a.data(), "[Pausing Logger]\n");
#endif
            push(INFO, -1, a, a);
            worker_.join();
        }
    }

    void resume() {
        {
            std::lock_guard<std::mutex> lock(mutexResume_);
            if (running_) return;
            running_ = true;
        }
        worker_ = std::thread(&Logger::LoggerImplementation::run, this);
    }

    void print() const {
        CG_INFO(0, "\n");
        CG_INFO(0, "[LOGGER] Logger configuration:\n");
        CG_INFO(0, "\n");
        CG_INFO(0, "         Compile-time options:\n");
        CG_INFO(0, "               Verbosity         (CG_LOGGING):         %d\n", CG_LOGGING);
#ifdef CG_COLOR_ARGUMENTS
        CG_INFO(0, "               Argument coloring (CG_COLOR_ARGUMENTS): %s\n", "yes");
#else
        CG_INFO(0, "               Argument coloring (CG_COLOR_ARGUMENTS): %s\n", "no");
#endif
        CG_INFO(0, "\n");
        CG_INFO(0, "         [Colors]          Use colors                  - %s\n", useColors_ ? "on" : "off");
        CG_INFO(0, "         [ShowTags]        Show tags                   - %s\n", showTags_ ? "on" : "off");
        CG_INFO(0, "         [Timestamps]      Show timestamps             - %s\n", showTimestamps_ ? "on" : "off");
        CG_INFO(0, "         [Signatures]      Show function signatures    - %s\n", showSignatures_ ? "on" : "off");
        CG_INFO(0, "         [MaxPrefixSize]   Max signature length        - %d\n", maxLogPrefixSize_);
        CG_INFO(0, "         [MaxMessageSize]  Max log message length      - %d\n", maxLogMessageSize_);
        CG_INFO(0, "\n");
        CG_INFO(0, "         Enabled STRING tags (count = %ld):\n", enabledTagsStr_.size());
        for (auto & tag : enabledTagsStr_) {
            if (!tag.second) continue;
            CG_INFO(0, "               %s\n", tag.first.c_str());
        }
        CG_INFO(0, "\n");
        CG_INFO(0, "         Enabled INT tags (count = %ld):\n", enabledTagsInt_.size());
        for (auto & tag : enabledTagsInt_) {
            if (!tag.second) continue;
            CG_INFO(0, "               %d\n", tag.first);
        }
        CG_INFO(0, "\n");
    }

    void push(TypeMessage type, int tag, const std::vector<char> & logPrefix, const std::vector<char> & logMessage) {
        Message aMsg;
        aMsg.type_ = type;
        aMsg.printPrefix_ = (type != INFOC);

        if (showTimestamps_) {
            aMsg.timestamp_ =
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        aMsg.hasTag_ = false;
        if (tag > 0) {
            aMsg.hasTag_ = true;
            aMsg.hasTagInt_ = true;
            aMsg.hasTagStr_ = false;
            aMsg.tagInt_ = tag;
        }

        if (showSignatures_ && aMsg.printPrefix_) {
            aMsg.prefix_ = logPrefix;
        } else {
            aMsg.prefix_.resize(1, 0);
        }
        aMsg.msg_ = logMessage;

        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.size() >= 1024) return;
        queue_.push(std::move(aMsg));
        lock.unlock();
        cv_.notify_one();
    }

    void push(TypeMessage type, const char *tag, const std::vector<char> & logPrefix, const std::vector<char> & logMessage) {
        Message aMsg;
        aMsg.type_ = type;
        aMsg.printPrefix_ = (type != INFOC);

        if (showTimestamps_) {
            aMsg.timestamp_ =
                std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        }


        aMsg.hasTag_ = false;
        if (tag) {
            aMsg.hasTag_ = true;
            aMsg.hasTagStr_ = true;
            aMsg.hasTagInt_ = false;
            aMsg.tagStr_ = std::string(tag);
        }

        if (showSignatures_ && aMsg.printPrefix_) {
            aMsg.prefix_ = logPrefix;
        } else {
            aMsg.prefix_.resize(1, 0);
        }
        aMsg.msg_ = logMessage;

        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.size() >= 1024) return;
        queue_.push(std::move(aMsg));
        lock.unlock();
        cv_.notify_one();
    }

    void configure(const std::string & /*fnameXMLConfig*/, const std::string & /*xmlNode*/, bool /*init = false*/) {
        showTimestamps_ = true;

        enabledTagsStr_["App"] = true;
        enabledTagsStr_["BaseManager"] = true;

        if (useColors_) {
            colors_[DEFAULT]    = COL_DEFAULT;
            colors_[RED]        = COL_RED;
            colors_[YELLOW]     = COL_YELLOW;
            colors_[GREEN]      = COL_GREEN;
            colors_[BLUE]       = COL_BLUE;
            colors_[MAGENTA]    = COL_MAGENTA;
            colors_[CYAN]       = COL_CYAN;
        } else {
            colors_[DEFAULT]    = "";
            colors_[RED]        = "";
            colors_[YELLOW]     = "";
            colors_[GREEN]      = "";
            colors_[BLUE]       = "";
            colors_[MAGENTA]    = "";
            colors_[CYAN]       = "";
        }

        bufferPrefix_.resize(maxLogPrefixSize_);
        bufferMsg_.resize(maxLogMessageSize_);
    }

    // used by Logger
    std::mutex mutexPush_;
    std::vector<char> bufferMsg_;
    std::vector<char> bufferPrefix_;

private:
    enum TypeColor {
        DEFAULT = 0,
        RED,
        YELLOW,
        GREEN,
        BLUE,
        MAGENTA,
        CYAN,
    };

    struct Message {
        TypeMessage type_;

        uint64_t timestamp_;

        bool printPrefix_;

        bool hasTag_;
        bool hasTagInt_;
        bool hasTagStr_;

        int tagInt_;
        std::string tagStr_;

        std::vector<char> prefix_;
        std::vector<char> msg_;
    };

    void run() {
        Message msg;
        while (true) {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] {return !queue_.empty();});
                if (!running_) break;

                msg = std::move(queue_.front());
                queue_.pop();
            }

            printMsg(msg);
        }

        std::lock_guard<std::mutex> lock(mutex_);
        while (!queue_.empty()) {
            msg = std::move(queue_.front());
            queue_.pop();

            printMsg(msg);
        }
    }

    void printMsg(Message &msg) {
        if (msg.hasTag_) {
            if (msg.hasTagInt_) {
                if (enabledTagsInt_.find(msg.tagInt_) == enabledTagsInt_.end()) return;
                if (enabledTagsInt_[msg.tagInt_] == false) return;
            } else if (msg.hasTagStr_) {
                if (enabledTagsStr_.find(msg.tagStr_) == enabledTagsStr_.end()) return;
                if (enabledTagsStr_[msg.tagStr_] == false) return;
            } else {
                return;
            }
        }

        if (msg.printPrefix_) {
            if (showTimestamps_) {
                fprintf(stdout, "%s[%" PRIu64 "]%s ", colors_[BLUE].c_str(), msg.timestamp_, colors_[DEFAULT].c_str());
            }

            if (showTags_) {
                if (msg.hasTag_) {
                    if (msg.hasTagInt_) {
                        fprintf(stdout, "%s[%-16d]%s ", colors_[CYAN].c_str(), msg.tagInt_, colors_[DEFAULT].c_str());
                    } else {
                        fprintf(stdout, "%s[%-16s]%s ", colors_[CYAN].c_str(), msg.tagStr_.c_str(), colors_[DEFAULT].c_str());
                    }
                }
            }
        }

        msg.prefix_[msg.prefix_.size() - 1] = 0;
        msg.msg_[msg.msg_.size() - 1] = 0;
        if (msg.type_ == DEBUG) {
            if (msg.hasTag_) {
                fprintf(stdout, "%s%s%s%s", colors_[BLUE].c_str(), msg.prefix_.data(), colors_[DEFAULT].c_str(), msg.msg_.data());
            } else {
                fprintf(stdout, "%s%s%s[DEBUG]%s %s", colors_[BLUE].c_str(), msg.prefix_.data(), colors_[YELLOW].c_str(), colors_[DEFAULT].c_str(), msg.msg_.data());
            }
        } else if (msg.type_ == WARNING) {
            fprintf(stdout, "%s%s%s[WARNG]%s %s", colors_[BLUE].c_str(), msg.prefix_.data(), colors_[MAGENTA].c_str(), colors_[DEFAULT].c_str(), msg.msg_.data());
        } else if (msg.type_ == FATAL) {
            fprintf(stdout, "%s%s%s[FATAL]%s %s", colors_[BLUE].c_str(), msg.prefix_.data(), colors_[RED].c_str(), colors_[DEFAULT].c_str(), msg.msg_.data());
        } else {
            fprintf(stdout, "%s%s", msg.prefix_.data(), msg.msg_.data());
        }
        fflush(stdout);
    }

    int maxLogPrefixSize_;
    int maxLogMessageSize_;

    bool running_;
    bool useColors_;
    bool showTags_;
    bool showTimestamps_;
    bool showSignatures_;

    std::queue< Message > queue_;
    std::condition_variable cv_;
    std::thread worker_;
    std::mutex mutex_;
    std::mutex mutexResume_;

    std::map<TypeColor, std::string> colors_;

    std::map<int, bool>         enabledTagsInt_;
    std::map<std::string, bool> enabledTagsStr_;
};

Logger::Logger() {
    impl_.reset(new LoggerImplementation());

    impl_->bufferMsg_.resize(kMaxLogMessageSize);
    impl_->bufferPrefix_.resize(kMaxLogPrefixSize);
}

Logger::~Logger() {
    impl_.reset();
}

void Logger::logMessage(TypeMessage type, const char * /*file*/, long /*line*/, const int   tag, const char *func, const char *msg, ...) {
    std::lock_guard<std::mutex> lock(impl_->mutexPush_);
    va_list ap; va_start(ap, msg);
#ifdef CG_COLOR_ARGUMENTS
    std::stringstream ss;
    for (int i = 0; msg[i] != 0; ++i) {
        if (msg[i] == '%') {
            ss << COL_BOLD;
            while (msg[i] != ' ' && msg[i] != ')' && msg[i] != ']' && msg[i] != 0) ss << msg[i++];
            if (msg[i] == 0) break;
            ss << COL_DEFAULT;
        }
        ss << msg[i];
    }
    vsnprintf(impl_->bufferMsg_.data(), kMaxLogMessageSize, ss.str().c_str(), ap);
#else
    vsnprintf(impl_->bufferMsg_.data(), kMaxLogMessageSize, msg, ap);
#endif
    va_end(ap);

    if (func) {
        snprintf(impl_->bufferPrefix_.data(), kMaxLogPrefixSize, "[%-80s] ", func);
    } else {
        impl_->bufferPrefix_[0] = 0;
    }
    impl_->push(type, tag, impl_->bufferPrefix_, impl_->bufferMsg_);
}

void Logger::logMessage(TypeMessage type, const char * /*file*/, long /*line*/, const char *tag, const char *func, const char *msg, ...) {
    std::lock_guard<std::mutex> lock(impl_->mutexPush_);

    va_list ap; va_start(ap, msg);
#ifdef CG_COLOR_ARGUMENTS
    std::stringstream ss;
    for (int i = 0; msg[i] != 0; ++i) {
        if (msg[i] == '%') {
            ss << COL_BOLD;
            while (msg[i] != ' ' && msg[i] != ')' && msg[i] != ']' && msg[i] != 0) ss << msg[i++];
            if (msg[i] == 0) break;
            ss << COL_DEFAULT;
        }
        ss << msg[i];
    }
    vsnprintf(impl_->bufferMsg_.data(), kMaxLogMessageSize, ss.str().c_str(), ap);
#else
    vsnprintf(impl_->bufferMsg_.data(), kMaxLogMessageSize, msg, ap);
#endif
    va_end(ap);

    if (func) {
        snprintf(impl_->bufferPrefix_.data(), kMaxLogPrefixSize, "[%-80s] ", func);
    } else {
        impl_->bufferPrefix_[0] = 0;
    }
    impl_->push(type, tag, impl_->bufferPrefix_, impl_->bufferMsg_);
}

void Logger::configure(const std::string &fnameXMLConfig, const std::string &xmlNode) {
    impl_->pause();
    impl_->configure(fnameXMLConfig, xmlNode, false);
    impl_->print();
    impl_->resume();
}

void Logger::disableInstance() { impl_->disableInstance(); }

}
