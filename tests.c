#include "contrib/unity/unity.h"
#include "helpers.h"
#include "stdlib.h"
#include "string.h"
#include "write_request.h"

void test_serialization_helper(WriteRequest *expected) {
    char *serialized = serialize_write_request(expected);
    WriteRequest *deserialized = deserialize_write_request(serialized);
    TEST_ASSERT_EQUAL(expected->is_folder, deserialized->is_folder);
    TEST_ASSERT_EQUAL(expected->data_length, deserialized->data_length);
    TEST_ASSERT_EQUAL_STRING(expected->dst_path, deserialized->dst_path);
    if (expected->data_length != 0) {
        TEST_ASSERT_EQUAL_UINT8_ARRAY(expected->data, deserialized->data, expected->data_length);
    }

    free(serialized);
    free_write_request(deserialized);
}

void test_serialization_simple() {
    const char *data = "abcdefg";

    WriteRequest *write_request = new_write_request(strlen(data));
    strncpy(write_request->dst_path, "/tmp/trash.txt", dst_path_max_size());
    write_request->is_folder = false;
    memcpy(write_request->data, data, strlen(data));

    test_serialization_helper(write_request);

    free_write_request(write_request);
}

void test_serialization_folder() {
    const char *data = "abcdefg";

    WriteRequest *write_request = new_write_request(strlen(data));
    strncpy(write_request->dst_path, "/tmp/trash.txt", dst_path_max_size());
    write_request->is_folder = true;
    memcpy(write_request->data, data, strlen(data));

    test_serialization_helper(write_request);

    free_write_request(write_request);
}

void test_serialization_empty_data() {
    WriteRequest *write_request = new_write_request(0);
    strncpy(write_request->dst_path, "/tmp/trash.txt", dst_path_max_size());
    write_request->is_folder = true;

    test_serialization_helper(write_request);

    free_write_request(write_request);
}

void test_serialization_root_path() {
    WriteRequest *write_request = new_write_request(0);
    strncpy(write_request->dst_path, "/", dst_path_max_size());
    write_request->is_folder = true;

    test_serialization_helper(write_request);

    free_write_request(write_request);
}

void test_serialization_binary_data() {
    const char *data = "abcdefg";

    WriteRequest *write_request = new_write_request(strlen(data));
    strncpy(write_request->dst_path, "/tmp/trash.txt", dst_path_max_size());
    write_request->is_folder = true;
    memcpy(write_request->data, data, strlen(data));
    write_request->data[1] = '\0'; // somewhere in the middle of the data

    test_serialization_helper(write_request);

    free_write_request(write_request);
}

void test_split_host_and_dir() {
    char host[32];
    char dir[256];

    int split_res = split_host_and_dir("host@/path/to/dir", host, dir);
    TEST_ASSERT_EQUAL(split_res, 0);
    TEST_ASSERT_EQUAL_STRING(host, "host");
    TEST_ASSERT_EQUAL_STRING(dir, "/path/to/dir");

    split_res = split_host_and_dir("0@/", host, dir);
    TEST_ASSERT_EQUAL(split_res, 0);
    TEST_ASSERT_EQUAL_STRING(host, "0");
    TEST_ASSERT_EQUAL_STRING(dir, "/");

    split_res = split_host_and_dir("@/path/to/dir", host, dir);
    TEST_ASSERT_EQUAL(split_res, -1);

    split_res = split_host_and_dir("host@", host, dir);
    TEST_ASSERT_EQUAL(split_res, -1);

    split_res = split_host_and_dir("host/path/to/dir", host, dir);
    TEST_ASSERT_EQUAL(split_res, -1);
}

void test_join_src_file_and_dst_dir() {
    char dst_dir[256];
    strncpy(dst_dir, "/cpy/", 256);
    join_src_file_and_dst_dir("/tmp/trash.txt", dst_dir);
    TEST_ASSERT_EQUAL_STRING(dst_dir, "/cpy/trash.txt");

    strncpy(dst_dir, "/cpy", 256);
    join_src_file_and_dst_dir("/tmp/trash.txt", dst_dir);
    TEST_ASSERT_EQUAL_STRING(dst_dir, "/cpy/trash.txt");

    strncpy(dst_dir, "/", 256);
    join_src_file_and_dst_dir("/tmp/trash.txt", dst_dir);
    TEST_ASSERT_EQUAL_STRING(dst_dir, "/trash.txt");

    strncpy(dst_dir, "/cpy", 256);
    join_src_file_and_dst_dir("/trash.txt", dst_dir);
    TEST_ASSERT_EQUAL_STRING(dst_dir, "/cpy/trash.txt");

    strncpy(dst_dir, "/cpy", 256);
    join_src_file_and_dst_dir("/trash/", dst_dir);
    TEST_ASSERT_EQUAL_STRING(dst_dir, "/cpy/trash");
}

