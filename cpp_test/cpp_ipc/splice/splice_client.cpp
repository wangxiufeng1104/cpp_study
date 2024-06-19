#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *hello = "Hello server";
    char buffer[1024] = {0};

    // 创建 socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket creation error");
        return -1;
    }

    // 设置服务器地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 将 IPv4 字符串转换为网络地址
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0)
    {
        perror("invalid address");
        return -1;
    }

    // 连接到服务器
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connection failed");
        return -1;
    }

    // 发送消息给服务器
    send(sock, hello, strlen(hello), 0);

    recv(sock, buffer, sizeof buffer, 0);

    printf("Message recv from server: %s\n", hello);

    close(sock);
    return 0;
}
