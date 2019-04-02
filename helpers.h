#ifndef RCP_DIVAN_HELPERS_H
#define RCP_DIVAN_HELPERS_H

void die(int line_number, const char *format, ...);

int split_host_and_dir(const char *input, char *host, char *dir);

struct addrinfo *get_addrinfo_simple(const char *hostname, const char *port);

void join_src_file_and_dst_dir(const char *src_file, char *dst_dir);

void remove_last_path_part(char *path);

#endif //RCP_DIVAN_HELPERS_H
