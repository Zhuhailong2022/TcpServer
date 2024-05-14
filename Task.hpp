#ifndef TASK_HPP
#define TASK_HPP
#define READ 1
#define WRITE 2
#include <WinSock2.h>
#include <iostream>

//#include "WorkerThread.hpp" // ���� WorkerThread ���ͷ�ļ�
class WorkerThread;
class Task {
public:
    Task(int clientSocket,int type,WorkerThread* ptr) : m_clientSocket(clientSocket), TYPE(type), m_workerThread(ptr){}

    // ִ������
    void execute();
    //���ݴ�����
    void processData(const char* data, int len);

private:
    int m_clientSocket; // �ͻ����׽����ļ�������
    int TYPE = 0;
    WorkerThread* m_workerThread; // ָ�� WorkerThread �����ָ�룬Ҳ�������Ϊ��Ǹ�������������ĸ����̴߳����ģ�����ִ����ɾfd�ļ�������
                                   
    //����������д������
    static const int MAX_BUFFER_SIZE = 4096;
    char buffer[MAX_BUFFER_SIZE];
    int totalBytesReceived = 0;
};

#endif // TASK_HPP
