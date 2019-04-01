#ifndef RCP_DIVAN_HELPERS_H
#define RCP_DIVAN_HELPERS_H

void die(int line_number, const char *format, ...);

int split_host_and_dir(const char *input, char *host, char *dir);

struct addrinfo *get_addrinfo_simple(const char *hostname, const char *port);

#endif //RCP_DIVAN_HELPERS_H
