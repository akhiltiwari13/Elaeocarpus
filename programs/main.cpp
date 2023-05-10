/* #include <ConfigReader.h> */
/* #include <log.h> */
/* #include <memLayoutOrder.h> */

#include <string>
#include <bitset>
#include <iostream>
#include <MemLayoutOrder.h>


class X {
    public:
        int dataMem1;

        /* protected: */
        int dataMem2;
};

/* friend char * access_order(data_type1 class_type::*mem1, data_type2
 * class_type::*mem2); */

// generic output operator for pairs (limited solution)
    template <typename T1, typename T2>
std::ostream& operator << (std::ostream& strm,
        const std::pair<T1,T2>& p)
{
    return strm << "[" << p.first << "," << p.second << "]";
}

// variadic function template to write data to ostream.
    template <typename T>
    void varPrint(std::ostream& ostrm, const T& arg){
        ostrm<< arg<<std::endl;
    }

    template <typename T, typename... types>
    void varPrint(std::ostream& ostrm, const T& fArg, const types&... args){
        ostrm<< fArg<<std::endl;
        varPrint(ostrm,args...);
    }

/* Sample program which demostrate how to use most of the libraries in this
 * project. */
int main(int argc, char **argv) {
    /* @todo: read required data from a config file. */
    std::string config("Config.ini");
    /* Logger::getLogger().setDump(std::string("./.SampleLog"), false); */
    /* LOG_INFO("Sample Test Logging, successful!"); */

    X x;

    std::cout << elaeocarpus::elaeoutils::access_order(&x.dataMem1, &x.dataMem2);

    /* std::cout<<"std::make_pair(34,56.0f):"<<std::make_pair(34,56.0f)<<std::endl; */
    /* varPrint(std::cout,"akhil", 23.6f, std::bitset<16>(325), "hello world!"); */

    return 0;
}
