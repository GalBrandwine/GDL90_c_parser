#ifndef GDL90_HEARTBEAT_H
#define GDL90_HEARTBEAT_H
#include "common.h"
typedef struct byte1_bits
{
    // Byte 1 (Status Byte 1)
    uint8_t gps_pos_valid : 1;   // Bit 7
    uint8_t maint_req : 1;       // Bit 6
    uint8_t ident : 1;           // Bit 5
    uint8_t addr_type : 1;       // Bit 4
    uint8_t gps_batt_low : 1;    // Bit 3
    uint8_t ratcs : 1;           // Bit 2
    uint8_t reserved5 : 1;       // Bit 1
    uint8_t uat_initialized : 1; // Bit 0
} byte1_bits;

typedef struct byte2_bits
{

    // Byte 2 (Status byte 2)
    uint8_t timestamp : 1;         // Bit 7 (MS bit)
    uint8_t csa_requested : 1;     // Bit 6
    uint8_t csa_not_available : 1; // Bit 5
    uint8_t reserved4 : 1;         // Bit 4
    uint8_t reserved3 : 1;         // Bit 3
    uint8_t reserved2 : 1;         // Bit 2
    uint8_t reserved1 : 1;         // Bit 1
    uint8_t utc_ok : 1;            // Bit 0
} byte2_bits;
typedef union heartbeat_status_data
{
    uint16_t raw;
    byte1_bits byte1;
    byte2_bits byte2;

} heartbeat_status;

typedef struct gdl90_heartbeat
{
    uint8_t id; // HEARTBEAT_MESSAGE_ID;
    heartbeat_status status;
    uint32_t timestamp;
    uint16_t message_counts;
} gdl90_heartbeat;

/// @brief parse from buffer a heartbeat_message, this will always work if ID is 0
/// @param buffer
/// @param verbose
/// @return gdl90_heartbeat
gdl90_heartbeat parse_heartbeat_message(uint8_t *buffer, int verbose)
{
    gdl90_heartbeat hb_message = {0};
    hb_message.id = buffer[0];

    // Sanity check
    if (hb_message.id != HEARTBEAT_MESSAGE_ID)
    {
        hb_message.id = UNKNOWN_MESSAGE; // Setting only ID, but leaving everything empty
        printf("[parse_heartbeat_message] cant parse buffer to HeartBeat message, wrong ID [expected=%d, got=%d]\n", HEARTBEAT_MESSAGE_ID, hb_message.id);
        return hb_message;
    }

    hb_message.status.raw = ((uint16_t)buffer[2] << 8) | buffer[1];
    hb_message.timestamp = (uint32_t)hb_message.status.byte2.timestamp << 17 | ((uint16_t)buffer[4] << 8) | buffer[3];
    hb_message.message_counts = ((uint16_t)buffer[6] << 8) | buffer[5];
    if (verbose == 1)
    {
        printf("hb_message.timestamp                            =  %d[10]\n", hb_message.timestamp);
        printf("hb_message.status.status_byte1                  =  %b\n", hb_message.status.byte1); // %b not supported in POSIX, I know - It wont go to production this way, I promise
        printf("hb_message.status.status_byte2                  =  %b\n", hb_message.status.byte2);
        printf("hb_message.status.status_byte1.gps_pos_valid    =  %d\n", hb_message.status.byte1.gps_pos_valid);
        printf("hb_message.status.status_byte1.maint_req        =  %d\n", hb_message.status.byte1.maint_req);
        printf("hb_message.status.status_byte1.ident            =  %d\n", hb_message.status.byte1.ident);
        printf("hb_message.status.status_byte1.addr_type        =  %d\n", hb_message.status.byte1.addr_type);
        printf("hb_message.status.status_byte1.gps_batt_low     =  %d\n", hb_message.status.byte1.gps_batt_low);
        printf("hb_message.status.status_byte1.ratcs            =  %d\n", hb_message.status.byte1.ratcs);
        printf("hb_message.status.status_byte1.reserved5        =  %d\n", hb_message.status.byte1.reserved5);
        printf("hb_message.status.status_byte1.uat_initialized  =  %d\n", hb_message.status.byte1.uat_initialized);
        printf("hb_message.status.status_byte2.timestamp        =  %d\n", hb_message.status.byte2.timestamp);
        printf("hb_message.status.status_byte2.csa_requested    =  %d\n", hb_message.status.byte2.csa_requested);
        printf("hb_message.status.status_byte2.csa_not_available=  %d\n", hb_message.status.byte2.csa_not_available);
        printf("hb_message.status.status_byte2.reserved4        =  %d\n", hb_message.status.byte2.reserved4);
        printf("hb_message.status.status_byte2.reserved3        =  %d\n", hb_message.status.byte2.reserved3);
        printf("hb_message.status.status_byte2.reserved2        =  %d\n", hb_message.status.byte2.reserved2);
        printf("hb_message.status.status_byte2.reserved1        =  %d\n", hb_message.status.byte2.reserved1);
        printf("hb_message.status.status_byte2.utc_ok           =  %d\n", hb_message.status.byte2.utc_ok);
        printf("hb_message.status.raw                           =  %b\n", hb_message.status.raw);
        printf("hb_message.status.raw                           =  %x[16]\n", hb_message.status.raw);
        printf("hb_message.message_counts                       =  %d[10]\n", hb_message.message_counts);
    }

    return hb_message;
}
#endif /*GDL90_HEARTBEAT_H*/