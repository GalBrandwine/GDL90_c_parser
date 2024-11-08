#include "unity.h"
#include "air_sample_task.h"

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

void test_gdl_90_parse_first_byte()
{
    parser_status status = {0};

    gdl_90_parse(0x7E, &status);
    sleep(1);
    TEST_ASSERT_EQUAL(status.status, PROCESSING);
}

/// @brief 2.2.4. Message Example The byte sequence [0x7E 0x00 0x81 0x41 0xDB 0xD0 0x08 0x02 0xB3 0x8B 0x7E] represents a Heartbeat message including the Flags and the CRC value.
void test_gdl_90_parse_message()
{
    parser_status status = {0};
    int raw_message_size = 11;
    uint8_t raw_message[11] = {0x7E, 0x00, 0x81, 0x41, 0xDB, 0xD0, 0x08, 0x02, 0xB3, 0x8B, 0x7E, 0x00};

    // Simulate bytes incoming from GDL90 uart connection
    for (size_t i = 0; i < raw_message_size; i++)
    {
        gdl_90_parse(raw_message[i], &status);
        usleep(1000);
        TEST_ASSERT_EQUAL(status.status, PROCESSING);
    }
    
    sleep(1);
    TEST_ASSERT_EQUAL(status.status, PROCESSING);
}
// not needed when using generate_test_runner.rb
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_gdl_90_parse_message);
    RUN_TEST(test_gdl_90_parse_first_byte);
    return UNITY_END();
}