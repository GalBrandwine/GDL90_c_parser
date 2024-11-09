#ifndef COMMON_H
#define COMMON_H

/******************
 * GDL message ID's
 *******************/
// #define HEARTBEAT_MESSAGE_ID 0
// #define GDL90_AIR_EXTENSION 0x40
#define GDL90_AIR_EXTENSION_1 0x41
#define UNKNOWN_MESSAGE 0xFF

// Size of the memory-mapped region
// Although GDL 90 has no limitation, I used the embedded SAE J1939 protocol as upper bound messages - 1785 Bytes (https://www.typhoon-hil.com/documentation/typhoon-hil-software-manual/References/j1939_protocol.html#:~:text=J1939%20defines%20the%20maximum%20message,sequence%208%20byte%20size%20messages.)
// + 100 Bytes for the mutexes
#define BUFFER_SIZE 2100
#define J1939_PGN_BUFFER_SIZE 1785

typedef enum message_types
{
    HEARTBEAT_MESSAGE_ID = 0,
    GDL90_AIR_EXTENSION = 0x40,
    // UNKNOW,
    MESSAGE_COUNT // This should always be the last element to get the count of enum types

} MESSAGE_TYPES;

typedef enum processing_status
{
    PROCESSING,
    FAILED,
    MESSAGE_READY
} STATUS;

// Define a function type that returns void and accepts a void* argument
typedef void (*message_ready_callback)(void *);

typedef struct parser_status
{
    STATUS status;
    MESSAGE_TYPES message_ready_type;
    message_ready_callback callbackMap[MESSAGE_COUNT];
} parser_status;

/// @brief Helper function
/// @param buffer
/// @param buffer_size
static void print_buffer(uint8_t *buffer, int buffer_size)
{
    printf("printing buffer [size=%d]:\n", buffer_size);
    for (size_t i = 0; i < buffer_size; i++)
    {
        printf("%x ", buffer[i]);
    }
    printf("\n");
}

enum q_notation
{
    /// @brief represents a signed number in range [-1; +1] and resolution of 1/511=0.001956947.
    /// The number occupies 10 bits. A number of -0.5 will be mapped to a -255 decimal number or 1100000001 two's complement binary.
    Q9 = 10,

    /// @brief represents a signed number in range [0; 1] and resolution of 1/511=0.001956947.
    /// The number occupies 9 bits. A number of 0.5 will be mapped to a 255 decimal number or 011111111 two's complement binary.
    UQ9 = 9,

    /// @brief represents a number ranging from [-9.81; +9.81] and resolution of 9.81/511=0.019197652.
    /// A number of 4 will be mapped to a 208 decimal number or 0011010000 two's complement binary.
    Q9_9_81 = 10,

    /// @brief  represents a number ranging from [-90; +90] and resolution of 30/511=0.05870841487.
    Q9_30 = 10,

    /// @brief  represents a number ranging from [-90; +90] and resolution of 90/511=0.17612524461.
    Q9_90 = 10
};

// Function to convert a Q9/90 fixed-point representation to a floating-point value
static float q9_90ToFloat(int16_t q9_90Value)
{
    // Scale the value back by dividing by 64 (2^6)
    return q9_90Value / 64.0f;
}

// Function to extract 10 bits from an array of bytes
static uint16_t extract10Bits(const uint8_t *data, uint16_t bitPosition)
{
    if (data == NULL)
    {
        return 0;
    }

    // Calculate the byte index and the bit offset within the byte
    uint16_t byteIndex = bitPosition / 8;
    uint8_t bitOffset = bitPosition % 8;

    // Extract the next 10 bits starting from the given bit position
    uint16_t extractedBits = 0;

    // Read enough bytes to cover the 10-bit span
    // This might cover up to 3 bytes if the 10 bits span across byte boundaries
    for (uint8_t i = 0; i < 3; i++)
    {
        extractedBits |= ((uint16_t)data[byteIndex + i]) << (8 * i);
    }

    // Right shift to discard the bits before the bitOffset
    extractedBits >>= bitOffset;

    // Mask to keep only the 10 bits we are interested in
    extractedBits &= 0x03FF;

    return extractedBits;
}

#endif /*COMMON_H*/