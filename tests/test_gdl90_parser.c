#include "unity.h"
#include "air_sample_task.h"
#include <unistd.h>
gdl90_heartbeat result_message;

void setUp(void)
{
    printf("setting up test\n");
    init_parser();
}

void tearDown(void)
{
    printf("tearing down\n");
    shut_down();
}

void test_gdl90_parse_first_byte()
{
    parser_status status = {0};

    gdl90_parse(0x7E, &status);
    sleep(1);
    TEST_ASSERT_EQUAL(status.status, PROCESSING);
}

/// @brief Example usage of a message returned by the parser
/// This is an explicit callback for the heartbeat message.
/// @param arg
void heartbeat_callback(void *arg)
{

    if (arg != NULL)
    {
        printf("Got heartbeat message!\n");
        result_message = *((gdl90_heartbeat *)arg);
        print_heartbeat_message(&result_message);
    }
    else
    {
        printf("heartbeat_callback called with NULL argument\n");
    }
}

/// @brief 2.2.4. Message Example The byte sequence [0x7E 0x00 0x81 0x41 0xDB 0xD0 0x08 0x02 0xB3 0x8B 0x7E] represents a Heartbeat message including the Flags and the CRC value.
void test_gdl90_parse_message()
{
    parser_status status = {0};
    status.status = PROCESSING;

    // Store callbackMap in status
    status.callbackMap[HEARTBEAT] = heartbeat_callback;

    int raw_message_size = 11;
    uint8_t raw_message[11] = {0x7E, 0x00, 0x81, 0x41, 0xDB, 0xD0, 0x08, 0x02, 0xB3, 0x8B, 0x7E};

    // Simulate bytes incoming from GDL90 uart connection
    for (size_t i = 0; i < raw_message_size; i++)
    {
        gdl90_parse(raw_message[i], &status);
        usleep(100);
    }

    while (status.status == PROCESSING)
    {
        usleep(100);
    }
    TEST_ASSERT_EQUAL(status.status, MESSAGE_READY);
}

/// @brief 2.2.4. Message Example The byte sequence [0x7E 0x00 0x81 0x41 0xDB 0xD0 0x08 0x02 0xB3 0x8B 0x7E] represents a Heartbeat message including the Flags and the CRC value.
void test_gdl90_parse_two_messages()
{
    parser_status status = {0};
    status.status = PROCESSING;

    // Store callbackMap in status
    status.callbackMap[HEARTBEAT] = heartbeat_callback;

    int raw_message_size = 11;
    uint8_t raw_message[11] = {0x7E, 0x00, 0x81, 0x41, 0xDB, 0xD0, 0x08, 0x02, 0xB3, 0x8B, 0x7E};

    // Simulate bytes incoming from GDL90 uart connection
    for (size_t i = 0; i < raw_message_size; i++)
    {
        gdl90_parse(raw_message[i], &status);
        usleep(100);
    }

    while (status.status == PROCESSING)
    {
        usleep(100);
    }
    TEST_ASSERT_EQUAL(status.status, MESSAGE_READY);
    int prev_message_count = result_message.message_counts;

    printf("\n\n Starting second part of the test\n\n");

    uint8_t raw_message_2[11] = {0x7E, 0x00, 0x81, 0x41, 0xDB, 0xD0, 0x09, 0x02, 0xB3, 0x8A, 0x7E}; // Updated the message_counter
    // Simulate bytes incoming from GDL90 uart connection
    for (size_t i = 0; i < raw_message_size; i++)
    {
        gdl90_parse(raw_message_2[i], &status);
        usleep(100);
    }

    while (status.status == PROCESSING)
    {
        usleep(100);
    }
    TEST_ASSERT_EQUAL(status.status, MESSAGE_READY);
    TEST_ASSERT_EQUAL(prev_message_count + 1, result_message.message_counts);
}

void test_gdl90_parse_message_byte_stuffing()
{
    parser_status status = {0};
    status.status = PROCESSING;

    // Store callbackMap in status
    status.callbackMap[HEARTBEAT] = heartbeat_callback;

    int raw_message_size = 12;
    //                                                                  Control
    //                                                                   Escape  Xored with 0x20
    //                                                                      |     |
    uint8_t raw_message_with_byte_stuffing[12] = {0x7E, 0x00, 0x81, 0x41, 0x7D, 0x5D, 0xD0, 0x08, 0x02, 0x6B, 0x3C, 0x7E};
    uint8_t expected_raw_message_befor_byte_stuffing[9] = {0x00, 0x81, 0x41, 0x7d, 0xd0, 0x08, 0x02, 0x6B, 0x3C}; // Without Flags

    // Simulate bytes incoming from GDL90 uart connection
    for (size_t i = 0; i < raw_message_size; i++)
    {
        gdl90_parse(raw_message_with_byte_stuffing[i], &status);
        usleep(100);
    }

    while (status.status == PROCESSING)
    {
        usleep(100);
    }
    TEST_ASSERT_EQUAL(status.status, MESSAGE_READY);

    // Check if byte de-stuffing worked
    // The relevant field is heartbeat timestamp
    uint32_t expected_timestamp = (uint32_t)1 << 17 | ((uint16_t)expected_raw_message_befor_byte_stuffing[4] << 8) | expected_raw_message_befor_byte_stuffing[3];
    TEST_ASSERT_EQUAL(expected_timestamp, result_message.timestamp);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_gdl90_parse_message_byte_stuffing);
    RUN_TEST(test_gdl90_parse_two_messages);
    RUN_TEST(test_gdl90_parse_message);
    RUN_TEST(test_gdl90_parse_first_byte);
    return UNITY_END();
}