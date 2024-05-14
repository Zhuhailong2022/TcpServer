#ifndef MAIN_THREAD_HPP
#define MAIN_THREAD_HPP
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#pragma comment (lib, "ws2_32.lib")
#include <WinSock2.h>
#include "WorkerThread.hpp" // 包含子线程类的头文件
#include<vector>
#include <iostream>
class MainThread {
public:
    MainThread();
    ~MainThread();
    void run();
private:
    int m_listenSocket;
    std::vector<WorkerThread*> m_workerThread_arr;

    fd_set m_masterSet;
    int m_maxFd;
};

#endif // MAIN_THREAD_HPP
