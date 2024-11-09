#ifndef GDL90_AIR_EXTENSION_H
#define GDL90_AIR_EXTENSION_H
#include "common.h"
#include <math.h>

typedef union flight_mode_flags
{
    uint8_t raw;
    struct bits
    {
        // I made it up - no further documentation than `bit encoding`
        uint8_t takeoff : 1; // MSB
        uint8_t land : 1;
        uint8_t steer_left : 1;
        uint8_t steer_right : 1;
        uint8_t roll_left : 1;
        uint8_t roll_right : 1;
        uint8_t shift_left : 1;
        uint8_t shift_right : 1; // LSB
    } bits;
} flight_mode_flags;

typedef struct pgn_85282_request_flight_command
{
    /// @brief Roll angle command
    ///
    /// Bit offset: 0
    /// Bit length: 8
    /// Data Type:  uint
    /// Units:      bit encoding
    flight_mode_flags flight_mode_flags;

    /// @brief Roll angle command
    ///
    /// Bit offset: 8
    /// Bit length: 10
    /// Data Type:  Q9/90 (q_notation::Q9_90)
    /// Units:      deg
    int16_t roll_angle_command;

    /// @brief Pitch angle command
    ///
    /// Bit offset: 18
    /// Bit length: 10
    /// Data Type:  Q9/90 (q_notation::Q9_90)
    /// Units:      deg
    int16_t pitch_angle_command;

    /// @brief Yaw angle command
    ///
    /// Bit offset: 28
    /// Bit length: 10
    /// Data Type:  Q9/90 (q_notation::Q9_90)
    /// Units:      deg/s
    int16_t yaw_angle_command;

    /// @brief Climb rate command
    ///
    /// Bit offset: 38
    /// Bit length: 10
    /// Data Type:  Q9/30 (q_notation::Q9_30)
    /// Units:      m/s
    int16_t climb_rate_command;

} pgn_85282_request_flight_command;

typedef struct gdl90_air_extension
{
    uint8_t id;
    uint32_t pgn;
    uint8_t sa;
    /// @brief Pointing to the start of the FCIB data section ina J1939 PGN message (This data section will be up to 1785 Bytes long)
    const uint8_t *fcib_data;
} gdl90_air_extension;

static void print_gdl90_air_extension_message(gdl90_air_extension *message)
{
    printf("message.pgn                            =  %d[10]\n", message->pgn);
    printf("message.pgn                            =  %x\n", message->pgn);
}

static void handle_pgn_65282(gdl90_air_extension *message, parser_status *status)
{
    pgn_85282_request_flight_command request_flight_command;

    request_flight_command.flight_mode_flags.raw = (uint8_t)message->fcib_data[0];

    /*
     * This parser does not support Q notation yet.
     */
    // request_flight_command.roll_angle_command =
    // request_flight_command.pitch_angle_command =
    // request_flight_command.yaw_angle_command =
    // request_flight_command.climb_rate_command =

    // After successfully parsing pgn_65828 message
    if (status->callbackMap[GDL90_AIR_EXTENSION])
    {
        status->status = MESSAGE_READY;
        status->message_ready_type = GDL90_AIR_EXTENSION;
        status->callbackMap[GDL90_AIR_EXTENSION]((void *)&request_flight_command);
    }
    else
    {
        printf("no attached callbacks");
    }
}

/// @brief parse from buffer a gdl90_air_extension
/// This parser will extract the J1939 message out of a GDL90 message, and parse it according to its PGN
/// @param buffer
/// @param status_parser - pointer to a context containing possible callback attached to specific message types
/// @param verbose
static void parse_gdl90_air_extension_message(const uint8_t *buffer, parser_status *status, int verbose)
{
    gdl90_air_extension message = {0};
    message.id = buffer[0];

    // Sanity check
    if (message.id != GDL90_AIR_EXTENSION && message.id != GDL90_AIR_EXTENSION_1)
    {
        printf("[parse_gdl90_air_extension_message] cant parse buffer to GDL90_AIR_EXTENSION message, wrong ID [expected=%d, got=%d]\n", GDL90_AIR_EXTENSION, message.id);
        message.id = UNKNOWN_MESSAGE; // Setting only ID, but leaving everything empty
        return;
    }

    /*
    The full PGN consists of 18 bits. The 18-th MSB is always assumed to be zero.
    The 17-th MSB is derived from the Message ID. The Message ID of 0x40
    covers PGN range of 0x0000 to 0xFFFF. The Message ID of 0x41 covers PGN
    range of 0x10000 to 0x1FFFF.
    */
    message.pgn = (uint32_t)(message.id & 0x000000001) << 17 | (uint32_t)buffer[2] << 8 | buffer[1];
    message.sa = buffer[3];

    switch (message.pgn)
    {
    case 61444:
        printf("J1939 PGN 61444 not supported");
        break;
    case 65282:
        printf("handling GN 65282: Requested Flight Command\n");
        message.fcib_data = buffer + 4; // The start of the FCIB data
        handle_pgn_65282(&message, status);
        break;

    default:
        printf("GDN %d not supported by this parser\n", message.pgn);
        break;
    }
}

#endif /*GDL90_AIR_EXTENSION_H*/