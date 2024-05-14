#include "Task.hpp"
#include "WorkerThread.hpp" // 包含 WorkerThread 类的头文件
void Task::execute() {
    if (TYPE == READ) {        
        // 循环内部接收数据
        while (true) {
            int bytesRead = recv(m_clientSocket, buffer + totalBytesReceived, MAX_BUFFER_SIZE - totalBytesReceived, 0);
            //std::cout << "bytesRead  " <<  bytesRead << std::endl;
            if (bytesRead <= 0) {
                // 读取失败或连接关闭，退出循环
                if (bytesRead == 0) {
                    //std::cout << m_clientSocket <<" Client disconnected" << std::endl;
                    m_workerThread->set_socket_rstatus(m_clientSocket, 0);//设置其为空闲状态 4.11
                }
                else {
                    std::cerr << "Failed to receive data from client:" << m_clientSocket << std::endl;
                }
                // 读取失败从管理列表移除 
                closesocket(m_clientSocket);
                m_workerThread->removeClientrSocket(m_clientSocket);
                break;
            }

            totalBytesReceived += bytesRead;

            // 如果缓冲区已满，则处理接收到的数据
            if (totalBytesReceived >= MAX_BUFFER_SIZE) {
                processData(buffer, totalBytesReceived); // 处理已接收到的数据
                totalBytesReceived = 0; // 重置 totalBytesReceived
            }
        }

        // 处理缓冲区中的剩余数据
        if (totalBytesReceived > 0) {
            processData(buffer, totalBytesReceived);
        }
    }
    if (TYPE == WRITE) {
        // 写数据
        const char* message = "Hello, client!";
        int bytesWritten = send(m_clientSocket, message, strlen(message), 0);
        //std::cout << " write " << std::endl;
        if (bytesWritten == -1) {
            // 写入失败，关闭套接字并从管理列表中移除
            closesocket(m_clientSocket);
            // 这里需要确保在 m_clientSockets 中查找并移除该套接字    
            m_workerThread->removeClientwSocket(m_clientSocket);
            return;
        }
        // 在 m_clientSockets 中移除该套接字
        m_workerThread->set_socket_wstatus(m_clientSocket, 0);//设置该套接字的读事件状态为空闲状态 4.11
        //m_workerThread->removeClientwSocket(m_clientSocket);
    }
}
void Task::processData(const char* data, int len)
{
    std::cout << std::string(data, len) << "from: " << m_clientSocket << std::endl;
}
