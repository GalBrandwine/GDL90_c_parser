#include "unity.h"
#include "gdl90_heartbeat.h"

void setUp(void)
{
    printf("setting up test\n");
}

void tearDown(void)
{
    printf("tearing down\n");
}

void test_gdl_90_parser_heartbeat_message_sanity_check()
{
    //                                  Wrong ID
    //                                  |
    uint8_t raw_heartbeat_message[7] = {0x01, 0x81, 0x41, 0xDB, 0xD0, 0x08, 0x02};

    gdl90_heartbeat hb_message = parse_heartbeat_message(raw_heartbeat_message, 0);
    TEST_ASSERT_EQUAL(HEARTBEAT_MESSAGE_ID, hb_message.id);
}
/// @brief 2.2.4. Message Example The byte sequence [0x7E 0x00 0x81 0x41 0xDB 0xD0 0x08 0x02 0xB3 0x8B 0x7E] represents a Heartbeat message including the Flags and the CRC value.
void test_gdl_90_parser_heartbeat_message()
{

    uint8_t raw_message[11] = {0x7E, 0x00, 0x81, 0x41, 0xDB, 0xD0, 0x08, 0x02, 0xB3, 0x8B, 0x7E};
    //                                  ID    [Status   ] [TimeStamp] [message-cnt]
    uint8_t raw_heartbeat_message[7] = {0x00, 0x81, 0x41, 0xDB, 0xD0, 0x08, 0x02}; // No flags, no CRC

    gdl90_heartbeat hb_message = parse_heartbeat_message(raw_heartbeat_message, 1);

    TEST_ASSERT_EQUAL(HEARTBEAT_MESSAGE_ID, hb_message.id);
    /* Expected Status bits (2 bytes)
     *   bit position:  14 13 12 11 10 9 8 7 6 5 4 3 2 1 0
     *   bin value -    1  0  0  0  0  0 1 1 0 0 0 0 0 0 1
     *  100000110000001
     */
    TEST_ASSERT_TRUE(hb_message.status.byte1.gps_pos_valid);
    TEST_ASSERT_FALSE(hb_message.status.byte1.maint_req);
    TEST_ASSERT_FALSE(hb_message.status.byte1.ident);
    TEST_ASSERT_FALSE(hb_message.status.byte1.addr_type);
    TEST_ASSERT_FALSE(hb_message.status.byte1.gps_batt_low);
    TEST_ASSERT_FALSE(hb_message.status.byte1.ratcs);
    TEST_ASSERT_FALSE(hb_message.status.byte1.reserved5);
    TEST_ASSERT_TRUE(hb_message.status.byte1.uat_initialized);
    TEST_ASSERT_TRUE(hb_message.status.byte2.timestamp);
    TEST_ASSERT_FALSE(hb_message.status.byte2.csa_requested);
    TEST_ASSERT_FALSE(hb_message.status.byte2.csa_not_available);
    TEST_ASSERT_FALSE(hb_message.status.byte2.reserved4);
    TEST_ASSERT_FALSE(hb_message.status.byte2.reserved3);
    TEST_ASSERT_FALSE(hb_message.status.byte2.reserved2);
    TEST_ASSERT_FALSE(hb_message.status.byte2.reserved1);
    TEST_ASSERT_TRUE(hb_message.status.byte2.utc_ok);
}
// not needed when using generate_test_runner.rb
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_gdl_90_parser_heartbeat_message_sanity_check);
    RUN_TEST(test_gdl_90_parser_heartbeat_message);

    return UNITY_END();
}