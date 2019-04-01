#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "helpers.h"


/*
 * NET
 * Разработать клиент-серверное приложение копирования файла (или поддерева файловой системы) с узла-клиента
 * на узел-сервер в указанный каталог (аналог стандартной UNIX-команды rcp).
 * Команда, выполняемая на стороне клиента, имеет следующий вид: cprem path.to.src.file host@path.to.dst.dir .
 */

char *read_request(int session_fd) {
    static const int REQUEST_BUFFER_SIZE = 80;
    char *request = (char *) malloc(REQUEST_BUFFER_SIZE);

    int read_count = read(session_fd, request, REQUEST_BUFFER_SIZE - 1 /* leaving space for '/0' */);
    if (read_count < 0) {
        die(__LINE__, "failed to read from socket (errno=%d)", errno);
    }

    request[read_count] = '\0';
    return request;
}

char *process_request(const char *request) {
    static const int RESPONSE_BUFFER_SIZE = 80;

    char *response = (char *) malloc(RESPONSE_BUFFER_SIZE);
    strncpy(response, request, RESPONSE_BUFFER_SIZE); // echo
    return response;
}

void write_response(int session_fd, const char *response) {
    int write_count = write(session_fd, response, strlen(response));
    if (write_count < 0) {
        die(__LINE__, "failed to write to socket (errno=%d)", errno);
    }
}

void handle_session(int session_fd) {
    char *request = read_request(session_fd);
    char *response = process_request(request);
    write_response(session_fd, response);

    free(request);
    free(response);
}

int create_socket(const char *hostname, const char *portname) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

    struct addrinfo *addr = 0;
    int err = getaddrinfo(hostname, portname, &hints, &addr);
    if (err != 0) {
        die(__LINE__, "failed to resolve local socket address (err=%d)", err);
    }

    int server_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (server_fd == -1) {
        die(__LINE__, "%s", strerror(errno));
    }
    int reuseaddr = 1;
    int setsockopt_res = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
    if (setsockopt_res != 0) {
        die(__LINE__, "%s", strerror(errno));
    }

    int bind_res = bind(server_fd, addr->ai_addr, addr->ai_addrlen);
    if (bind_res != 0) {
        die(__LINE__, "%s", strerror(errno));
    }

    freeaddrinfo(addr);

    return server_fd;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        die(__LINE__, "incorrect usage: waiting for ./rcp_server port_number");
    }

    int server_fd = create_socket(argv[1], argv[2]);
    int listen_res = listen(server_fd, SOMAXCONN);
    if (listen_res != 0) {
        die(__LINE__, "failed to listen for connections (errno=%d)", errno);
    }

    for (;;) {
        int session_fd = accept(server_fd, 0, 0);
        if (session_fd == -1) {
            if (errno == EINTR)
                continue;
            die(__LINE__, "failed to accept connection (errno=%d)", errno);
        }
        pid_t pid = fork();
        if (pid == -1) {
            die(__LINE__, "failed to create child process (errno=%d)", errno);
        } else if (pid == 0) {
            close(server_fd);
            handle_session(session_fd);
            close(session_fd);
            _exit(0);
        } else {
            close(session_fd);
        }
    }
}
