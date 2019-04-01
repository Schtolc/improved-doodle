#include <netdb.h>
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
