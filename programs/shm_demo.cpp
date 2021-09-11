#include <iostream>
#include <shmstream.h>

struct demost{
    int ival;
    float fval;
    std::string sval;
};

int main(){
    shmIndexHash<demost> demoSHMHash;
    if(!demoSHMHash.BuildShmIndexHash(std::string("shmFileName"), 1024)){
        std::cout<<"shm created successfully"<<std::endl;
    }

    return 0;
}
