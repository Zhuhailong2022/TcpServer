#include <iostream>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define SER_PORT 9527

void muti_thread_run(int i) {
    struct sockaddr_in serc_addr;
    serc_addr.sin_family = AF_INET;
    serc_addr.sin_port = htons(SER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serc_addr.sin_addr.s_addr);

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return;
    }

    SOCKET cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == INVALID_SOCKET) {
        std::cerr << "socket error" << std::endl;
        WSACleanup();
        return;
    }

    int ret = connect(cfd, reinterpret_cast<struct sockaddr*>(&serc_addr), sizeof(serc_addr));
    if (ret == SOCKET_ERROR) {
        std::cerr << "connect error" << std::endl;
        closesocket(cfd);
        WSACleanup();
        return;
    }

    int counter = 10;
    while (counter--) {
        char buf[BUFSIZ] = {};
        snprintf(buf, BUFSIZ, "thread: %d hello socket\n", i);
        int bytes_written = send(cfd, buf, strlen(buf), 0);
        if (bytes_written == SOCKET_ERROR) {
            std::cerr << "send failed" << std::endl;
            break;
        }

        ret = recv(cfd, buf, sizeof(buf), 0);
        if (ret == SOCKET_ERROR || ret == 0) {
            std::cerr << "recv failed" << std::endl;
            break;
        }

        buf[ret] = '\0';
        std::cout << buf;
    }

    closesocket(cfd);
    WSACleanup();
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    for (int i = 0; i < 40; i++) {
        std::thread t(muti_thread_run, i);
        t.join();
    }

    WSACleanup();
    std::cout << "main exit" << std::endl;
    return 0;
}
