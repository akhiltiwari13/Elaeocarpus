#ifndef SMSTREAM_H
#define SMSTREAM_H

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <type_traits>
#include <utility>

template <typename T> class shmIndexHash {
public:
  /* initializes shm pointer to nullptr. */
  shmIndexHash() : m_pTPtr(nullptr) {}

  int BuildShmIndexHash(const std::string &szFileName, const int64_t nSize) {
    m_szFileName = szFileName;
    key_t lnKey = ftok(m_szFileName.c_str(), 'R');
    if (lnKey == -1) {
      std::cout << "ShmIndexHash error  ftok(" << szFileName
                << ") : " << strerror(errno) << std::endl;
      return -1;
    }

    bool lbStatus = false;
    const long m_nTotalSize = nSize * sizeof(T);
    m_shmId = shmget(lnKey, m_nTotalSize, IPC_CREAT | IPC_EXCL | 0666);
    if (m_shmId == -1 && errno == EEXIST) {
      m_shmId = shmget(lnKey, m_nTotalSize, IPC_CREAT | 0666);
      if (m_shmId == -1) {
        std::cout << "ShmIndexHash error shmget(): " << strerror(errno)
                  << " : KeyFile:" << szFileName << " :nKey:" << std::hex
                  << lnKey << " " << m_shmId << " Size " << m_nTotalSize << " "
                  << nSize << std::endl;
        return -1;
      }
    } else if (errno == 0) {
      std::cout << m_shmId << " " << errno << strerror(errno)
                << " : KeyFile:" << szFileName << std::endl;
      lbStatus = true;
    }

    /* m_pTPtr = (T *)shmat(m_shmId, NULL, 0); */
    m_pTPtr = std::make_shared<T>(shmat(m_shmId, NULL, 0));
    if (m_pTPtr == nullptr) {
      std::cout << "ShmIndexHash error shmat(): " << strerror(errno)
                << " : KeyFile:" << szFileName << std::endl;
      return -1;
    }

    if (lbStatus)
      memset(m_pTPtr.get(), 0, nSize * sizeof(T));

    std::cout << "ShmIndexHash " << szFileName << " " << m_shmId << std::endl;
    return 0;
  }

  /* T *operator[](const int64_t &nIndex) { return &m_pTPtr[nIndex]; } */
  std::shared_ptr<T> operator[](const int64_t &nIndex) {
    return &m_pTPtr[nIndex];
  }

  /* T *operator=(const int64_t &nIndex) { return m_pTPtr[nIndex]; } */
  std::shared_ptr<T> operator=(const int64_t &nIndex) {
    return m_pTPtr[nIndex];
  }

  T *getPointer() { return m_pTPtr.get(); }

public:
  int m_shmId;

private:
  /* T*                  m_pTPtr; */
  std::shared_ptr<T> m_pTPtr;
  int64_t m_nTotalSize;
  std::string m_szFileName;
};

#endif /* SMSTREAM_H */
