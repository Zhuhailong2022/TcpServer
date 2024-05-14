#include "WorkerThread.hpp"

WorkerThread::~WorkerThread() 
{

    stopThreadPool();
    m_exitFlag = true;//修改 检测客户端函数执行的标志，停止检测。
    // 等待线程执行完毕并退出
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

//向集合中添加新增客户端的套接字
void WorkerThread::addClientSocket(int clientSocket) {
    // 加锁保护共享资源
    std::lock_guard<std::mutex> lock(m_mutex);
    m_clientSockets.push_back(clientSocket);
    socketrStatusMap[clientSocket] = 0;
    socketwStatusMap[clientSocket] = 0;
}
//给主线程检查子线程中管理了多少个套接字接口
int WorkerThread::getManagedClientCount()  {
    return m_clientSockets.size();
}
// 创建线程池
void WorkerThread::createThreadPool()
{
    // 创建线程池，这里假设使用4个线程
    for (int i = 0; i < 5; ++i) {
        std::cout << "create thread" << i << std::endl;
        m_threadPool.emplace_back(&WorkerThread::taskExecution, this);
    }
}

// 停止线程池
void WorkerThread::stopThreadPool()
{
    // 设置停止标志为 true
    m_stopRequested = true;

    // 等待所有线程结束
    for (auto& thread : m_threadPool) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// 使用select检测客户端套接字是否有读写事件
void WorkerThread::checkClientSockets()
{
    while (!m_exitFlag)
    {
        fd_set readfds;
        fd_set writefds;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        int maxSocket = -1;

        if (m_clientSockets.size() == 0) {
            //std::cerr << "No client sockets to check" << std::endl;
            Sleep(10);
            continue;
        }

        // 将合适的客户端套接字添加到读写事件检测集合中
        for (int clientSocket : m_clientSockets) {
            ////检测clientSocket处于什么状态，只有在其为空闲状态 0 时才将其添加到读写套接字检测集合中
            std::lock_guard<std::mutex> lock(m_mutex);
            if (socketrStatusMap[clientSocket] == 0)
            {
                FD_SET(clientSocket, &readfds);
            }
            if (socketwStatusMap[clientSocket] == 0)
            {
                FD_SET(clientSocket, &writefds);
            }
            if (clientSocket > maxSocket) {
                maxSocket = clientSocket;
                //std::cout << "update maxSocket" << std::endl;
            }
        }

        if (maxSocket == -1) continue;

        // 设置超时时间为0，表示立即返回
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;



        // 调用 select 函数检测套接字是否有读事件
        int result = select(maxSocket + 1, &readfds, &writefds, nullptr, &timeout);
        if (result == -1) {
            // 错误处理
            std::cerr << "Error in WorkerThread select function: " << errno << std::endl;
            return;
            //std::cerr << "Error in WorkerThread"<< id <<" select function" << std::endl;
            //return;
        }

        if (result > 0) {

            //std::cout << "result : " << result << std::endl;
            // 有套接字有读事件发生
            for (auto it :m_clientSockets) {
                int clientSocket = it;
                //如果有读事件
                if (FD_ISSET(clientSocket, &readfds)) {
                    // 处理客户端套接字的读事件
                    Task* task =new Task(clientSocket, READ, this);
                    //设置读取忙状态
                    set_socket_rstatus(clientSocket, 1);
                    //set_socket_status(clientSocket, 1);//将套接字状态设置为忙碌状态
                    //加锁向任务队列增加任务，设置套接字状态
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_taskQueue.push(task);
                    std::cout << "read "<< clientSocket<<" task" << std::endl;
                    std::cout << "m_taskQueue: " << m_taskQueue.size()  << std::endl;
                    // 解锁
                    lock.unlock();
                    // 唤醒等待的线程
                    m_condition.notify_all();
                }
                //如果可写
                if (FD_ISSET(clientSocket, &writefds))
                {
                    Task* task = new Task(clientSocket, WRITE, this);
                    //写入忙状态
                    set_socket_wstatus(clientSocket, 1);
                    //加锁向任务队列增加任务
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_taskQueue.push(task);
                    std::cout << "write m_taskQueue: " << m_taskQueue.size() << std::endl;
                    std::cout << "write " << clientSocket << "task" << std::endl;
                    // 解锁
                    lock.unlock();
                    // 唤醒等待的线程
                    m_condition.notify_all();
                }
            }
        }
    }
}

// 拉取任务并执行
void WorkerThread::taskExecution() {
    while (!m_stopRequested) {
        // 加锁获取任务
        //std::cout << "get task " << std::endl;
        std::unique_lock<std::mutex> lock(m_mutex);
        // 如果任务队列为空，则等待新任务到来
        m_condition.wait(lock, [this] { return !m_taskQueue.empty(); });
        //std::cout << "wake up " << std::endl;
        // 任务队列不为空时取任务
        if(!m_taskQueue.empty())
        {
            Task* task = m_taskQueue.front();
            m_taskQueue.pop();
            // 解锁
            lock.unlock();
            // 执行任务
            task->execute();
            delete task;
        }
        else {
            // 解锁
            lock.unlock();
        }
    }
}


//移除套接字
void WorkerThread::removeClientrSocket(int clientSocket)
{
    // 加锁保护共享资源
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find(m_clientSockets.begin(), m_clientSockets.end(), clientSocket);
    if (it != m_clientSockets.end()) {
        // 找到套接字，从列表中移除
        m_clientSockets.erase(it);
        socketrStatusMap.erase(clientSocket); 
    }
}
void WorkerThread::removeClientwSocket(int clientSocket)
{
    // 加锁保护共享资源
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find(m_clientSockets.begin(), m_clientSockets.end(), clientSocket);
    if (it != m_clientSockets.end()) {
        // 找到套接字，从列表中移除
        m_clientSockets.erase(it);
        socketwStatusMap.erase(clientSocket);
    }
}


void WorkerThread::start()
{
    // 启动线程并执行 checkClientSockets 函数
    m_thread = std::thread(&WorkerThread::checkClientSockets, this);
    //创建线程池
    createThreadPool();
}


void WorkerThread::set_socket_rstatus(const int client_socket, int status)
{
    //对共享资源操作加锁
    std::lock_guard<std::mutex> lock(m_mutex);
    socketrStatusMap[client_socket] = status;
}

void WorkerThread::set_socket_wstatus(const int client_socket, int status)
{
    //对共享资源操作加锁
    std::lock_guard<std::mutex> lock(m_mutex);
    socketwStatusMap[client_socket] = status;
}