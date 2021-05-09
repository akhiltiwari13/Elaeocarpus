#include <log.h>
#include <ConfigReader.h>

#include <string>

/* Sample program which demostrate how to use most of the libraries in this project. */
int main(int argc, char **argv){
    /* @todo: read required data from a config file. */
    std::string config("Config.ini");
    Logger::getLogger().setDump(std::string("./.SampleLog"), false);
    LOG_INFO("Sample Test Logging, successful!");

    return 0;
}
