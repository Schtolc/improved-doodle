#include <sys/stat.h>
#include <errno.h>
#include <zconf.h>
#include <libgen.h>
#include "write_request.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "helpers.h"

#define DST_DIR_SIZE 256
#define DATA_SIZE 1024
#define DATA_LENGTH_SIZE 8

static void validate_write_request(const WriteRequest *write_request) {
    if (write_request->dst_dir == NULL || strlen(write_request->dst_dir) == 0) {
        die_if(__LINE__, "dst_dir is null");
    }

    if (write_request->data_length != 0 && write_request->data == NULL) {
        die_if(__LINE__, "data is null");
    }

    if (strlen(write_request->dst_dir) >= DST_DIR_SIZE) {
        die_if(__LINE__, "dst_dir too long to serialize %s", write_request->dst_dir);
    }

    if (write_request->data_length >= DATA_SIZE || write_request->data_length > 9999999 /* DATA_LENGTH_SIZE - 1 */) {
        die_if(__LINE__, "data too long to serialize %s", write_request->data);
    }
}

int write_request_max_size() {
    return DST_DIR_SIZE + DATA_LENGTH_SIZE + DATA_SIZE;
}

int dst_path_max_size() {
    return DST_DIR_SIZE;
}

// | dst_dir\0 | data_length\0 | data |
char *serialize_write_request(const WriteRequest *input) {
    validate_write_request(input);

    char *output = (char *) calloc(write_request_max_size(), sizeof(char));

    strncpy(output, input->dst_dir, DST_DIR_SIZE);

    char data_length_str[DATA_LENGTH_SIZE];
    sprintf(data_length_str, "%d", input->data_length);
    strncpy(output + DST_DIR_SIZE, data_length_str, DATA_LENGTH_SIZE);

    memcpy(output + DST_DIR_SIZE + DATA_LENGTH_SIZE, input->data, input->data_length);

    return output;
}

WriteRequest *deserialize_write_request(const char *input) {
    char data_length_str[DATA_LENGTH_SIZE];
    strncpy(data_length_str, input + DST_DIR_SIZE, DATA_LENGTH_SIZE);
    int data_length = atoi(data_length_str);

    WriteRequest *output = new_write_request(data_length);

    strncpy(output->dst_dir, input, DST_DIR_SIZE);
    memcpy(output->data, input + DST_DIR_SIZE + DATA_LENGTH_SIZE, output->data_length);

    return output;
}

WriteRequest *new_write_request(int data_length) {
    WriteRequest *write_request = (WriteRequest *) calloc(1, sizeof(WriteRequest));
    write_request->dst_dir = (char *) calloc(DST_DIR_SIZE, sizeof(char));
    write_request->data_length = data_length;
    write_request->data = (uint8_t *) calloc(write_request->data_length, sizeof(uint8_t));
    return write_request;
}

WriteRequest *new_write_request_from_file(char *file_path, const char *dst_path) {
        // cd file_path withour last /
    char file_name[64];
    basename_r(file_path, file_name);
    remove_last_path_part(file_path);

    int chdir_res = chdir(file_path);
    die_if(chdir_res == -1, "failed to chdir %s", strerror(errno));

    char tar_command[256];
    snprintf(tar_command, 256, "tar -czvf my.tar.gz %s", file_name);
    int system_res = system(tar_command);
    die_if(system_res == -1, "failed to tar %s", file_name);

    WriteRequest *write_request = NULL;
    FILE *fileptr = fopen("my.tar.gz", "rb");
    die_if(fileptr == NULL, "failed to open file my.tar.gz");

    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    int filelen = ftell(fileptr);         // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    write_request = new_write_request(filelen);
    strncpy(write_request->dst_dir, dst_path, dst_path_max_size());
    fread(write_request->data, filelen, 1, fileptr);
    fclose(fileptr);
    return write_request;
}

void free_write_request(WriteRequest *write_request) {
    free(write_request->dst_dir);
    free(write_request->data);
    free(write_request);
}
