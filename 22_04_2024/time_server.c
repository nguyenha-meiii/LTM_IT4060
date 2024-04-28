#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>

void signalHandler(int signo)
{
    int pid = wait(NULL);
    printf("Child process done, pid = %d\n", pid);
}

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

    // Đăng ký hàm xử lý tín hiệu
    signal(SIGCHLD, signalHandler);

    while (1)
    {
        printf("Waiting for incoming connection...\n");
        int client = accept(listener, NULL, NULL);
        printf("New client connected: %d\n", client);

        // Lay thoi gian hien tai
        time_t rawtime;
        struct tm *timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);

        // Set to Vietnam (UTC + 7)
        timeinfo->tm_hour += 7;
        if (timeinfo->tm_hour >= 24)
        {
            timeinfo->tm_hour -= 24;
            timeinfo->tm_mday += 1;
        }

        if (fork() == 0)
        {
            // Tiến trình con, nhận dữ liệu từ client và trả về thời gian
            // Đóng socket lắng nghe của tiến trình cha trong tiến trình con
            close(listener);

            while (1)
            {
                char buf[256];
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    break;
                }
                buf[ret] = 0;
                printf("Received from client: %s\n", buf);

                char cmd[16], format[16], tmp[16];
                int n = sscanf(buf, "%s %s %s", cmd, format, tmp);
                if (n == 2 && strcmp(cmd, "GET_TIME") == 0)
                {
                    if (strcmp(format, "dd/mm/yyyy") == 0)
                    {
                        strftime(buf, sizeof(buf), "%d/%m/%Y", timeinfo);
                    }
                    else if (strcmp(format, "dd/mm/yy") == 0)
                    {
                        strftime(buf, sizeof(buf), "%d/%m/%y", timeinfo);
                    }
                    else if (strcmp(format, "mm/dd/yyyy") == 0)
                    {
                        strftime(buf, sizeof(buf), "%m/%d/%Y", timeinfo);
                    }
                    else if (strcmp(format, "mm/dd/yy") == 0)
                    {
                        strftime(buf, sizeof(buf), "%m/%d/%y", timeinfo);
                    }
                    else if (strcmp(format, "YYYY-MM-DD") == 0)
                    {
                        strftime(buf, sizeof(buf), "%Y-%m-%d", timeinfo);
                    }
                    else
                    {
                        strcpy(buf, "Invalid format");
                    }
                }
                else
                {
                    strcpy(buf, "Invalid command");
                }

                buf[strlen(buf)] = '\0';
                // Gui thoi gian hien tai ve client
                send(client, buf, strlen(buf), 0);
            }
            // Dong socket ket noi o tien trinh con
            exit(0);
        }

        // Dong socket ket noi o tien trinh cha
        close(client);
    }

    return 0;
}