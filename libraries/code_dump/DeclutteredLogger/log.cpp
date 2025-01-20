#include <cstdarg>
#include <fstream>
#include <iostream>
#include <log.h>
#include <setaffinity.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

std::unique_ptr<Logger> Logger::_instance;
std::once_flag Logger::_onceFlag;

Logger::Logger()
    : _logFilePath("/tmp/"), _logLevel(INFO), _size(QUEUE_SIZE),
      _flag(static_cast<std::atomic<bool> *>(
          std::malloc(sizeof(std::atomic<bool>) * QUEUE_SIZE))),
      _readIndex(0), _writeIndex(0), _coreID(0),
      _records(static_cast<LogMsg *>(std::malloc(sizeof(LogMsg) * QUEUE_SIZE))),
      _exit(false) {
  assert(QUEUE_SIZE >= 2);
  if (!_records || !_flag) {
    throw std::bad_alloc();
  }
  for (int i = 0; i < QUEUE_SIZE; i++) {
    _flag[i].store(false);
  }

  initEncryption();
};

void Logger::setLevel(const int &level) {
  if (level < 0 || level > 4) {
    return;
  }
  _logLevel = level;
  if (level <= TRACE) {
    IsTraceLogEnable() = true;
  } else {
    IsTraceLogEnable() = false;
    if (level <= DEBUG) {
      IsDebugLogEnable() = true;
    } else {
      IsDebugLogEnable() = false;
    }
  }
};

void Logger::setCoreID(const int &coreID) { _coreID = coreID; }

Logger::~Logger() {
  exit();
  if (_bgthread.joinable()) {
    _bgthread.join();
  }
  delete _records;
  /* fclose (m_pcFile); */
}

void Logger::log(const int &level, const std::string &message) {
  if ((level < _logLevel) || (level > FATAL)) {
    return;
  }
  auto const currentWrite =
      std::atomic_fetch_add(&_writeIndex, (unsigned long long)1);
  auto queuePos = currentWrite % _size;
  while (_readIndex + _size <= _writeIndex) {
    sched_yield();
  }
  new (&_records[queuePos]) LogMsg(level, message);
  _flag[queuePos].store(true);
}

void Logger::log(const int &level, const LogBuffer &logRecord) {
  if ((level < _logLevel) || (level > FATAL)) {
    return;
  }
  auto const currentWrite =
      std::atomic_fetch_add(&_writeIndex, (unsigned long long)1);
  auto queuePos = currentWrite % _size;
  while (_readIndex + _size <= _writeIndex) {
    sched_yield();
  }
  new (&_records[queuePos]) LogMsg(level, logRecord.record_);
  _flag[queuePos].store(true);
}

void Logger::log(const int &level, const LoggerMessage &loggerMessage) {
  if (level != PERF) {
    return;
  }

  auto const currentWrite =
      std::atomic_fetch_add(&_writeIndex, (unsigned long long)1);
  auto queuePos = currentWrite % _size;
  while (_readIndex + _size <= _writeIndex) 
  {
    sched_yield();
  }
  new (&_records[queuePos]) LogMsg(level, loggerMessage);
  _flag[queuePos].store(true);
}

void Logger::log(short level, int line, const char *file, const char *format,
                 ...) {
  if ((level < _logLevel) || (level > FATAL)) {
    return;
  }

  LogBuffer logBuffer;
  va_list vl;
  va_start(vl, format);
  vsprintf(logBuffer.record_, format, vl);
  va_end(vl);
  if (!_logFilePath.empty()) {
    log(level, logBuffer.record_);
  } else {
    std::cout << levels[level]
              << " "  << logBuffer.record_
              << std::endl;
  }
}

void Logger::exit(void) { _exit = true; }

