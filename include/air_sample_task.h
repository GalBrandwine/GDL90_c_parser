#ifndef AIR_SAMPLE_TASK_H
#define AIR_SAMPLE_TASK_H
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
typedef enum message_types
{
    HEARTBEAT,
    UNKNOW

} MESSAGE_TYPES;
typedef enum processing_status
{
    PROCESSING,
    MESSAGE_READY
} STATUS;

typedef struct
{
    STATUS status;
} parser_status;

// Size of the memory-mapped region
// Although GDL 90 has no limitation, I used the embedded SAE J1939 protocol as upper bound messages - 1785 Bytes (https://www.typhoon-hil.com/documentation/typhoon-hil-software-manual/References/j1939_protocol.html#:~:text=J1939%20defines%20the%20maximum%20message,sequence%208%20byte%20size%20messages.)
// + 100 Bytes for the mutexes
#define MMAP_SIZE 2100

static void print_buffer(uint8_t *buffer, int buffer_size)
{
    printf("printing buffer [size=%d]:\n", buffer_size);
    for (size_t i = 0; i < buffer_size; i++)
    {
        printf("%x ", buffer[i]);
    }
    printf("\n");
}

// Shared data structure
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    uint8_t data[MMAP_SIZE - sizeof(pthread_mutex_t) - sizeof(pthread_cond_t)]; // Make sure we're not overflowing memory-mapped region
    int data_ready;
    int message_ready;
    int index;
} shared_data_t;

static shared_data_t *shared_data;

int init_parser();
void shut_down();
void gdl_90_parse(uint8_t raw_byte, parser_status *status);

#endif /*AIR_SAMPLE_TASK_H*/