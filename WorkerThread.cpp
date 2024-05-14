#include "WorkerThread.hpp"

WorkerThread::~WorkerThread() 
{

    stopThreadPool();
    m_exitFlag = true;//�޸� ���ͻ��˺���ִ�еı�־��ֹͣ��⡣
    // �ȴ��߳�ִ����ϲ��˳�
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

//�򼯺�����������ͻ��˵��׽���
void WorkerThread::addClientSocket(int clientSocket) {
    // ��������������Դ
    std::lock_guard<std::mutex> lock(m_mutex);
    m_clientSockets.push_back(clientSocket);
    socketrStatusMap[clientSocket] = 0;
    socketwStatusMap[clientSocket] = 0;
}
//�����̼߳�����߳��й����˶��ٸ��׽��ֽӿ�
int WorkerThread::getManagedClientCount()  {
    return m_clientSockets.size();
}
// �����̳߳�
void WorkerThread::createThreadPool()
{
    // �����̳߳أ��������ʹ��4���߳�
    for (int i = 0; i < 5; ++i) {
        std::cout << "create thread" << i << std::endl;
        m_threadPool.emplace_back(&WorkerThread::taskExecution, this);
    }
}

// ֹͣ�̳߳�
void WorkerThread::stopThreadPool()
{
    // ����ֹͣ��־Ϊ true
    m_stopRequested = true;

    // �ȴ������߳̽���
    for (auto& thread : m_threadPool) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// ʹ��select���ͻ����׽����Ƿ��ж�д�¼�
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

        // �����ʵĿͻ����׽�����ӵ���д�¼���⼯����
        for (int clientSocket : m_clientSockets) {
            ////���clientSocket����ʲô״̬��ֻ������Ϊ����״̬ 0 ʱ�Ž�����ӵ���д�׽��ּ�⼯����
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

        // ���ó�ʱʱ��Ϊ0����ʾ��������
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;



        // ���� select ��������׽����Ƿ��ж��¼�
        int result = select(maxSocket + 1, &readfds, &writefds, nullptr, &timeout);
        if (result == -1) {
            // ������
            std::cerr << "Error in WorkerThread select function: " << errno << std::endl;
            return;
            //std::cerr << "Error in WorkerThread"<< id <<" select function" << std::endl;
            //return;
        }

        if (result > 0) {

            //std::cout << "result : " << result << std::endl;
            // ���׽����ж��¼�����
            for (auto it :m_clientSockets) {
                int clientSocket = it;
                //����ж��¼�
                if (FD_ISSET(clientSocket, &readfds)) {
                    // ����ͻ����׽��ֵĶ��¼�
                    Task* task =new Task(clientSocket, READ, this);
                    //���ö�ȡæ״̬
                    set_socket_rstatus(clientSocket, 1);
                    //set_socket_status(clientSocket, 1);//���׽���״̬����Ϊæµ״̬
                    //��������������������������׽���״̬
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_taskQueue.push(task);
                    std::cout << "read "<< clientSocket<<" task" << std::endl;
                    std::cout << "m_taskQueue: " << m_taskQueue.size()  << std::endl;
                    // ����
                    lock.unlock();
                    // ���ѵȴ����߳�
                    m_condition.notify_all();
                }
                //�����д
                if (FD_ISSET(clientSocket, &writefds))
                {
                    Task* task = new Task(clientSocket, WRITE, this);
                    //д��æ״̬
                    set_socket_wstatus(clientSocket, 1);
                    //���������������������
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_taskQueue.push(task);
                    std::cout << "write m_taskQueue: " << m_taskQueue.size() << std::endl;
                    std::cout << "write " << clientSocket << "task" << std::endl;
                    // ����
                    lock.unlock();
                    // ���ѵȴ����߳�
                    m_condition.notify_all();
                }
            }
        }
    }
}

// ��ȡ����ִ��
void WorkerThread::taskExecution() {
    while (!m_stopRequested) {
        // ������ȡ����
        //std::cout << "get task " << std::endl;
        std::unique_lock<std::mutex> lock(m_mutex);
        // ����������Ϊ�գ���ȴ���������
        m_condition.wait(lock, [this] { return !m_taskQueue.empty(); });
        //std::cout << "wake up " << std::endl;
        // ������в�Ϊ��ʱȡ����
        if(!m_taskQueue.empty())
        {
            Task* task = m_taskQueue.front();
            m_taskQueue.pop();
            // ����
            lock.unlock();
            // ִ������
            task->execute();
            delete task;
        }
        else {
            // ����
            lock.unlock();
        }
    }
}


//�Ƴ��׽���
void WorkerThread::removeClientrSocket(int clientSocket)
{
    // ��������������Դ
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find(m_clientSockets.begin(), m_clientSockets.end(), clientSocket);
    if (it != m_clientSockets.end()) {
        // �ҵ��׽��֣����б����Ƴ�
        m_clientSockets.erase(it);
        socketrStatusMap.erase(clientSocket); 
    }
}
void WorkerThread::removeClientwSocket(int clientSocket)
{
    // ��������������Դ
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find(m_clientSockets.begin(), m_clientSockets.end(), clientSocket);
    if (it != m_clientSockets.end()) {
        // �ҵ��׽��֣����б����Ƴ�
        m_clientSockets.erase(it);
        socketwStatusMap.erase(clientSocket);
    }
}


void WorkerThread::start()
{
    // �����̲߳�ִ�� checkClientSockets ����
    m_thread = std::thread(&WorkerThread::checkClientSockets, this);
    //�����̳߳�
    createThreadPool();
}


void WorkerThread::set_socket_rstatus(const int client_socket, int status)
{
    //�Թ�����Դ��������
    std::lock_guard<std::mutex> lock(m_mutex);
    socketrStatusMap[client_socket] = status;
}

void WorkerThread::set_socket_wstatus(const int client_socket, int status)
{
    //�Թ�����Դ��������
    std::lock_guard<std::mutex> lock(m_mutex);
    socketwStatusMap[client_socket] = status;
}