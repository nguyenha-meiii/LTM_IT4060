#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

int main()
{

    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    char buf[1024];

    for (int i = 0; i < 8; i++)
    {
        if (fork() == 0)
        {
            while (1)
            {
                // chờ kết nối từ client
                int client = accept(listener, NULL, NULL);
                printf("Client connected: %d\n", client);

                // chờ dữ liệu từ client
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    close(client);
                    continue;
                }

                // xử lý dữ liệu trả lại kết quả cho client
                buf[ret] = 0;
                printf("%s\n", buf);

                char msg[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Nguyen Thu Ha - 20210299</h1></body></html>";
                send(client, msg, strlen(msg), 0);

                // đóng kết nối
                close(client);
            }

            exit(0);
        }
    }

    getchar();
    killpg(0, SIGKILL);

    return 0;
}