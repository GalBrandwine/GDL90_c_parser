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
void gdl_90_byte_unstuff(const uint8_t *input, size_t input_len, uint8_t *output, size_t *output_len)
{
    size_t out_idx = 0;
    size_t i = 0;

    while (i < input_len)
    {
        if (input[i] == 0x7D)
        {
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
                    // Error handling for unexpected escape sequence
                    fprintf(stderr, "Unexpected escape sequence: 0x%02X\n", input[i]);
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                // Error handling for incomplete escape sequence
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

MESSAGE_TYPES gdl_parse_to_message(uint8_t *buffer_without_flags, int buffer_size)
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
    return UNKNOW;
}

void *message_parser_t(void *arg)
{
    shared_data_t *shared_data = (shared_data_t *)arg;
    // uint8_t local_buffer[MMAP_SIZE];
    // int local_buffer_size = 0;

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
            // usleep(500);
        }

        // Read the data

        uint8_t flag = shared_data->data[0]; // We expect 0x7e at the head of the buffer
        if (flag != 0x7e)
        {
            printf("not synchronized, dropping packets\n"); // for simplicity. Eventually I would like to resync the parser with the stream
                                                            // Lock the mutex
            shared_data->data_ready = 0;
            pthread_mutex_unlock(&shared_data->mutex);
            continue;
        }
        int start_message_index = 1;
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
        uint8_t local_buffer[MMAP_SIZE];
        int local_buffer_size = 0;
        for (size_t i = 0; i <= shared_data->index; i++)
        {
            printf("byte in buffer[%ld]=%x\n", i, shared_data->data[start_message_index + i]);
            local_buffer[i] = shared_data->data[start_message_index + i];
            if (shared_data->data[start_message_index + i] == 0x7e)
            {
                printf("got Control-Escape flag\n");
                local_buffer_size = i;
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
        uint8_t local_buffer_unstuffed[MMAP_SIZE];
        int local_buffer_unstuffed_size = 0;
        gdl_90_byte_unstuff(local_buffer, local_buffer_size, local_buffer_unstuffed, &local_buffer_unstuffed_size);
        printf("after byte stuffing\n");
        print_buffer(local_buffer_unstuffed, local_buffer_unstuffed_size);

        /****************************************************************
         *                                                              *
         * 3. Calculate CRC                                             *
         *                                                              *
         ***************************************************************/
        // uint16_t crc16 = crc16_ccitt(local_buffer_unstuffed, local_buffer_unstuffed_size - 2); // This uses precompiled crc table
        unsigned int crc16 = crcCompute(local_buffer_unstuffed, local_buffer_unstuffed_size - 2);                                                                  // This is the example provided in GDL90 documentation
        uint16_t message_crc16 = (uint16_t)local_buffer_unstuffed[local_buffer_unstuffed_size - 1] << 8 | local_buffer_unstuffed[local_buffer_unstuffed_size - 2]; // Little endian
        if (crc16 != message_crc16)
        {
            printf("crc mismatch!!\n [calculated crc: %d expected_crc=%d]\n", crc16, message_crc16);
        }

        gdl_parse_to_message(local_buffer_unstuffed, local_buffer_unstuffed_size - 2);
        printf("testing after\n");
        print_buffer(local_buffer, local_buffer_size);

        // byte_stuffing_check - wherever a 0x7D or 0x7E byte is found in between the
        // two Flag Bytes, a Control-Escape character is inserted, followed by the original byte XOR’ed
        // with the value 0x20

        // Reset the data_ready flag
        shared_data->data_ready = 0;

        // Unlock the mutex
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
        printf("shutting down\n");
        pthread_cond_signal(&shared_data->cond);
        sleep(1);
    }

    // Wait for the threads to finish
    printf("joining\n");
    pthread_join(thread_id, NULL);

    // Clean up
    munmap(shared_data, MMAP_SIZE);
    unlink("mmapfile");
}

int init_parser()
{
    crcInit();

    int fd;

    // Create a file for memory mapping
    fd = open("mmapfile", O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    // Set the file size
    if (ftruncate(fd, MMAP_SIZE) == -1)
    {
        perror("ftruncate");
        close(fd);
        return EXIT_FAILURE;
    }

    // Map the file to memory
    shared_data = (shared_data_t *)mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_data == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    // Initialize the shared data structure
    pthread_mutex_init(&shared_data->mutex, NULL);
    pthread_cond_init(&shared_data->cond, NULL);
    shared_data->data_ready = 0;

    // Close the file descriptor as it's no longer needed
    close(fd);

    // Create reader threads
    pthread_create(&thread_id, NULL, message_parser_t, shared_data);
    return EXIT_SUCCESS;
}

void gdl_90_parse(uint8_t raw_byte, parser_status *status)
{
    printf("started processing %x\n", raw_byte);
    shared_data->data[shared_data->index++] = raw_byte;
    shared_data->data_ready = 1;

    pthread_cond_signal(&shared_data->cond);
    printf("raw byte added to buffer\n");

    // Here I should check if the message is ready, if so return it to the user
    if (shared_data->message_ready == 1)
    {
        status->status = MESSAGE_READY;
    }
    else
    {
        status->status = PROCESSING;
    }
}
