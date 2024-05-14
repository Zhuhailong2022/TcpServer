#ifndef WORKER_THREAD_HPP
#define WORKER_THREAD_HPP
#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
#include<vector>
#include <iostream>
#include <thread>
#include <algorithm>
#include <mutex>
#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>
#include <map>
#include "Task.hpp"

class WorkerThread {
public:
    WorkerThread(int w_id) :id(w_id) {}
    ~WorkerThread();
    // 向子线程添加客户端套接字
    void addClientSocket(int clientSocket);
    // 查询当前管理的客户端套接字数量
    int getManagedClientCount();
    //移除套接字
    void removeClientrSocket(int clientSocket);
    void removeClientwSocket(int clientSocket);
    // 创建线程池
    void createThreadPool();

    // 停止线程池
    void stopThreadPool();

    // 任务执行函数
    void taskExecution();

    // 使用select检测客户端套接字是否有读写事件
    void checkClientSockets();

    //启动WorkerThread
    void start();
    //更改套接字状态的映射
    void set_socket_rstatus(const int client_socket, int status);
    void set_socket_wstatus(const int client_socket, int status);
private:
    std::vector<int> m_clientSockets;//线程管理的文件描述符集合
    std::map<int, int> socketrStatusMap;// 套接字状态映射，键为套接字文件描述符，值为套接字状态（0为空闲，1为忙碌）
    std::map<int, int> socketwStatusMap;
    int m_maxFd; // 最大文件描述符
    std::thread m_thread;//专门的线程用来运行检测管理客户端的读写事件函数
    bool m_exitFlag = false; // 用于控制检测管理客户端的读写事件函数退出的条件
    fd_set m_readSet; // 读事件集合
    std::mutex m_mutex; // 互斥锁
    std::queue<Task*> m_taskQueue; // 任务队列
    std::condition_variable m_condition; // 条件变量，用于线程同步
    std::vector<std::thread> m_threadPool; // 线程池
    int id;//标记本 workerthread是第几号
    std::atomic<bool> m_stopRequested{ false }; // 线程池停止标志
};

#endif // WORKER_THREAD_HPP
