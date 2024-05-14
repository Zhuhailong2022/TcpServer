#include "MainThread.hpp"

MainThread::MainThread() : m_listenSocket(-1), m_maxFd(-1) {
    FD_ZERO(&m_masterSet);
    //����һЩ�����̶߳���
    for (int i = 0; i < 10; i++)
    {
        m_workerThread_arr.push_back(new WorkerThread(i));
    }
}

MainThread::~MainThread() {
    if (m_listenSocket != -1) {
        closesocket(m_listenSocket);
    }

    //�������̶߳���
    for (auto t : m_workerThread_arr) 
    {
        delete t;
    }
}

void MainThread::run() {
    // ���������socket
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
        // ʹ�� WSAGetLastError() ��ȡ�������
        int errorCode = WSAGetLastError();
        // ʹ�� FormatMessageW() ��ȡ�ɶ��Ĵ�����Ϣ��ע�⺯��������� 'W'
        wchar_t errorMsg[256]; // ����Ϊ wchar_t ���͵�����
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, 0, errorMsg, 256, NULL);
        // �����ַ�ת��Ϊ���ֽ��ַ������
        char narrowErrorMsg[256];
        WideCharToMultiByte(CP_UTF8, 0, errorMsg, -1, narrowErrorMsg, sizeof(narrowErrorMsg), NULL, NULL);
        std::cerr << "Failed to create socket: " << narrowErrorMsg << std::endl;
        return;
    }

    // �󶨵�ַ�Ͷ˿�
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9527); // ʹ�ö˿�9527
    if (bind(m_listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        int errorCode = WSAGetLastError();
        std::cerr << "Failed to bind socket. Error code: " << errorCode << std::endl;
        closesocket(m_listenSocket);
        return;
    }

    // �����˿�
    if (listen(m_listenSocket, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        closesocket(m_listenSocket);
        return;
    }

    std::cout << "Server started, listening on port 9527" << std::endl;

    // ��������socket��ӵ����ļ�������������
    FD_SET(m_listenSocket, &m_masterSet);
    m_maxFd = m_listenSocket;

    for (int i = 0; i < 10; i++)
    {
        
        m_workerThread_arr[i]->start();
        std::cout << "WorkerThread "<< i <<"th starting" << std::endl;
    }


    // ���̵߳��¼�ѭ����ʹ��select����µĿͻ�������
    while (true) {
        // �������ļ����������ϣ���Ϊselect���޸���
        fd_set readSet = m_masterSet;

        // ʹ�� select ����ļ�������
        if (select(m_maxFd + 1, &readSet, nullptr, nullptr, nullptr) == -1) {
            std::cerr << "Error in select" << std::endl;
            continue;
        }


        // ��������socket�Ƿ����µĿͻ�������
        if (FD_ISSET(m_listenSocket, &readSet)) {
            // �����µĿͻ�������
            int clientSocket = accept(m_listenSocket, nullptr, nullptr);
            if (clientSocket == -1) {
                std::cerr << "Failed to accept client connection" << std::endl;
            }
            else {
                //std::cout << "New client connected, socket: " << clientSocket << std::endl;
                // �������ӵĿͻ���socket�ļ�����������ͻ������ٵ��Ǹ����߳�
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

    // �رշ����socket
    closesocket(m_listenSocket);
}
