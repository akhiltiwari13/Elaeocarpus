#include <ConfigReader.h>

#include <fstream>
#include <cctype>
#include <algorithm>

void ConfigReader::parseFile()
{
  std::ifstream configFile(_filename.c_str());
  std::string line;

  if(configFile.is_open())
  {
    while(getline(configFile, line))
    {
      /* remove white space */
      line.erase(std::remove_if(line.begin(), line.end(),(int(*)(int))isspace),line.end());
      
      /* Ignoring lines that start with a #. use # for comments. */
      if(line[0] == '#')
      {
        continue;
      }
      std::string::size_type pos;
      std::string property;
      std::string value;
      
      if((pos=line.find_first_of('='))!=std::string::npos)
      {
        property = line.substr(0,pos);
        value = line.substr(pos+1,line.length());
        addProperty(property, value);
      }
      /* Ignore bad lines */
      else
      {
        continue;
      }
    }
  }
  else{
      std::cout<<"ERROR! Cannot open file."<<std::endl;
  }
};
  
void ConfigReader::addProperty( std::string& property, std::string& value)
{
  _propValMap[property] = value;
};

void ConfigReader::setProperty(std::string& property, std::string& value)
{
  _propValMap[property] = value;
};
  
void ConfigReader::delProperty(std::string property)
{
  _propValMap.erase(property);
};

std::string ConfigReader::getProperty(std::string property)
{
  return _propValMap[property];
};
void ConfigReader::reset()
{
  _propValMap.clear();
};

void ConfigReader::dump()
{
  for(std::map<std::string,std::string>::iterator it=_propValMap.begin() ; it!=_propValMap.end(); ++it)
  {
    std::cout<<it->first << "=" <<it->second <<std::endl;
  }
};

ConfigReader::ConfigReader(const std::string& filename):_filename(filename)
{
  parseFile();
};
