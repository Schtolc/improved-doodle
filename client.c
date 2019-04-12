#include <libgen.h>
#include <dirent.h>
#include "assert.h"
#include "errno.h"
#include "netdb.h"
#include "netinet/in.h"
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/ioctl.h"
#include "sys/socket.h"
#include "sys/stat.h"
#include "time.h"
#include "unistd.h"

#include "helpers.h"


/*
 * NET
 * Разработать клиент-серверное приложение копирования файла (или поддерева файловой системы) с узла-клиента
 * на узел-сервер в указанный каталог (аналог стандартной UNIX-команды rcp).
 * Команда, выполняемая на стороне клиента, имеет следующий вид: cprem path.to.src.file host@path.to.dst.dir .
 */

int main(int argc, char **argv) {
    die_if(argc != 3, "incorrect usage: waiting for cprem path.to.src.file host[:port]@path.to.dst.dir");

    // host@path.to.dst.dir -> host, path.to.dst.dir
    char *token = NULL, *sep_ptr = NULL, *host_and_dst_path = NULL;
    host_and_dst_path = sep_ptr = strdup(argv[2]);
    token = strsep(&sep_ptr, "@");
    die_if(token == NULL, "invalid destination argument, should be host[:port]@path.to.dst.dir");
    char *hostname_and_port = strdup(token);
    token = strsep(&sep_ptr, "@");
    die_if(token == NULL, "invalid destination argument, should be host[:port]@path.to.dst.dir");
    char dst_path[DST_DIR_SIZE];
    strncpy(dst_path, token, DST_DIR_SIZE);

    // hostname:port -> hostname, port
    sep_ptr = hostname_and_port;
    token = strsep(&sep_ptr, ":");
    die_if(token == NULL, "invalid destination argument, should be host[:port]@path.to.dst.dir");
    char *hostname = strdup(hostname_and_port);
    token = strsep(&sep_ptr, ":");
    char *port = token == NULL ? NULL : strdup(token);

    // /path/to/src/file -> /path/to/src, file
    char *file_path_cpy = strdup(argv[1]);
    char *file_path_cpy2 = strdup(argv[1]);
    char *file_name = basename(file_path_cpy);
    char *dir_name = dirname(file_path_cpy2);

    // connect to server
    struct addrinfo *addr = resolve_addrinfo(hostname, port == NULL ? DEFAULT_PORT : port);
    int client_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    die_if(client_fd < 0, "failed to create socket %s", strerror(errno));
    int connect_res = connect(client_fd, addr->ai_addr, addr->ai_addrlen);
    die_if(connect_res < 0, "failed to connect to server socket %s", strerror(errno));

    // send write request
    int n = write(client_fd, dst_path, strlen(dst_path));
    die_if(n < 0, "failed to write to socket %s", strerror(errno));

    int chdir_res = chdir(dir_name);
    die_if(chdir_res == -1, "failed to chdir %s", strerror(errno));

    close(1);
    int dup_res = dup(client_fd);
    die_if(dup_res == -1, "failed to dup %s", strerror(errno));

    freeaddrinfo(addr);
    free(host_and_dst_path);
    free(hostname_and_port);
    free(hostname);
    free(port);
    free(file_path_cpy);
    free(file_path_cpy2);

    char *tar_argv[] = {"tar", "-czv", file_name};
    execvp(tar_argv[0], tar_argv);
    die_if(dup_res == -1, "failed to exec tar %s", strerror(errno));

    return 0;
}
