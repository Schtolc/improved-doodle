#include "errno.h"
#include "fcntl.h"
#include "netdb.h"
#include "netinet/in.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/socket.h"
#include "sys/stat.h"
#include "unistd.h"

#include "helpers.h"
#include "write_request.h"

/*
 * NET
 * Разработать клиент-серверное приложение копирования файла (или поддерева файловой системы) с узла-клиента
 * на узел-сервер в указанный каталог (аналог стандартной UNIX-команды rcp).
 * Команда, выполняемая на стороне клиента, имеет следующий вид: cprem path.to.src.file host@path.to.dst.dir .
 */

static char *read_request(int session_fd) {
    int request_buffer_size = write_request_max_size();

    char *request = (char *) calloc(request_buffer_size, sizeof(char));
    int read_count = read(session_fd, request, request_buffer_size);
    if (read_count < 0) {
        die(__LINE__, "failed to read from socket %s", strerror(errno));
    }
    return request;
}

static void process_request(const char *request) {
    WriteRequest *write_request = deserialize_write_request(request);
    printf("Recieved write request. [dst_path: %s][is_folder: %d][data_len: %d]\n",
           write_request->dst_path,
           write_request->is_folder,
           write_request->data_length);

    if (write_request->is_folder) {
        int mkdir_res = mkdir(write_request->dst_path, 0777);
        if (mkdir_res != 0) {
            die(__LINE__, "failed to create folder %s: %s", write_request->dst_path, strerror(errno));
        }
    } else {
        int fd = open(write_request->dst_path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            die(__LINE__, "failed to open file %s: %s", write_request->dst_path, strerror(errno));
        }
        int write_res = write(fd, write_request->data, write_request->data_length);
        if (write_res < 0) {
            die(__LINE__, "failed to write to file %s", strerror(errno));
        }
        close(fd);
    }

    free_write_request(write_request);
}

static void write_ok(int session_fd) {
    const char *ok = "ok";
    int write_count = write(session_fd, ok, strlen(ok));
    if (write_count < 0) {
        die(__LINE__, "failed to write to socket %s", strerror(errno));
    }
}

static void handle_session(int session_fd) {
    char *request = read_request(session_fd);
    process_request(request);
    write_ok(session_fd);

    free(request);
}

static int create_socket(const char *hostname, const char *portname) {
    struct addrinfo *addr = get_addrinfo_simple(hostname, portname);
    int server_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (server_fd == -1) {
        die(__LINE__, "failed to create socket %s", strerror(errno));
    }
    int reuseaddr = 1;
    int setsockopt_res = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
    if (setsockopt_res != 0) {
        die(__LINE__, "failed to set reuseaddr %s", strerror(errno));
    }

    int bind_res = bind(server_fd, addr->ai_addr, addr->ai_addrlen);
    if (bind_res != 0) {
        die(__LINE__, "failed to bind socket %s", strerror(errno));
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
        die(__LINE__, "failed to listen for connections %s", strerror(errno));
    }

    for (;;) {
        int session_fd = accept(server_fd, 0, 0);
        if (session_fd == -1) {
            if (errno == EINTR)
                continue;
            die(__LINE__, "failed to accept connection %s", strerror(errno));
        }
        pid_t pid = fork();
        if (pid == -1) {
            die(__LINE__, "failed to create child process %s", strerror(errno));
        } else if (pid == 0) {
            close(server_fd);
            handle_session(session_fd);
            close(session_fd);
            exit(0);
        } else {
            close(session_fd);
        }
    }
}
