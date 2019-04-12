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

/*
 * NET
 * Разработать клиент-серверное приложение копирования файла (или поддерева файловой системы) с узла-клиента
 * на узел-сервер в указанный каталог (аналог стандартной UNIX-команды rcp).
 * Команда, выполняемая на стороне клиента, имеет следующий вид: cprem path.to.src.file host@path.to.dst.dir .
 */

/*
 *
 */
static void handle_session(int session_fd) {
    char dst_dir[DST_DIR_SIZE];
    int read_count = read(session_fd, dst_dir, DST_DIR_SIZE);
    die_if(read_count < 0, "failed to read from socket %s", strerror(errno));
    dst_dir[read_count] = '\0';
    printf("Recieved destionation directory %s, len %d\n", dst_dir, read_count);

    int chdir_res = chdir(dst_dir);
    die_if(chdir_res == -1, "failed to chdir %s", strerror(errno));

    close(0);
    int dup_res = dup(session_fd);
    die_if(dup_res == -1, "failed to dup %s", strerror(errno));
    char *argv[2] = {"tar", "-xzv"};
    execvp(argv[0], argv);
    die_if(dup_res == -1, "failed to exec tar %s", strerror(errno));
}

int main(int argc, char **argv) {
    // start listening
    struct addrinfo *addr = resolve_addrinfo(argc > 1 ? argv[1] : "0", argc > 2 ? argv[2] : DEFAULT_PORT);
    int server_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    die_if(server_fd == -1, "failed to create socket %s", strerror(errno));
    int reuseaddr = 1;
    int setsockopt_res = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
    die_if(setsockopt_res != 0, "failed to set reuseaddr %s", strerror(errno));
    int bind_res = bind(server_fd, addr->ai_addr, addr->ai_addrlen);
    die_if(bind_res != 0, "failed to bind socket %s", strerror(errno));
    int listen_res = listen(server_fd, SOMAXCONN);
    die_if(listen_res != 0, "failed to listen for connections %s", strerror(errno));
    freeaddrinfo(addr);

    // accept incoming requests
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
