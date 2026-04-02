#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>

#define PORT 8080
#define SIZE 1024

void error(char *msg)
{
    perror(msg);
    exit(1);
}

char *to_lower(char *str)
{
    for (int i = 0; str[i]; i++)
        str[i] = tolower(str[i]);
    return str;
}

char *get_mime_type(char *path)
{
    char *type = malloc(100);
    strcpy(type,"Content-Type: ");
    char *ext = strrchr(path, '.');
    if (!ext)
    {
        strcat(type, "application/octet-stream");
        return type;
    }
    ext++;
    char tmp[20];
    strncpy(tmp, ext, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = '\0';
    to_lower(tmp);
    if (strcmp(tmp, "html") == 0)
        strcat(type, "text/html");
    else if (strcmp(tmp, "css") == 0)
        strcat(type, "text/css");
    else if (strcmp(tmp, "js") == 0)
        strcat(type, "application/javascript");
    else if (strcmp(tmp, "png") == 0)
        strcat(type, "image/png");
    else if (strcmp(tmp, "jpg") == 0 || strcmp(tmp, "jpeg") == 0)
        strcat(type, "image/jpeg");
    else if (strcmp(tmp, "gif") == 0)
        strcat(type, "image/gif");
    else
        strcat(type, "application/octet-stream");
    return type;
}

int main()
{
    int sockfd, clientfd;
    char buffer[SIZE];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Socket creation failed");
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&server, sizeof(server)) < 0)
        error("Bind failed");

    if (listen(sockfd, 5) < 0)
        error("Listen failed");

    printf("Server running on port %d...\n", PORT);

    while (1)
    {
        clientfd = accept(sockfd, (struct sockaddr*)&client, &len);
        if (clientfd < 0)
            continue;

        memset(buffer, 0, SIZE);
        int bytes = recv(clientfd, buffer, SIZE - 1, 0);
        if (bytes <= 0)
        {
            close(clientfd);
            continue;
        }
        buffer[bytes] = '\0';
        char *method = strtok(buffer, " ");
        char *path = strtok(NULL, " ");
        if (!path)
        {
            close(clientfd);
            continue;
        }
        if (strstr(path, ".."))
        {
            char *resp = "HTTP/1.1 403 Forbidden\r\n\r\n";
            send(clientfd, resp, strlen(resp), 0);
            close(clientfd);
            continue;
        }
        if (strcmp(path, "/") == 0)
            path = "/index.html";
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s", path + 1);

        FILE *fp = fopen(filepath, "rb");
        if (!fp)
        {
            char *resp =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 22\r\n"
                "\r\n"
                "<h1>404 Not Found</h1>";

            send(clientfd, resp, strlen(resp), 0);
            close(clientfd);
            continue;
        }
        fseek(fp, 0, SEEK_END);
        long filesize = ftell(fp);
        rewind(fp);

        char *mime = get_mime_type(filepath);
        char header[256];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "%s\r\n"
                 "Content-Length: %ld\r\n"
                 "\r\n",
                 mime, filesize);

        send(clientfd, header, strlen(header), 0);

        while (1)
        {
            int n = fread(buffer, 1, SIZE, fp);
            if (n <= 0)
                break;
            send(clientfd, buffer, n, 0);
        }

        fclose(fp);
        free(mime);
        close(clientfd);
    }
    close(sockfd);
    return 0;
}
