#include <ConfigReader.h>
#include <log.h>
#include <memLayoutOrder.h>

#include <string>

using namespace com::elaeocarpus::assortedUtils;

class X {
public:
  int dataMem1;

/* protected: */
  int dataMem2;
};

/* friend char * access_order(data_type1 class_type::*mem1, data_type2
 * class_type::*mem2); */

/* Sample program which demostrate how to use most of the libraries in this
 * project. */
int main(int argc, char **argv) {
  /* @todo: read required data from a config file. */
  std::string config("Config.ini");
  Logger::getLogger().setDump(std::string("./.SampleLog"), false);
  LOG_INFO("Sample Test Logging, successful!");

  /* X x; */

  /* std::cout << access_order(&X::dataMem2, &X::dataMem1); */

  return 0;
}
