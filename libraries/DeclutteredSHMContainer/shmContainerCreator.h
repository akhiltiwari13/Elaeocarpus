#ifndef SHMCONTAINERUSER_H_
#define SHMCONTAINERUSER_H_

#include<cassert>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include <string.h>

//typedef char[1024] queue_element;

template <class T>
class shmContainerCreator
{
  int _shmid;
  key_t _shmKey;
  char _shmKeyFilePath[1024];
  T* _ptrQ;
  public:

  template<class ...Args>
  shmContainerCreator(const char* shmKeyFilePath, Args&&... constructorArgs)
  {
    assert(strlen(shmKeyFilePath) <= 1024); // File name to long. Stop being so verbose
    strncpy(_shmKeyFilePath,shmKeyFilePath,1023);
    _shmKeyFilePath[1023] = '\0';

    if((_shmKey = ftok(_shmKeyFilePath,'R')) == -1)
    {
      std::cout << "error ftok " << _shmKeyFilePath << " : " << strerror(errno) << std::endl;
      perror("ftok() didn't work");
      exit(1);
    }

    bool lbStatus = false;
    _shmid = shmget(_shmKey, sizeof(T), IPC_CREAT | IPC_EXCL | 0666);      
    if (_shmid  == -1 && errno == EEXIST)
    {
      _shmid = shmget(_shmKey, sizeof(T), IPC_CREAT | 0666);      
      if(_shmid == -1)
      {
        std::cout << "error shmget(): "  << strerror(errno) <<" : KeyFile:"<< _shmKeyFilePath 
                  <<":nKey:"<<std::hex << _shmKey << std::endl;
        perror("shmget() didn't work");
        printf("%s\n",_shmKeyFilePath);
        exit(1);
      }
    }
    else if (errno == 0)
    {
      lbStatus = true;      
    }
     
    _ptrQ = (T *)shmat(_shmid, NULL, 0);
    if(_ptrQ == (T *)(-1))
    {
      std::cout << "error shmat(): " << strerror(errno) <<" : KeyFile:"<< _shmKeyFilePath<< std::endl;
      perror("shmat() didn't work");
      exit(1);
    }
    
    std::cout << "shmContainer " << _shmKeyFilePath << " " << _shmid << std::endl;

    //int32_t* lpcInt32 = (int32_t*)_ptrQ;
    if(lbStatus) 
    {
      new (_ptrQ) T(std::forward<Args>(constructorArgs)...);
    }
  };

  T* getPointer(){return _ptrQ;};
};
#endif

