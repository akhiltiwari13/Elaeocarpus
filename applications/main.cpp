/* #include <ConfigReader.h> */
/* #include <log.h> */
/* #include <memLayoutOrder.h> */

#include <string>
#include <bitset>
#include <iostream>
// #include <MemLayoutOrder.h>


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
int main() {
    /* @todo: read required data from a config file. */
    std::string config("Config.ini");
    /* Logger::getLogger().setDump(std::string("./.SampleLog"), false); */
    /* LOG_INFO("Sample Test Logging, successful!"); */


    /* std::cout<<"std::make_pair(34,56.0f):"<<std::make_pair(34,56.0f)<<std::endl; */
    /* varPrint(std::cout,"akhil", 23.6f, std::bitset<16>(325), "hello world!"); */

    return 0;
}
