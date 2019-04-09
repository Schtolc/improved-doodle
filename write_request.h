#ifndef RCP_DIVAN_WRITE_REQUEST_H
#define RCP_DIVAN_WRITE_REQUEST_H

#include "stdbool.h"
#include "stdint.h"

typedef struct WriteRequest {
    char *dst_dir;

    int data_length;
    uint8_t *data;
} WriteRequest;

WriteRequest *new_write_request(int data_length);

WriteRequest *load_write_request(char *file_path, const char *dst_dir);

void dump_write_request(WriteRequest * write_request);

void free_write_request(WriteRequest *write_request);

char *serialize_write_request(const WriteRequest *input);

WriteRequest *deserialize_write_request(const char *input);

int write_request_max_size();

int dst_path_max_size();

#endif //RCP_DIVAN_WRITE_REQUEST_H
