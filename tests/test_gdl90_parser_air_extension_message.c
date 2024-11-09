#include "unity.h"
#include "gdl90_air_extension.h"

const int request_flight_command_byte_index = 4;
int roll_angle_command_bit_index = (request_flight_command_byte_index + 1) * 8; // Comes right after request_flight_command
/// @brief Helper function for creating a buffer containing a pgn 65282 message
/// @param A buffer
/// @param k i'th bit
void setBit(int A[], int k)
{
    A[k / 32] |= 1 << (k % 32); // Set the bit at the k-th position in A[i]
}

void printBits(unsigned int num)
{
    unsigned int size = sizeof(unsigned int);
    unsigned int maxPow = 1 << (size * 8 - 1);
    // printf("MAX POW : %u\n", maxPow);
    int i = 0, j;
    for (; i < size; ++i)
    {
        for (; i < size * 8; ++i)
        {
            // print last bit and shift left.
            printf("%u ", num & maxPow ? 1 : 0);
            num = num << 1;
        }
    }
}

// Function to print the bits of a float
void printFloatBits(float value)
{
    // Create a union to interpret the float as an integer
    union
    {
        float f;
        uint32_t u;
    } floatUnion;

    // Assign the float value to the union
    floatUnion.f = value;

    // Print the bits
    printf("Float value: %f\n", value);
    printf("Bits: ");
    for (int i = 31; i >= 0; i--)
    {
        printf("%d", (floatUnion.u >> i) & 1);
        if (i == 31 || i == 23)
        {
            printf(" "); // Add space after the sign bit and exponent bits
        }
    }
    printf("\n");
}

void set_id_GDL90_AIR_EXTENSION(uint8_t *buffer)
{
    // setBit(buffer, 0); //GDL90_AIR_EXTENSION; //0100 0000
    // setBit(buffer, 1);
    // setBit(buffer, 2);
    // setBit(buffer, 3);

    // setBit(buffer, 4);
    // setBit(buffer, 5);
    setBit(buffer, 6);
    // setBit(buffer, 7);
}

void set_PGN_65282(uint8_t *buffer)
{
    // setBit(buffer, 8);  // 0x02; // PGN 65282 lsb - 0000 0010
    setBit(buffer, 9);
    // setBit(buffer, 10);
    // setBit(buffer, 11);

    // setBit(buffer, 12);
    // setBit(buffer, 13);
    // setBit(buffer, 14);
    // setBit(buffer, 15);

    setBit(buffer, 16); // 0xFF; // PGN 65282 msb - 1111 1111
    setBit(buffer, 17);
    setBit(buffer, 18);
    setBit(buffer, 19);

    setBit(buffer, 20);
    setBit(buffer, 21);
    setBit(buffer, 22);
    setBit(buffer, 23);
}

void set_SA(uint8_t *buffer)
{
    int byte_index = 3;
    // setBit(buffer, 8*byte_index + 0);  // SA is 1 byte
    // setBit(buffer, 8*byte_index + 1);
    // setBit(buffer, 8*byte_index + 2);
    // setBit(buffer, 8*byte_index + 3);

    // setBit(buffer, 8*byte_index + 4);
    // setBit(buffer, 8*byte_index + 5);
    // setBit(buffer, 8*byte_index + 6);
    // setBit(buffer, 8*byte_index + 7);
}

/// @brief First byte is Flight mode flags
// Filling with made-up flags
/// @param buffer
void set_request_flight_command(uint8_t *buffer)
{

    setBit(buffer, 8 * request_flight_command_byte_index + 0);
    // setBit(buffer, 8*request_flight_command_byte_index + 1);
    // setBit(buffer, 8*request_flight_command_byte_index + 2);
    // setBit(buffer, 8*request_flight_command_byte_index + 3);

    // setBit(buffer, 8*request_flight_command_byte_index + 4);
    // setBit(buffer, 8*request_flight_command_byte_index + 5);
    setBit(buffer, 8 * request_flight_command_byte_index + 6);
    // setBit(buffer, 8*request_flight_command_byte_index + 7);
}

