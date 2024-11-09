#ifndef AIR_SAMPLE_TASK_H
#define AIR_SAMPLE_TASK_H
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include "gdl90_heartbeat.h"
#include "gdl90_air_extension.h"



// Shared data structure
typedef struct shared_data_s
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    uint8_t data[BUFFER_SIZE - sizeof(pthread_mutex_t) - sizeof(pthread_cond_t)]; // Make sure we're not overflowing memory-mapped region
    int data_ready;
    int index;
    int flag_bytes_index;
    parser_status *parser_status;
} shared_data_s;

static shared_data_s shared_data;

/// @brief The User must call this before starting parsing incoming bytes
/// @return
int init_parser();
void shut_down();

/// @brief The GDL90 parser
/// An non-blocking bytes parser, that support callback attaching when message of interest has arrive completely
/// @param raw_byte - the next byte to parse
/// @param status - a context containing parsing statuses, and possible callback attached
void gdl90_parse(uint8_t raw_byte, parser_status *status);

#endif /*AIR_SAMPLE_TASK_H*/