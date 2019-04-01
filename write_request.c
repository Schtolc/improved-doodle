#include "write_request.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "helpers.h"

#define DST_PATH_SIZE 256
#define DATA_SIZE 1024
#define DATA_LENGTH_SIZE 8
#define IS_FOLDER_SIZE 1

static void validate_write_request(const WriteRequest *write_request) {
    if (write_request->dst_path == NULL || strlen(write_request->dst_path) == 0) {
        die(__LINE__, "dst_path is null");
    }

    if (write_request->data_length != 0 && write_request->data == NULL) {
        die(__LINE__, "data is null");
    }

    if (strlen(write_request->dst_path) >= DST_PATH_SIZE) {
        die(__LINE__, "dst_path too long to serialize %s", write_request->dst_path);
    }

    if (write_request->data_length >= DATA_SIZE || write_request->data_length > 9999999 /* DATA_LENGTH_SIZE - 1 */) {
        die(__LINE__, "data too long to serialize %s", write_request->data);
    }
}

int write_request_max_size() {
    return DST_PATH_SIZE + IS_FOLDER_SIZE + DATA_LENGTH_SIZE + DATA_SIZE;
}

int dst_path_max_size() {
    return DST_PATH_SIZE;
}

// | dst_path\0 | is_folder | data_length\0 | data |
char *serialize_write_request(const WriteRequest *input) {
    validate_write_request(input);

    char *output = (char *) calloc(write_request_max_size(), sizeof(char));

    strncpy(output, input->dst_path, DST_PATH_SIZE);
    output[DST_PATH_SIZE] = input->is_folder ? '1' : '0';

    char data_length_str[DATA_LENGTH_SIZE];
    sprintf(data_length_str, "%d", input->data_length);
    strncpy(output + DST_PATH_SIZE + IS_FOLDER_SIZE, data_length_str, DATA_LENGTH_SIZE);

    memcpy(output + DST_PATH_SIZE + IS_FOLDER_SIZE + DATA_LENGTH_SIZE, input->data, input->data_length);

    return output;
}

WriteRequest *deserialize_write_request(const char *input) {
    char data_length_str[DATA_LENGTH_SIZE];
    strncpy(data_length_str, input + DST_PATH_SIZE + IS_FOLDER_SIZE, DATA_LENGTH_SIZE);
    int data_length = atoi(data_length_str);

    WriteRequest *output = new_write_request(data_length);

    strncpy(output->dst_path, input, DST_PATH_SIZE);
    output->is_folder = input[DST_PATH_SIZE] == '1' ? true : false;
    memcpy(output->data, input + DST_PATH_SIZE + IS_FOLDER_SIZE + DATA_LENGTH_SIZE, output->data_length);

    return output;
}

WriteRequest *new_write_request(int data_length) {
    WriteRequest *write_request = (WriteRequest *) calloc(1, sizeof(WriteRequest));
    write_request->dst_path = (char *) calloc(DST_PATH_SIZE, sizeof(char));
    write_request->data_length = data_length;
    write_request->data = (uint8_t *) calloc(write_request->data_length, sizeof(uint8_t));
    return write_request;
}

void free_write_request(WriteRequest *write_request) {
    free(write_request->dst_path);
    free(write_request->data);
    free(write_request);
}