void set_roll_angle_command(uint8_t *buffer)
{
    float value = 4.0f;
    // Convert the floating-point value to Q9/90 fixed-point representation
    // float_t q9_90_roll_angle_command = 45.0f * (0.176125245f); // convert float to Q9/90

    float q_9_81_factor = (9.81f / 511.0f);
    printf("q_9_81_factor = %f\n", q_9_81_factor);
    float scaledValue = value * q_9_81_factor;
    printf("scaledValue = %f\n", scaledValue);
    printFloatBits(scaledValue);

    // 10 Bits
    // setBit(buffer, roll_angle_command_bit_index + 0);
    // setBit(buffer, roll_angle_command_bit_index + 1);
    // setBit(buffer, roll_angle_command_bit_index + 2);
    // setBit(buffer, roll_angle_command_bit_index + 3);
    setBit(buffer, roll_angle_command_bit_index + 4);
    // setBit(buffer, roll_angle_command_bit_index + 5);
    setBit(buffer, roll_angle_command_bit_index + 6);
    setBit(buffer, roll_angle_command_bit_index + 7);
    // setBit(buffer, roll_angle_command_bit_index + 8);
    setBit(buffer, roll_angle_command_bit_index + 9);
}
int got_65828_message = 0;

/// @brief Example usage of a message returned by the parser
/// This is an explicit callback for the heartbeat message.
/// @param arg
void air_extension_pgn_65282_callback(void *arg)
{
    pgn_85282_request_flight_command message;
    if (arg != NULL)
    {
        printf("Got pgn 65282 message!\n");
        message = *((pgn_85282_request_flight_command *)arg);
        // printf("message.flight_data =  %d[10]\n", message.flight_data);
        printf("message.flight_data                     =  %b\n", message.flight_mode_flags.raw);
        printf("message.flight_data.bits.takeoff        =  %d\n", message.flight_mode_flags.bits.takeoff);
        printf("message.flight_data.bits.land           =  %d\n", message.flight_mode_flags.bits.land);
        printf("message.flight_data.bits.steer_left     =  %d\n", message.flight_mode_flags.bits.steer_left);
        printf("message.flight_data.bits.steer_right    =  %d\n", message.flight_mode_flags.bits.steer_right);
        printf("message.flight_data.bits.roll_left      =  %d\n", message.flight_mode_flags.bits.roll_left);
        printf("message.flight_data.bits.roll_right     =  %d\n", message.flight_mode_flags.bits.roll_right);
        printf("message.flight_data.bits.shift_left     =  %d\n", message.flight_mode_flags.bits.shift_left);
        printf("message.flight_data.bits.shift_right    =  %d\n", message.flight_mode_flags.bits.shift_right);

        printf("message.roll_angle_command              =  %d\n", message.roll_angle_command);

        got_65828_message = 1;
    }
    else
    {
        printf("heartbeat_callback called with NULL argument\n");
    }
}

void setUp(void)
{
    printf("setting up test\n");
    got_65828_message = 0;
}

void tearDown(void)
{
    printf("tearing down\n");
}

void test_gdl90_parser_air_extension_message_message_sanity_check()
{
    //                                  Wrong ID
    //                                  |
    uint8_t air_extension_message[7] = {0x4f, 0x81, 0x41, 0xDB, 0xD0, 0x08, 0x02};

    parser_status status;
    status.callbackMap[GDL90_AIR_EXTENSION] = air_extension_pgn_65282_callback;
    parse_gdl90_air_extension_message(air_extension_message, &status, 1);
    TEST_ASSERT_EQUAL(got_65828_message, 0);
}

void test_gdl90_parser_air_extension_message_message()
{
    // Setup
    parser_status status;
    status.callbackMap[GDL90_AIR_EXTENSION] = air_extension_pgn_65282_callback;

    // Message with PGN 65282
    int message_size = 7;
    uint8_t raw_air_extension_message[7] = {0}; // GDL90_AIR_EXTENSION, 0x02, 0xFF, 0x04, 0x00, 0x08, 0x02}; // No flags, no CRC

    set_id_GDL90_AIR_EXTENSION(raw_air_extension_message);
    set_PGN_65282(raw_air_extension_message);
    set_SA(raw_air_extension_message);
    set_request_flight_command(raw_air_extension_message);
    set_roll_angle_command(raw_air_extension_message);

    for (int i = 0; i < message_size; i++)
    {
        printf("%x ", raw_air_extension_message[i]);
        printBits(raw_air_extension_message[i]);
        printf("\n");
    }
    printf("\n");

    parse_gdl90_air_extension_message(raw_air_extension_message, &status, 1);
    TEST_ASSERT_EQUAL(got_65828_message, 1);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_gdl90_parser_air_extension_message_message_sanity_check);
    RUN_TEST(test_gdl90_parser_air_extension_message_message);

    return UNITY_END();
}