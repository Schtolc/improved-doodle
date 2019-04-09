#ifndef RCP_DIVAN_HELPERS_H
#define RCP_DIVAN_HELPERS_H

#include "stdbool.h"

#define DEFAULT_PORT "8888"

void die_if(bool fail, const char *format, ...);

struct addrinfo *resolve_addrinfo(const char *hostname, const char *port);

void remove_last_path_part(char *path);

#endif //RCP_DIVAN_HELPERS_H
