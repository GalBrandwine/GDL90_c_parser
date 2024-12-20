#include "air_sample_task.h"
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "crc16.h"

pthread_t thread_id;
int keep_running = 1;
int message_parser_still_running = 1;

// Function to perform GDL 90 byte unstuffing
void gdl90_byte_unstuff(const uint8_t *input, size_t input_len, uint8_t *output, size_t *output_len)
{
    size_t out_idx = 0;
    size_t i = 0;

    while (i < input_len)
    {
        if (input[i] == 0x7D)
        {
            printf("found byte stuffing [i=%d]\n", i);
            i++;
            if (i < input_len)
            {
                switch (input[i])
                {
                case 0x5E:
                    output[out_idx++] = 0x7E;
                    break;
                case 0x5D:
                    output[out_idx++] = 0x7D;
                    break;
                case 0x5F:
                    output[out_idx++] = 0x7F;
                    break;
                default:
                    fprintf(stderr, "Unexpected escape sequence: 0x%02X\n", input[i]);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                fprintf(stderr, "Incomplete escape sequence at end of input\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            output[out_idx++] = input[i];
            // print_buffer(output, out_idx);
        }

        i++;
    }
    *output_len = out_idx;
}

void gdl90_parse_to_message(shared_data_s *shared_data, uint8_t *message_buffer, int message_size)
{
    /*
    2.2.2. Message ID
    In order to provision for future inclusion of a data link address field, the Message ID is
    represented by the 7 least-significant bits of the byte immediately following the initial Flag byte.
    In equipment that complies with this version of this interface document, the most significant bit
    of the Message ID will always be ZERO, so that the Message ID can be treated as an 8-bit value
    with a range of 0-127 (decimal). Any messages that have a Message ID outside of this range
    should be discarded.
    */
    uint8_t id = message_buffer[0];
    switch (id)
    {
    case HEARTBEAT_MESSAGE_ID:
        printf("Got a heart bit message - processing...\n");
        shared_data->parser_status->message_ready_type = HEARTBEAT_MESSAGE_ID;
        gdl90_heartbeat hb_message = parse_heartbeat_message(message_buffer, 0);
        if (hb_message.id == UNKNOWN_MESSAGE)
        {
            printf("Failed parsing HEARTBEAT message!\n");
            shared_data->parser_status->status = FAILED;
        }
        else
        {
            if (shared_data->parser_status->callbackMap[HEARTBEAT_MESSAGE_ID])
            {
                shared_data->parser_status->status = MESSAGE_READY;
                shared_data->parser_status->callbackMap[HEARTBEAT_MESSAGE_ID]((void *)&hb_message);
            }
            else
            {
                printf("No user attached to this kinda message\n");
            }
        }

        break;
    case GDL90_AIR_EXTENSION:
    case GDL90_AIR_EXTENSION_1:
        printf("Got a AIR EXTENTION MENAGE - processing...\n");
        shared_data->parser_status->message_ready_type = id;
        parse_gdl90_air_extension_message(message_buffer, shared_data->parser_status, 0);
        break;
    default:
        break;
    }
}

void *message_parser_t(void *arg)
{
    shared_data_s *shared_data = (shared_data_s *)arg;

    while (keep_running)
    {
        // Lock the mutex
        pthread_mutex_lock(&shared_data->mutex);

        // Wait for new data
        while (!shared_data->data_ready)
        {
            pthread_cond_wait(&shared_data->cond, &shared_data->mutex);
            if (!keep_running)
                break;
        }

        // Read the data
        printf("flag_bytes_index=%d index=%d\n", shared_data->flag_bytes_index, shared_data->index);
        uint8_t flag = shared_data->data[shared_data->flag_bytes_index]; // We expect 0x7e at the head of the buffer
        if (flag != 0x7e)
        {
            printf("not synchronized, dropping packets\n"); // for simplicity. Eventually I would like to resync the parser with the stream
                                                            // Lock the mutex
            shared_data->data_ready = 0;
            pthread_mutex_unlock(&shared_data->mutex);
            continue;
        }

        printf("Reader thread received: %x\n", flag);

        if (shared_data->index <= 1)
        {
            printf("not enough data in buffer, dropping\n");
            shared_data->data_ready = 0;
            pthread_mutex_unlock(&shared_data->mutex);
            continue;
        }

        /****************************************************************
         *                                                              *
         * 1. Strip beginning 0x7E and last 0x7E flags from the message *
         *                                                              *
         ***************************************************************/
        uint8_t local_buffer[BUFFER_SIZE];
        int local_buffer_size = 0;
        int local_buffer_index = 0;
        for (size_t i = shared_data->flag_bytes_index; i <= shared_data->index; i++, local_buffer_index++)
        {
            printf("byte in buffer[%ld]=%x\n", i, shared_data->data[i + 1]);
            local_buffer[local_buffer_index] = shared_data->data[i + 1];
            if (shared_data->data[i + 1] == 0x7e)
            {
                printf("got Control-Escape flag\n");
                local_buffer_size = local_buffer_index;
                break;
            }
        }

        if (local_buffer_size == 0)
        {
            printf("couldn't reach a Control-Escape flag, dropping current buffer\n");
            // Reset the data_ready flag
            shared_data->data_ready = 0;
            pthread_mutex_unlock(&shared_data->mutex);
            continue;
        }

        printf("local_buffer before byte_stuffing\n");
        print_buffer(local_buffer, local_buffer_size);

        /****************************************************************
         *                                                              *
         * 2. Byte unstuffing                                           *
         *                                                              *
         ***************************************************************/
        uint8_t local_buffer_unstuffed[BUFFER_SIZE];
        int local_buffer_unstuffed_size = 0;
        gdl90_byte_unstuff(local_buffer, local_buffer_size, local_buffer_unstuffed, &local_buffer_unstuffed_size);
        printf("after byte stuffing\n");
        print_buffer(local_buffer_unstuffed, local_buffer_unstuffed_size);

        /****************************************************************
         *                                                              *
         * 3. Calculate CRC                                             *
         *                                                              *
         ***************************************************************/
        // uint16_t crc16 = crc16_ccitt(local_buffer_unstuffed, local_buffer_unstuffed_size - 2); // This uses precompiled crc table
        unsigned int crc16 = crcCompute(local_buffer_unstuffed, local_buffer_unstuffed_size - 2);                                                                  // This is the example provided in GDL90 documentation
        uint16_t message_crc16 = (uint16_t)local_buffer_unstuffed[local_buffer_unstuffed_size - 1] << 8 | local_buffer_unstuffed[local_buffer_unstuffed_size - 2]; // The FCS is a 16-bit CRC with the least significant byte first.
        if (crc16 != message_crc16)
        {
            printf("crc mismatch!!\n [calculated crc: %d expected_crc=%d]\n", crc16, message_crc16);
            fprintf(stderr, "crc mismatch!!\n [calculated crc: %d expected_crc=%d]\n", crc16, message_crc16);
            // Reset the data_ready flag
            shared_data->data_ready = 0;
            shared_data->parser_status->status = FAILED;
            pthread_mutex_unlock(&shared_data->mutex);
            continue;
        }

        /****************************************************************
         *                                                              *
         * 4. Parse byte_stream to message                              *
         *                                                              *
         ***************************************************************/
        gdl90_parse_to_message(shared_data, local_buffer_unstuffed, local_buffer_unstuffed_size - 2); // minus 2 bytes -  CRC

        // Reset the data_ready flag
        shared_data->data_ready = 0;

        // Resetting flag_bytes_index
        shared_data->flag_bytes_index = shared_data->index;
        pthread_mutex_unlock(&shared_data->mutex);
        usleep(500);
    }
    message_parser_still_running = 0;
}

void shut_down()
{
    printf("shutting down\n");
    keep_running = 0;
    while (message_parser_still_running == 1)
    {
        printf("still shutting down\n");
        pthread_cond_signal(&shared_data.cond);
        sleep(1);
    }

    // Wait for the threads to finish
    printf("joining\n");
    pthread_join(thread_id, NULL);
}

int init_parser()
{
    crcInit();

    // Initialize the shared data structure
    pthread_mutex_init(&shared_data.mutex, NULL);
    pthread_cond_init(&shared_data.cond, NULL);
    shared_data.data_ready = 0;

    // Create reader threads, and house keeping flags
    keep_running = 1;
    message_parser_still_running = 1;
    pthread_create(&thread_id, NULL, message_parser_t, &shared_data);
    return EXIT_SUCCESS;
}

void gdl90_parse(uint8_t raw_byte, parser_status *status)
{
    printf("started processing %x [index=%d]\n", raw_byte, shared_data.index);
    if (raw_byte == 0x7E && status->status != PROCESSING)
    {
        shared_data.flag_bytes_index = shared_data.index;
    }

    status->status = PROCESSING;
    shared_data.data[shared_data.index++] = raw_byte;
    shared_data.data_ready = 1;
    shared_data.parser_status = status;

    pthread_cond_signal(&shared_data.cond);
    printf("raw byte added to buffer\n");
}
