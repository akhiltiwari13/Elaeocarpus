#ifndef LOG_H_
#define LOG_H_

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <sched.h>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

#define LOG_MSG_SIZE 255

#define QUEUE_SIZE 1000000

const int TRACE = 0, DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4, FATAL = 5,
          PERF = 6;
const std::string levels[] = {"TRACE", "DEBUG", "INFO ", "WARN ",
                              "ERROR", "FATAL", "PERF "};

#define LOG_FATAL(_format_, ...)                                               \
  Logger::getLogger().log(FATAL, __LINE__, __BASE_FILE__, _format_,            \
                          ##__VA_ARGS__)
#define LOG_ERROR(_format_, ...)                                               \
  Logger::getLogger().log(ERROR, __LINE__, __BASE_FILE__, _format_,            \
                          ##__VA_ARGS__)
#define LOG_WARN(_format_, ...)                                                \
  Logger::getLogger().log(WARN, __LINE__, __BASE_FILE__, _format_,             \
                          ##__VA_ARGS__)
#define LOG_INFO(_format_, ...)                                                \
  Logger::getLogger().log(INFO, __LINE__, __BASE_FILE__, _format_,             \
                          ##__VA_ARGS__)

struct LOGGER_HEADER {
  int64_t _nMsgCode;
  LOGGER_HEADER() { _nMsgCode = -1; };
};

typedef struct logBuffer {
  char record_[1024];
} LogBuffer;

#pragma pack(2)

typedef struct logMessageBuffer {
  LOGGER_HEADER _header;
  char _message[LOG_MSG_SIZE];
} LogMessageBuffer;

typedef union _LoggerMessage {
  LOGGER_HEADER _header;

  LogMessageBuffer logMessageBuffer;
  /* DMA_Client_Heart_Beat_Log       _dma_Client_Heart_Beat_Log; */
  _LoggerMessage(){};

} LoggerMessage;

struct BinaryLogMsg {
  int size;
  struct timeval tv;
  struct tm tm;
  char level[10];
  char msg[LOG_MSG_SIZE];
};
#pragma pack()

class LogMsg {
public:
  LoggerMessage loggerMessage_;
  uint32_t _level;

  LogMsg(int level, std::string message) {
    _level = level;
    loggerMessage_.logMessageBuffer._header._nMsgCode = 0;
    strncpy(loggerMessage_.logMessageBuffer._message, message.c_str(),
            LOG_MSG_SIZE);
    loggerMessage_.logMessageBuffer._message[LOG_MSG_SIZE - 1] = '\0';
  };

  LogMsg(int level, const LogBuffer &message) {
    _level = level;
    loggerMessage_.logMessageBuffer._header._nMsgCode = 0;
    strncpy(loggerMessage_.logMessageBuffer._message, message.record_,
            LOG_MSG_SIZE);
    loggerMessage_.logMessageBuffer._message[LOG_MSG_SIZE - 1] = '\0';
  };

  LogMsg(int level, const LoggerMessage &message) {
    _level = level;
    memcpy(&loggerMessage_, &message, sizeof(LoggerMessage));
  };

  LogMsg() {
    memset(loggerMessage_.logMessageBuffer._message, '\0', LOG_MSG_SIZE);
    _level = -1;
  }

  void set(int level, std::string message) {
    _level = level;
    strncpy(loggerMessage_.logMessageBuffer._message, message.c_str(),
            LOG_MSG_SIZE);
    loggerMessage_.logMessageBuffer._message[LOG_MSG_SIZE - 1] = '\0';
  };

  void reset() {
    loggerMessage_.logMessageBuffer._header._nMsgCode = 0;
    memset(loggerMessage_.logMessageBuffer._message, '\0', LOG_MSG_SIZE);
    _level = -1;
  };
};

class Logger {
  Logger();
  Logger(const Logger &);
  Logger &operator=(const Logger &);

  void backgroundThread();

  std::string _logFilePath;
  std::thread _bgthread;

  int _logLevel;

  static std::unique_ptr<Logger> _instance;
  static std::once_flag _onceFlag;

  const uint32_t _size;
  std::atomic<bool> *const _flag;
  std::atomic<unsigned long long> _readIndex;
  std::atomic<unsigned long long> _writeIndex;

  int _coreID;
  LogMsg *const _records;

  bool _exit;
  FILE *m_pcFile;

  // BINARY LOGGING
  BinaryLogMsg _binaryLogMsg;
  void (Logger::*_writeLogFunc)(const BinaryLogMsg &);

public:
  ~Logger();
  static Logger& getLogger();
  bool setDump(const std::string &filePathPrefix,
               const bool enableBinaryLog /*= true*/); // BINARY LOGGING IN IE
                                                       // AU 20151109
  void setDump();
  void setLevel(const int &level);

  int getLevel() const { return _logLevel; }
  void setCoreID(const int &coreID);

  void log(const int &level, const std::string &message);
  void log(const int &level, const LogBuffer &logRecord);
  void log(const int &level, const LoggerMessage &loggerMessage);
  void log(short level, int line, const char *file, const char *format, ...);
  void exit(void);

  inline void writeBinaryLog(const BinaryLogMsg &logMsg) {
    writeStringLog(logMsg);
  }

  inline void writeStringLog(const BinaryLogMsg &logMsg) {
    char lcDateString[32 + 1] = {0};
    snprintf(lcDateString, sizeof(lcDateString), "%02d:%02d:%02d.%06ld",
             logMsg.tm.tm_hour, logMsg.tm.tm_min, logMsg.tm.tm_sec,
             logMsg.tv.tv_usec);

    std::string logMsgString = "[" + std::string(logMsg.level) + "] [" +
                               lcDateString + "] " + logMsg.msg + "\n";

    fwrite(logMsgString.c_str(), 1, logMsgString.size(), m_pcFile);
    fflush(m_pcFile);
  }

  void initEncryption() {}
};

static volatile bool &IsTraceLogEnable() {
  static volatile bool bTraceLogEnable =
      (Logger::getLogger().getLevel() <= TRACE) ? true : false;
  return bTraceLogEnable;
}

static volatile bool &IsDebugLogEnable() {
  static volatile bool bDebugLogEnable =
      (Logger::getLogger().getLevel() <= DEBUG) ? true : false;
  return bDebugLogEnable;
}

#define LOG_TRACE(_format_, ...)                                               \
  {                                                                            \
    if (IsTraceLogEnable()) {                                                  \
      Logger::getLogger().log(TRACE, __LINE__, __BASE_FILE__, _format_,        \
                              ##__VA_ARGS__);                                  \
    }                                                                          \
  }

#define LOG_DEBUG(_format_, ...)                                               \
  {                                                                            \
    if (IsDebugLogEnable()) {                                                  \
      Logger::getLogger().log(DEBUG, __LINE__, __BASE_FILE__, _format_,        \
                              ##__VA_ARGS__);                                  \
    }                                                                          \
  }

#endif
