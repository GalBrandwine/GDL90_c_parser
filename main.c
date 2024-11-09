#include "air_sample_task.h"

/// @brief Example usage of a message returned by the parser
/// This is an explicit callback for the heartbeat message.
/// @param arg
void air_extension_pgn_65282_callback(void *arg)
{
    pgn_85282_request_flight_command message;
    if (arg != NULL)
    {
        printf("Got pgn 65282 message! [NOTE - this parser doesnt support Q notation yet]\n");
        message = *((pgn_85282_request_flight_command *)arg);

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
    }
    else
    {
        printf("heartbeat_callback called with NULL argument\n");
    }
}

gdl90_heartbeat heartbeat_message;
/// @brief Example usage of a message returned by the parser
/// This is an explicit callback for the heartbeat message.
/// @param arg
void heartbeat_callback(void *arg)
{

    if (arg != NULL)
    {
        printf("Got heartbeat message!\n");
        heartbeat_message = *((gdl90_heartbeat *)arg);
    }
    else
    {
        printf("heartbeat_callback called with NULL argument\n");
    }
}

int main(int argc, char const *argv[])
{
    int ret = init_parser();
    if (ret != 0)
    {
        printf("failed initiating parser, exiting program");
        return 1;
    }

    parser_status status = {0};
    // Store callbackMap in status
    status.callbackMap[HEARTBEAT_MESSAGE_ID] = heartbeat_callback;
    status.callbackMap[GDL90_AIR_EXTENSION] = air_extension_pgn_65282_callback;
    status.status = PROCESSING;

    int raw_message_size = 12;
    //                                                                  Control
    //                                                                   Escape  Xored with 0x20
    //                                                                      |     |
    uint8_t raw_message_with_byte_stuffing[12] = {0x7E, 0x00, 0x81, 0x41, 0x7D, 0x5D, 0xD0, 0x08, 0x02, 0x6B, 0x3C, 0x7E};

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

    if (status.status == MESSAGE_READY)
    {
        printf("Got GDL90 heartbeat message:\n");
        print_heartbeat_message(&heartbeat_message);
    }

    // status.status = PROCESSING;

    // Message with PGN 65282
    raw_message_size = 13;
    uint8_t raw_air_extension_message[13] = {0x7E, GDL90_AIR_EXTENSION, 0x02, 0xFF, 0x81, 0x41, 0x5D, 0xD0, 0x08, 0x02, 0x8E, 0x36, 0x7E};
    //    {0}; // GDL90_AIR_EXTENSION, 0x02, 0xFF, 0x04, 0x00, 0x08, 0x02}; // No flags, no CRC

    // Simulate bytes incoming from GDL90 uart connection
    for (size_t i = 0; i < raw_message_size; i++)
    {
        gdl90_parse(raw_air_extension_message[i], &status);
        usleep(100);
    }

    while (status.status == PROCESSING)
    {
        usleep(100);
    }

    if (status.status == MESSAGE_READY)
    {
        printf("Got PGN 65282 message:\n");
    }
    shut_down();
    return 0;
}
