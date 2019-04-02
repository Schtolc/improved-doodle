#include <netdb.h>
#include <libgen.h>
#include "helpers.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "zconf.h"

void die(int line_number, const char *format, ...) {
    va_list vargs;
    va_start (vargs, format);
    fprintf(stderr, "%d: ", line_number);
    vfprintf(stderr, format, vargs);
    fprintf(stderr, ".\n");
    va_end (vargs);
    exit(1);
}

int split_host_and_dir(const char *input, char *host, char *dir) {
    int pos = 0;
    int host_pos = 0;
    for (; pos < strlen(input) && input[pos] != '@'; ++pos) {
        host[host_pos++] = input[pos];
    }
    if (host_pos == 0 || host_pos == strlen(input))
        return -1;

    host[host_pos] = '\0';

    ++pos;
    int dir_pos = 0;
    for (; pos < strlen(input); ++pos) {
        dir[dir_pos++] = input[pos];
    }
    if (dir_pos == 0)
        return -1;
    dir[dir_pos] = '\0';
    return 0;
}

struct addrinfo *get_addrinfo_simple(const char *hostname, const char *portname) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

    struct addrinfo *addr = NULL;
    int err = getaddrinfo(hostname, portname, &hints, &addr);
    if (err != 0) {
        die(__LINE__, "failed to resolve server socket address (err=%d)", err);
    }
    return addr;
}

void join_src_file_and_dst_dir(const char *src_file, char *dst_dir) {
    char basename[64];
    basename_r(src_file, basename);
    int dst_dir_end = strlen(dst_dir) - 1;
    if (dst_dir[dst_dir_end] != '/') {
        dst_dir[dst_dir_end + 1] = '/';
        dst_dir[dst_dir_end + 2] = '\0';
    }
    strncat(dst_dir, basename, 64);
}

void remove_last_path_part(char *path) {
    int i = strlen(path) - 1;
    while (path[i] == '/')
        --i;
    for (; i >= 0; --i) {
        if (path[i] == '/') {
            path[i] = '\0';
            break;
        }
    }
    if (strlen(path) == 0) {
        path[0] = '/';
        path[1] = '\0';
    }
}
