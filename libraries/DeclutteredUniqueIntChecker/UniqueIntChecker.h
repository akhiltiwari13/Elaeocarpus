#ifndef UNIQUEINTCHECKER_H
#define UNIQUEINTCHECKER_H
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>




namespace com {
namespace elaeocarpus {
namespace decluttered {

const uint64_t BIN_POWER[64] = {
    1ull,
    2ull,
    4ull,
    8ull,
    16ull,
    32ull,
    64ull,
    128ull,
    256ull,
    512ull,
    1024ull,
    2048ull,
    4096ull,
    8192ull,
    16384ull,
    32768ull,
    65536ull,
    131072ull,
    262144ull,
    524288ull,
    1048576ull,
    2097152ull,
    4194304ull,
    8388608ull,
    16777216ull,
    33554432ull,
    67108864ull,
    134217728ull,
    268435456ull,
    536870912ull,
    1073741824ull,
    2147483648ull,
    4294967296ull,
    8589934592ull,
    17179869184ull,
    34359738368ull,
    68719476736ull,
    137438953472ull,
    274877906944ull,
    549755813888ull,
    1099511627776ull,
    2199023255552ull,
    4398046511104ull,
    8796093022208ull,
    17592186044416ull,
    35184372088832ull,
    70368744177664ull,
    140737488355328ull,
    281474976710656ull,
    562949953421312ull,
    1125899906842624ull,
    2251799813685248ull,
    4503599627370496ull,
    9007199254740992ull,
    18014398509481984ull,
    36028797018963968ull,
    72057594037927936ull,
    144115188075855872ull,
    288230376151711744ull,
    576460752303423488ull,
    1152921504606846976ull,
    2305843009213693952ull,
    4611686018427387904ull,
    9223372036854775808ull
};
//set bit

inline void setBit(uint64_t& number, char position) {
    /* LOG_DEBUG("Set called at address %" PRIu64 " with value %" PRIu64 " and with position at %d", */ 
    printf("Set called at address %" PRIu64 " with value %" PRIu64 " and with position at %d", &number, number, position);
    number |= BIN_POWER[position];
}

//clear bit

inline void clearBit(uint64_t& number, char position) {
    number &= ~(BIN_POWER[position]);
}
//toggle

inline void toggleBit(uint64_t& number, char position) {
    number ^= BIN_POWER[position];
}
//check bit
inline bool checkBit(uint64_t& number, char position) {
    if ((number >> position) & 1ull){
            /* LOG_DEBUG("checkBit called at address %" PRIu64 " with value %" PRIu64 " and with position at %d evaluates to %" PRIu64 ".", */ 
            printf("checkBit called at address %" PRIu64 " with value %" PRIu64 " and with position at %d evaluates to %" PRIu64 ".", 
            &number, number, position, (BIN_POWER[position]) & 1ull );
        return true;
    }
    else return false;
}

class UniqueIdChecker {
private:

    UniqueIdChecker(const UniqueIdChecker&) = delete;

    /*
     * Function to calculate base^exponent using recursion
     */
    int64_t getPower(int base, int exponent);

    bool fresh_, initState_, duplicateCheck_;
    int indexFd_, dataFd_;
    int loadFactor_; // 22 for 4194304
    std::string persistLocation_;
    std::string name_;
    uint64_t* elements_;
    size_t memSize_;
    UniqueIdChecker& operator=(const UniqueIdChecker&) = delete;

public:

    UniqueIdChecker() : fresh_(false),
    initState_(false), duplicateCheck_(false), indexFd_(0), dataFd_(0), loadFactor_(31),
    persistLocation_("/tmp"), name_("UniqueIdChecker"), elements_(0), memSize_(0) {
    }

    inline void SetPersistLocation(const std::string& location) {
        persistLocation_ = location;
    }
    
    inline void SetIgnoreDuplicateCheck(const bool duplicateCheck) {
        duplicateCheck_ = duplicateCheck;
    }

