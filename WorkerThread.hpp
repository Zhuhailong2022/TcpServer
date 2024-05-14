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
    // �����߳���ӿͻ����׽���
    void addClientSocket(int clientSocket);
    // ��ѯ��ǰ����Ŀͻ����׽�������
    int getManagedClientCount();
    //�Ƴ��׽���
    void removeClientrSocket(int clientSocket);
    void removeClientwSocket(int clientSocket);
    // �����̳߳�
    void createThreadPool();

    // ֹͣ�̳߳�
    void stopThreadPool();

    // ����ִ�к���
    void taskExecution();

    // ʹ��select���ͻ����׽����Ƿ��ж�д�¼�
    void checkClientSockets();

    //����WorkerThread
    void start();
    //�����׽���״̬��ӳ��
    void set_socket_rstatus(const int client_socket, int status);
    void set_socket_wstatus(const int client_socket, int status);
private:
    std::vector<int> m_clientSockets;//�̹߳�����ļ�����������
    std::map<int, int> socketrStatusMap;// �׽���״̬ӳ�䣬��Ϊ�׽����ļ���������ֵΪ�׽���״̬��0Ϊ���У�1Ϊæµ��
    std::map<int, int> socketwStatusMap;
    int m_maxFd; // ����ļ�������
    std::thread m_thread;//ר�ŵ��߳��������м�����ͻ��˵Ķ�д�¼�����
    bool m_exitFlag = false; // ���ڿ��Ƽ�����ͻ��˵Ķ�д�¼������˳�������
    fd_set m_readSet; // ���¼�����
    std::mutex m_mutex; // ������
    std::queue<Task*> m_taskQueue; // �������
    std::condition_variable m_condition; // ���������������߳�ͬ��
    std::vector<std::thread> m_threadPool; // �̳߳�
    int id;//��Ǳ� workerthread�ǵڼ���
    std::atomic<bool> m_stopRequested{ false }; // �̳߳�ֹͣ��־
};

#endif // WORKER_THREAD_HPP