void Logger::backgroundThread() {
  std::string errstring("Logger: Pinned to Core:" + std::to_string(_coreID));
  ;
  TaskSet((unsigned int)_coreID, 0, errstring);
  LOG_INFO(errstring.c_str());

  time_t now = time(0);
  struct tm tstruct;
  char cfilename[80];
  tstruct = *localtime(&now);
  strftime(cfilename, sizeof(cfilename), "_%Y%m%d", &tstruct);
  std::string filename(cfilename);

  filename = _logFilePath + filename; // + std::string(".log");
  m_pcFile = fopen(filename.c_str(), "a+");

  if (!m_pcFile) {
    std::cout << "ERROR:Failed to open log file :" << filename << std::endl;
    return;
  }

  LogMsg record;
  char perfMessage[1024 + 1];
  memset(perfMessage, 0, 1024 + 1);

  while (!_exit) {
    auto currentReadIndex =
        std::atomic_fetch_add(&_readIndex, (unsigned long long)1);
    auto currentRead = currentReadIndex % _size;
    auto status = _flag[currentRead].load();
    while (!_exit && status == false) {
      status = _flag[currentRead].load();
    }
    record = std::move(_records[currentRead]);
    _records[currentRead].~LogMsg();
    _flag[currentRead].store(false);

    memset(&_binaryLogMsg, 0, sizeof(BinaryLogMsg));
    gettimeofday(&_binaryLogMsg.tv, NULL);
    localtime_r(&_binaryLogMsg.tv.tv_sec, &_binaryLogMsg.tm);

    _binaryLogMsg.size = sizeof(BinaryLogMsg);
    strncpy(_binaryLogMsg.level, levels[record._level].c_str(), 10);
    if (record._level == PERF) {
      memset(perfMessage, 0, 1024 + 1);
      memcpy(_binaryLogMsg.msg, perfMessage, LOG_MSG_SIZE);
      _binaryLogMsg.msg[LOG_MSG_SIZE - 1] = '\0';
    } else {
      memcpy(_binaryLogMsg.msg, record.loggerMessage_.logMessageBuffer._message,
             LOG_MSG_SIZE);
    }
    (this->*_writeLogFunc)(_binaryLogMsg);
  }

  auto currentReadIndex =
      std::atomic_fetch_add(&_readIndex, (unsigned long long)1);
  auto currentRead = currentReadIndex % _size;
  auto status = _flag[currentRead].load();
  while (status != false) {
    record = std::move(_records[currentRead]);
    _records[currentRead].~LogMsg();
    _flag[currentRead].store(false);

    memset(&_binaryLogMsg, 0, sizeof(BinaryLogMsg));
    gettimeofday(&_binaryLogMsg.tv, NULL);
    localtime_r(&_binaryLogMsg.tv.tv_sec, &_binaryLogMsg.tm);

    _binaryLogMsg.size = sizeof(BinaryLogMsg);
    strncpy(_binaryLogMsg.level, levels[record._level].c_str(), 10);
    if (record._level == PERF) {
      memset(perfMessage, 0, 1024 + 1);
      memcpy(_binaryLogMsg.msg, perfMessage, LOG_MSG_SIZE);
      _binaryLogMsg.msg[LOG_MSG_SIZE - 1] = '\0';
    } else {
      memcpy(_binaryLogMsg.msg, record.loggerMessage_.logMessageBuffer._message,
             LOG_MSG_SIZE);
    }
    (this->*_writeLogFunc)(_binaryLogMsg);

    currentReadIndex =
        std::atomic_fetch_add(&_readIndex, (unsigned long long)1);
    currentRead = currentReadIndex % _size;
    status = _flag[currentRead].load();
  }
};

bool Logger::setDump(const std::string &filePathPrefix,
                     const bool enableBinaryLog) {
  std::size_t found = filePathPrefix.find_last_of("/\\");
  std::string strFileUpload = filePathPrefix.substr(0, found + 1);
  std::cout << "strFileUpload:" << strFileUpload << std::endl;
  int result = access(strFileUpload.c_str(), W_OK);
  if (result == 0) {
    _logFilePath = filePathPrefix;
  } else {
    return false;
  }

  if (enableBinaryLog) {
    _writeLogFunc = &Logger::writeBinaryLog;
  } else {
    _writeLogFunc = &Logger::writeStringLog;
  }

  _bgthread = std::thread(&Logger::backgroundThread, _instance.get());

  return true;
};

void Logger::setDump() {
  _bgthread = std::thread(&Logger::backgroundThread, _instance.get());
};

Logger &Logger::getLogger() {
  std::call_once(_onceFlag, [] { _instance.reset(new Logger); });
  return *_instance.get();
};
