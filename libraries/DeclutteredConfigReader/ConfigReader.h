#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class ConfigReader {
private:
  std::map<std::string, std::string> _propValMap;
  std::string _filename;
  void parseFile();
  bool isFileParsed;

public:
  explicit ConfigReader(const std::string &);

  void addProperty(std::string &property, std::string &value);
  void setProperty(std::string &property, std::string &value);
  void delProperty(std::string property);

  inline void ltrim(std::string &s) {
    s.erase(s.begin(),
            std::find_if(s.begin(), s.end(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))));
  }

  inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace)))
                .base(),
            s.end());
  }

  inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
  }

  inline bool checkAllKeysPresent(std::vector<std::string> keysList) {
    std::string val("");
    bool isConfigOk = true;
    if (isFileParsed) {
      for (auto &key : keysList) {
        trim(key);
        val = _propValMap[key];
        if (val.empty()) {
          std::cout << "Missing key: " << key.c_str() << " " << val.c_str()
                    << std::endl;
          isConfigOk = false;
        }
      }
    } else {
      parseFile();
      isFileParsed = true;
      return checkAllKeysPresent(keysList);
    }
    return isConfigOk;
  }

  std::string getProperty(std::string property);
  void reset();
  void dump();
  inline int count() const { return _propValMap.size(); }
};

#endif
