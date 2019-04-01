#ifndef RCP_DIVAN_WRITE_REQUEST_H
#define RCP_DIVAN_WRITE_REQUEST_H

#include <stdbool.h>
#include <stdint.h>

typedef struct WriteRequest {
    char *dst_path;
    bool is_folder;

    char *data;
} WriteRequest;

WriteRequest * new_write_request();
char * serialize_write_request(const WriteRequest * input);
WriteRequest * deserialize_write_request(char * input);
void free_write_request(WriteRequest * write_request);

#endif //RCP_DIVAN_WRITE_REQUEST_H
