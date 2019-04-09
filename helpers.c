#include <netdb.h>
#include <libgen.h>
#include <stdbool.h>
#include "helpers.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "zconf.h"

void die_if(bool fail, const char *format, ...) {
    if (!fail)
        return;

    va_list vargs;
    va_start(vargs, format);
    vfprintf(stderr, format, vargs);
    fprintf(stderr, ".\n");
    va_end(vargs);
    exit(1);
}

struct addrinfo *resolve_addrinfo(const char *hostname, const char *port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;

    struct addrinfo *addr = NULL;
    int err = getaddrinfo(hostname, port, &hints, &addr);
    die_if(err != 0, "failed to resolve server socket address (err=%d)", err);

    return addr;
}
