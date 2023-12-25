#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
int main()
{
    int sockfd[2];

    // 创建socket对
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == -1)
    {
        perror("socketpair");
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return 1;
    }
    else if (pid > 0)
    {
        close(sockfd[1]);

        const char *message_to_child = "Hello Child Process!";
        if (send(sockfd[0], message_to_child, strlen(message_to_child), 0) == -1)
        {
            perror("send");
            return 1;
        }

        char buffer[1024];
        int nbytes = recv(sockfd[0], buffer, sizeof buffer, 0);
        if (nbytes == -1)
        {
            perror("recv");
            return 1;
        }
        buffer[nbytes] = '\0';
        std::cout << "Parent Process recv:" << buffer << std::endl;
        close(sockfd[0]);
    }
    else
    {
        close(sockfd[0]); // 关闭子进程中不需要的连接
        char buffer[1024];
        int nbytes = recv(sockfd[1], buffer, sizeof(buffer), 0);
        if (nbytes == -1)
        {
            perror("recv");
            return 1;
        }
        buffer[nbytes] = '\0';
        std::cout << "Child Process received: " << buffer << std::endl;

        const char *message_to_parent = "Hello Parent Process!";
        if (send(sockfd[1], message_to_parent, strlen(message_to_parent), 0) == -1)
        {
            perror("send");
            return 1;
        }

        close(sockfd[1]); // 关闭子进程中不需要的连接
    }
    return 0;
}