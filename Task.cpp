#include "Task.hpp"
#include "WorkerThread.hpp" // ���� WorkerThread ���ͷ�ļ�
void Task::execute() {
    if (TYPE == READ) {        
        // ѭ���ڲ���������
        while (true) {
            int bytesRead = recv(m_clientSocket, buffer + totalBytesReceived, MAX_BUFFER_SIZE - totalBytesReceived, 0);
            //std::cout << "bytesRead  " <<  bytesRead << std::endl;
            if (bytesRead <= 0) {
                // ��ȡʧ�ܻ����ӹرգ��˳�ѭ��
                if (bytesRead == 0) {
                    //std::cout << m_clientSocket <<" Client disconnected" << std::endl;
                    m_workerThread->set_socket_rstatus(m_clientSocket, 0);//������Ϊ����״̬ 4.11
                }
                else {
                    std::cerr << "Failed to receive data from client:" << m_clientSocket << std::endl;
                }
                // ��ȡʧ�ܴӹ����б��Ƴ� 
                closesocket(m_clientSocket);
                m_workerThread->removeClientrSocket(m_clientSocket);
                break;
            }

            totalBytesReceived += bytesRead;

            // ���������������������յ�������
            if (totalBytesReceived >= MAX_BUFFER_SIZE) {
                processData(buffer, totalBytesReceived); // �����ѽ��յ�������
                totalBytesReceived = 0; // ���� totalBytesReceived
            }
        }

        // ���������е�ʣ������
        if (totalBytesReceived > 0) {
            processData(buffer, totalBytesReceived);
        }
    }
    if (TYPE == WRITE) {
        // д����
        const char* message = "Hello, client!";
        int bytesWritten = send(m_clientSocket, message, strlen(message), 0);
        //std::cout << " write " << std::endl;
        if (bytesWritten == -1) {
            // д��ʧ�ܣ��ر��׽��ֲ��ӹ����б����Ƴ�
            closesocket(m_clientSocket);
            // ������Ҫȷ���� m_clientSockets �в��Ҳ��Ƴ����׽���    
            m_workerThread->removeClientwSocket(m_clientSocket);
            return;
        }
        // �� m_clientSockets ���Ƴ����׽���
        m_workerThread->set_socket_wstatus(m_clientSocket, 0);//���ø��׽��ֵĶ��¼�״̬Ϊ����״̬ 4.11
        //m_workerThread->removeClientwSocket(m_clientSocket);
    }
}
void Task::processData(const char* data, int len)
{
    std::cout << std::string(data, len) << "from: " << m_clientSocket << std::endl;
}
