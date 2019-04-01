#include "write_request.h"

#include <stdlib.h>
#include <string.h>

#include "helpers.h"

#define DST_PATH_SIZE 256
#define DATA_SIZE 1024
#define IS_FOLDER_SIZE 1

void validate_write_request(const WriteRequest * write_request) {
    if (write_request->dst_path == NULL) {
        die(__LINE__, "dst_path is null");
    }

    if (write_request->data == NULL) {
        die(__LINE__, "data is null");
    }

    if (strlen(write_request->dst_path) >= DST_PATH_SIZE) {
        die(__LINE__, "dst_path too long to serialize %s", write_request->dst_path);
    }

    if (strlen(write_request->data) >= DATA_SIZE) {
        die(__LINE__, "data too long to serialize %s", write_request->data);
    }
}

char *serialize_write_request(const WriteRequest *input) {
    validate_write_request(input);

    char *output = (char *) malloc(DST_PATH_SIZE + DATA_SIZE + IS_FOLDER_SIZE);

    strncpy(output, input->dst_path, DST_PATH_SIZE);
    output[DST_PATH_SIZE] = input->is_folder ? '1' : '0';
    strncpy(output + DST_PATH_SIZE + IS_FOLDER_SIZE, input->data, DATA_SIZE);

    return output;
}

WriteRequest *deserialize_write_request(char *input) {
    WriteRequest * output = new_write_request();

    strncpy(output->dst_path, input, DST_PATH_SIZE);
    output->is_folder = input[DST_PATH_SIZE] == '1' ? true : false;
    strncpy(output->data, input + DST_PATH_SIZE + IS_FOLDER_SIZE, DATA_SIZE);

    return output;
}

WriteRequest * new_write_request() {
    WriteRequest *write_request = (WriteRequest *) malloc(sizeof(WriteRequest));
    write_request->dst_path = (char *) malloc(DST_PATH_SIZE);
    write_request->data = (char *) malloc(DATA_SIZE);
    return write_request;
}

void free_write_request(WriteRequest * write_request) {
    free(write_request->dst_path);
    free(write_request->data);
    free(write_request);
}