    bool init(const std::string& sessionName ) {        
        if (initState_ == true) {
            /* LOG_INFO("UniqueIdChecker::init() - Duplicate initialisation."); */
            printf("UniqueIdChecker::init() - Duplicate initialisation.\n");
            return true;
        }
        int result = 0;
        this->name_ = sessionName;
        // Based on data packing the size of memory map coupld vary. This has to be programmed later looking at alignment property.
        // For now it is a simple multiplication
        char cfilename[80];
        time_t now = time(0);
        struct tm tstruct;
        tstruct = *localtime(&now);
        memset(cfilename, 0, 80);
        strftime(cfilename,sizeof(cfilename),"_%Y%m%d",&tstruct);
        std::string indexFile = persistLocation_ + "/" + name_ + cfilename +"cl_id_store.idx";

        indexFd_ = open(indexFile.c_str(), O_RDWR, (mode_t) 0600);
        if (indexFd_ == -1) {
            indexFd_ = open(indexFile.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
            if (indexFd_ == -1) {
                /* LOG_ERROR("Error opening file for writing"); */
                printf("Error opening file for writing");
                exit(EXIT_FAILURE);
            }
            fresh_ = true;
        }
        if (fresh_) {
            result = lseek(indexFd_, 1024 - 1, SEEK_SET);
            if (result == -1) {
                close(indexFd_);
                /* LOG_ERROR("Error calling lseek() to 'stretch' the file"); */
                printf("Error calling lseek() to 'stretch' the file");
                exit(EXIT_FAILURE);
            }
            result = write(indexFd_, "", 1);
            if (result != 1) {
                close(indexFd_);
                /* LOG_ERROR("Error writing last byte of the file"); */
                printf("Error writing last byte of the file");
                exit(EXIT_FAILURE);
            }
        }
        
        long pageSize = sysconf(_SC_PAGESIZE);

        std::string datFile = persistLocation_ + "/" + name_ + cfilename + "cl_id_store.dat";
        memSize_ = getPower(2, loadFactor_) / sizeof (uint64_t);
        if (fresh_) {
            dataFd_ = open(datFile.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
            if (dataFd_ == -1) {
                /* LOG_ERROR("Error opening file for writing"); */
                printf("Error opening file for writing");
                exit(EXIT_FAILURE);
            }
            result = lseek(dataFd_, memSize_ - 1, SEEK_SET);
            if (result == -1) {
                close(dataFd_);
                /* LOG_ERROR("Error calling lseek() to 'stretch' the file"); */
                printf("Error calling lseek() to 'stretch' the file");
                exit(EXIT_FAILURE);
            }
            result = write(dataFd_, "", 1);
            if (result != 1) {
                close(dataFd_);
                /* LOG_ERROR("Error writing last byte of the file"); */
                printf("Error writing last byte of the file");
                exit(EXIT_FAILURE);
            }
        } else {
            /* LOG_DEBUG("Mapping named memory: %s", datFile.c_str()); */
            printf("Mapping named memory: %s", datFile.c_str());
            dataFd_ = open(datFile.c_str(), O_RDWR, (mode_t) 0600);
            if (dataFd_ == -1) {
                /* LOG_ERROR("Mapping named memory failed.: %s", datFile.c_str()); */
                printf("Mapping named memory failed.: %s", datFile.c_str());
                exit(EXIT_FAILURE);
            }
        }
        /* LOG_INFO("Attaching memory of size: %" PRIu64 "", memSize_); */
        printf("Attaching memory of size: %" PRIu64 "", memSize_);
        elements_ = (uint64_t*) mmap(0, memSize_, PROT_READ | PROT_WRITE, MAP_SHARED, dataFd_, 0);
        /* LOG_INFO("Address of memory map: %" PRIu64 "", elements_); */
        printf("Address of memory map: %" PRIu64 "", elements_);
        
        if (elements_ == MAP_FAILED) {
            /* LOG_ERROR("Mapping to ClientOrderId Store record failed.: %s. Errno :%s", datFile.c_str(), strerror(errno)); */
            printf("Mapping to ClientOrderId Store record failed.: %s. Errno :%s", datFile.c_str(), strerror(errno));
            close(dataFd_);
            exit(EXIT_FAILURE);
        }
        if (fresh_) {
              for (size_t i = 0; i < memSize_ / sizeof (uint64_t); ++i){
                elements_[i] = 0;
//              for (int i = 0; i < 2147483647; ++i)
//                 resetBit(i);
              }
            /* LOG_INFO("Sync map file to zero. Status : %d ",msync(elements_, memSize_,MS_SYNC)); */
            printf("Sync map file to zero. Status : %d ",msync(elements_, memSize_,MS_SYNC));
        }

        initState_ = true;
        return initState_;
    }

    bool ifNotDuplicateInsert(int id) {
        if(!duplicateCheck_)
          return false;
        
        int arrayIndex = id / 64;
        char position = id % 64;
        /* LOG_DEBUG("Position at %d and offset at %d Number:%d", arrayIndex, position, id); */
        printf("Position at %d and offset at %d Number:%d", arrayIndex, position, id);

        // Prevent -ve numbers . 
        if(id < -1 )
        {
            return true;
        }//False probing special case
        else if (id == -1) {
            return false;
        }
        // 0 or more allowed
        if (checkBit(elements_[arrayIndex], position)) // Duplicate
        {
            return true;
        } else {
            setBit(elements_[arrayIndex], position);
            return false;
        }
    }

    ~UniqueIdChecker() {
        if (initState_) {
            msync(elements_, memSize_,MS_SYNC);
            if (munmap(elements_, memSize_) == -1) {
                perror("Error un-mmapping the file");
                /* LOG_ERROR("UniqueIdChecker::~UniqueIdChecker(). Memory Unmap failed."); */
                printf("UniqueIdChecker::~UniqueIdChecker(). Memory Unmap failed.");
            }
            close(dataFd_);
        }
    }
};

}
}
}
#endif /* UNIQUEINTCHECKER_H */

