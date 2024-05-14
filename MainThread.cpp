#include "MainThread.hpp"

MainThread::MainThread() : m_listenSocket(-1), m_maxFd(-1) {
    FD_ZERO(&m_masterSet);
    //创建一些工作线程对象
    for (int i = 0; i < 10; i++)
    {
        m_workerThread_arr.push_back(new WorkerThread(i));
    }
}

MainThread::~MainThread() {
    if (m_listenSocket != -1) {
        closesocket(m_listenSocket);
    }

    //析构子线程对象
    for (auto t : m_workerThread_arr) 
    {
        delete t;
    }
}

void MainThread::run() {
    // 创建服务端socket
   /* if ((m_listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }*/
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "Failed to initialize Winsock: " << result << std::endl;
        return;
    }

    if ((m_listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        // 使用 WSAGetLastError() 获取错误代码
        int errorCode = WSAGetLastError();
        // 使用 FormatMessageW() 获取可读的错误消息，注意函数名后面的 'W'
        wchar_t errorMsg[256]; // 声明为 wchar_t 类型的数组
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, 0, errorMsg, 256, NULL);
        // 将宽字符转换为多字节字符以输出
        char narrowErrorMsg[256];
        WideCharToMultiByte(CP_UTF8, 0, errorMsg, -1, narrowErrorMsg, sizeof(narrowErrorMsg), NULL, NULL);
        std::cerr << "Failed to create socket: " << narrowErrorMsg << std::endl;
        return;
    }

    // 绑定地址和端口
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9527); // 使用端口9527
    if (bind(m_listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        int errorCode = WSAGetLastError();
        std::cerr << "Failed to bind socket. Error code: " << errorCode << std::endl;
        closesocket(m_listenSocket);
        return;
    }

    // 监听端口
    if (listen(m_listenSocket, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        closesocket(m_listenSocket);
        return;
    }

    std::cout << "Server started, listening on port 9527" << std::endl;

    // 将服务器socket添加到主文件描述符集合中
    FD_SET(m_listenSocket, &m_masterSet);
    m_maxFd = m_listenSocket;

    for (int i = 0; i < 10; i++)
    {
        
        m_workerThread_arr[i]->start();
        std::cout << "WorkerThread "<< i <<"th starting" << std::endl;
    }


    // 主线程的事件循环，使用select检测新的客户端连接
    while (true) {
        // 复制主文件描述符集合，因为select会修改它
        fd_set readSet = m_masterSet;

        // 使用 select 检测文件描述符
        if (select(m_maxFd + 1, &readSet, nullptr, nullptr, nullptr) == -1) {
            std::cerr << "Error in select" << std::endl;
            continue;
        }


        // 检查服务器socket是否有新的客户端连接
        if (FD_ISSET(m_listenSocket, &readSet)) {
            // 接受新的客户端连接
            int clientSocket = accept(m_listenSocket, nullptr, nullptr);
            if (clientSocket == -1) {
                std::cerr << "Failed to accept client connection" << std::endl;
            }
            else {
                //std::cout << "New client connected, socket: " << clientSocket << std::endl;
                // 将新连接的客户端socket文件描述符放入客户端最少的那个子线程
                int client_num = INT_MAX,min_idx = -1;
                for (int i = 0; i < 10; i++)
                {
                    INT TMP = m_workerThread_arr[i]->getManagedClientCount();
                    if(TMP < client_num)
                    {
                        client_num = TMP;
                        min_idx = i;
                    }
                }
                //std::cout << " new conect add "<< clientSocket <<"to m_workerThread_arr:" << min_idx << std::endl;
                m_workerThread_arr[min_idx]->addClientSocket(clientSocket);
            }
        }

    }
    WSACleanup();

    // 关闭服务端socket
    closesocket(m_listenSocket);
}
