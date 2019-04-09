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
        die_if(__LINE__, "failed to read from socket %s", strerror(errno));
    }
    return request;
}

static void process_request(const char *request) {
    WriteRequest *write_request = deserialize_write_request(request);
    printf("Recieved write request. [dst_dir: %s][data_len: %d]\n",
           write_request->dst_dir,
           write_request->data_length);

    int chdir_res = chdir(write_request->dst_dir);
    die_if(chdir_res == -1, "failed to chdir %s", strerror(errno));

    int fd = open("my.tar.gz", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        die_if(__LINE__, "failed to open file %s: %s", write_request->dst_dir, strerror(errno));
    }
    int write_res = write(fd, write_request->data, write_request->data_length);
    if (write_res < 0) {
        die_if(__LINE__, "failed to write to file %s", strerror(errno));
    }
    close(fd);

    int system_res = system("tar -xzvf my.tar.gz");
    die_if(system_res == -1, "failed to untar");
    int remove_res = remove("my.tar.gz");
    die_if(remove_res != 0, "failed to remove tar");

    free_write_request(write_request);
}

static void write_ok(int session_fd) {
    const char *ok = "ok";
    int write_count = write(session_fd, ok, strlen(ok));
    if (write_count < 0) {
        die_if(__LINE__, "failed to write to socket %s", strerror(errno));
    }
}

static void handle_session(int session_fd) {
    char *request = read_request(session_fd);
    process_request(request);
    write_ok(session_fd);

    free(request);
}

int main(int argc, char **argv) {
    struct addrinfo *addr = resolve_addrinfo(argc > 1 ? argv[1] : "0", argc > 2 ? argv[2] : DEFAULT_PORT);
    int server_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    die_if(server_fd == -1, "failed to create socket %s", strerror(errno));

    int reuseaddr = 1;
    int setsockopt_res = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
    die_if(setsockopt_res != 0, "failed to set reuseaddr %s", strerror(errno));

    int bind_res = bind(server_fd, addr->ai_addr, addr->ai_addrlen);
    die_if(bind_res != 0, "failed to bind socket %s", strerror(errno));

    freeaddrinfo(addr);

    int listen_res = listen(server_fd, SOMAXCONN);
    die_if(listen_res != 0, "failed to listen for connections %s", strerror(errno));

    for (;;) {
        int session_fd = accept(server_fd, 0, 0);
        die_if(session_fd == -1, "failed to accept connection %s", strerror(errno));

        pid_t pid = fork();
        die_if(pid == -1, "failed to create child process %s", strerror(errno));
        if (pid == 0) {
            close(server_fd);
            handle_session(session_fd);
            close(session_fd);
            exit(0);
        } else {
            close(session_fd);
        }
    }
}
