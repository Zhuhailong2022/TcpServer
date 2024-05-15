#ifndef TASK_HPP
#define TASK_HPP
#define READ 1
#define WRITE 2
#include <WinSock2.h>
#include <iostream>

//#include "WorkerThread.hpp" // 包含 WorkerThread 类的头文件
class WorkerThread;
class Task {
public:
    Task(int clientSocket,int type,WorkerThread* ptr) : m_clientSocket(clientSocket), TYPE(type), m_workerThread(ptr){}

    // 执行任务
    void execute();
    //数据处理函数
    void processData(const char* data, int len);

private:
    int m_clientSocket; // 客户端套接字文件描述符
    int TYPE = 0;
    WorkerThread* m_workerThread; // 指向 WorkerThread 对象的指针，也可以理解为标记该任务对象是由哪个子线程创建的，用于执行增删fd文件描述符
                                   
    //读缓冲区、写缓冲区
    static const int MAX_BUFFER_SIZE = 4096;
    char buffer[MAX_BUFFER_SIZE];
    int totalBytesReceived = 0;
};

#endif // TASK_HPP