void test_remove_last_path_part() {
    char path[256];
    strncpy(path, "/cpy/trash.txt", 256);
    remove_last_path_part(path);
    TEST_ASSERT_EQUAL_STRING(path, "/cpy");

    strncpy(path, "/cpy/trash.txt/", 256);
    remove_last_path_part(path);
    TEST_ASSERT_EQUAL_STRING(path, "/cpy");

    strncpy(path, "/cpy", 256);
    remove_last_path_part(path);
    TEST_ASSERT_EQUAL_STRING(path, "/");
}

void test_serialization_long_strings() {
    const char *data = "VMLcam2W2ldtZsruqzDp3KVsDJWwzSl3XcKEix3EKY0aDjAzgBTf80Wt3Z81k156K25NtkitoP754GONydrhf8cpgKZw"
                       "zorQe5s2hDeUyNhR3bzsgkR3Ao6k7WcEFeWJdTZIj73wK6AVquAJqxMxjcXJEMWtchaNggMQklj8MgU2b2ukFnOnNc6u"
                       "753hurG02Mx7XdhgvOUpE6ovNVN295UTyKSkqGNrz0Hn0y0Y1xA5uQGaEunEdLHH4xgNov2h5NGCG0yypZk4bp1h3PQV"
                       "7jPo6vk8pxPlExlqqfHm8VTBAR7ZEanZRy7OmHN1TrFW8gwy0ORPDM2d8AzlZlMSLrJIYSrGF6RhKyq9KFPmNZfXlEOO"
                       "ctC8kW3PxLzswksGRaZiEpvYaKLimRxXwlrXV0TbW9gZCc4JTXmMe3ui1jVouFyVgtwmBmlvylu67jIwL3KPvt5L1SO4"
                       "D7GyJAC9JuMRAYsjacf5tYLOWEcj72w1Cx8fBJyZpnVtqAUHECCUXtRMlhLDudBaq5ZZ6MHWNm8y0aWSccGE9atVzFBA"
                       "7SUbxYhoN2XDYMyRNh3LryUzJwNEP38PiJumpqHnBK22HoWWU0PVATnV4iQH7tWLUKyOyp3wptITUR8Jg4Do4qCWzmx3"
                       "OABbcnJzycpxVzRx1EoytuC3ahvpwRyDUXMT5KsPuSOQGgTgCcdPMw9VcAVJlbtbSBPgC2zZBUPYhf6iE0Unc5j4rxp9"
                       "q0FtJ99e19bwas8Dj2pO4fzbdZxuxPbcvIcHoL6s24kYJiKpNhCCoYMy2uehwcrJCOWazEsg8hGhZom1kN9uc2webyzo"
                       "3L19syYzh1uYiFx8jN739K5eai5ubxOLKKAXCsxpLL3hmwygSZFJQqU6YoyrTZxAxI3wMOhIGNiN6WNglaeNtfgeE2Su"
                       "GkaC40uZBqS3ESpfmoA34y2i3cHKU5XS2PwhY0wSEe2E7D1Xhhshi9K2cVNq7AqAvkZxpjpgpRFA3mSXGz4vzz0A44N5"
                       "5Bl2j4zicZJ";
    TEST_ASSERT_EQUAL(strlen(data), 1023);

    WriteRequest *write_request = new_write_request(strlen(data));
    const char *dst_path = "zOpCdiX1y5U0EhI5drrdmRSJ746gR0VDF0jGBRO6Gt5tBMLGRWFt9AHZXEqhgP2yBKa4pB1oYJkf5JROVFoZN0B7"
                           "M6JbapsepFxU8idNl3p8RxhTqlM6EW9SkfubPwR4hHswJIWOOEXXrSLumUHl0yw1D09gEhSeWL7QezyocVVQjClV"
                           "GHreUHuJxyC9Iyj4mpTOcuzPr04DXC5IBDb9zN9vjbGndomzhUjkpNGRU3uoMwo1rpgEtCWHXONltJY";
    TEST_ASSERT_EQUAL(strlen(dst_path), 255);

    strncpy(write_request->dst_path, dst_path, dst_path_max_size());
    write_request->is_folder = true;
    memcpy(write_request->data, data, strlen(data));

    test_serialization_helper(write_request);

    char *serialized = serialize_write_request(write_request);
    WriteRequest *deserialized = deserialize_write_request(serialized);
    TEST_ASSERT_EQUAL(strlen(deserialized->dst_path), 255);
    TEST_ASSERT_EQUAL(deserialized->data_length, 1023);
    free(serialized);
    free_write_request(deserialized);
    free_write_request(write_request);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_serialization_simple);
    RUN_TEST(test_serialization_folder);
    RUN_TEST(test_serialization_empty_data);
    RUN_TEST(test_serialization_root_path);
    RUN_TEST(test_serialization_long_strings);
    RUN_TEST(test_serialization_binary_data);

    RUN_TEST(test_split_host_and_dir);

    RUN_TEST(test_join_src_file_and_dst_dir);

    RUN_TEST(test_remove_last_path_part);

    return UNITY_END();
}
