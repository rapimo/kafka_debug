#include <stdio.h>
#include <stdlib.h>

#include <librdkafka/rdkafka.h>

#define PARTITION 0
#define TOPIC "retention_test"
#define BATCHSIZE 100

#define KAFKA_MAX_ERR_MSG 255

static rd_kafka_t *      kafka_handle;
static rd_kafka_topic_t *kafka_topic_handle;

static void cleanup();

int
main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage:");
        printf("%s [offset]", argv[0]);
        return 1;
    }

    rd_kafka_topic_conf_t *topic_conf;
    char                   errstr[KAFKA_MAX_ERR_MSG];
    rd_kafka_conf_t *      conf;
    rd_kafka_message_t **  buffer;
    rd_kafka_message_t *   message;
    int                    buffer_count;
    uint64_t               offset = strtoll(argv[1], NULL, 10);
    ;

    buffer = malloc(sizeof(*buffer) * BATCHSIZE);

    conf       = rd_kafka_conf_new();
    topic_conf = rd_kafka_topic_conf_new();

    rd_kafka_topic_conf_set(topic_conf, "auto.offset.reset", "earliest", NULL, 0);

    kafka_handle = rd_kafka_new(RD_KAFKA_CONSUMER, conf, errstr, KAFKA_MAX_ERR_MSG);
    if (kafka_handle != NULL)
    {
        if (rd_kafka_brokers_add(kafka_handle, "localhost:9092") < 1)
        {
            rd_kafka_destroy(kafka_handle);
            printf("No valid brokers specified\n");
            return 1;
        }

        /* Create topic handle */
        kafka_topic_handle = rd_kafka_topic_new(kafka_handle, TOPIC, topic_conf);
        topic_conf         = NULL; /* Now owned by kafka_topic_handle */
    }
    else
    {
        printf("unable to create kafka_handle\n");
        return 1;
    }

    if (rd_kafka_consume_start(kafka_topic_handle, 0, offset) == -1)
    {
        rd_kafka_resp_err_t err = rd_kafka_last_error();
        printf("kafka_fdw: Failed to start consuming: %s\n", rd_kafka_err2str(err));
    }

    printf("START consuming...\n");
    buffer_count = rd_kafka_consume_batch(kafka_topic_handle, PARTITION, 100, buffer, BATCHSIZE);
    printf("GOT %d messages\n", buffer_count);
    if (buffer_count != -1)
    {
        for (int i = 0; i < buffer_count; i++)
        {
            message = buffer[i];
            if (message->err == RD_KAFKA_RESP_ERR__PARTITION_EOF)
            {
                printf("kafka_fdw has reached the end of the queue\n");
                break;
            }
            if (message->err != RD_KAFKA_RESP_ERR_NO_ERROR)
            {
                printf("kafka_fdw got an error %d when fetching a message from queue\n", message->err);
                cleanup();
                return 1;
            }

            printf("OFFSET: %lld PAYLOAD: %s\n", message->offset, message->payload);
            rd_kafka_message_destroy(message);
        }
    }
    else
    {
        printf("%% Error consume_batch: %s\n", rd_kafka_err2str(rd_kafka_last_error()));
    }

    cleanup();
    return 0;
}

static void
cleanup()
{
    if (rd_kafka_consume_stop(kafka_topic_handle, PARTITION) == -1)
    {
        rd_kafka_resp_err_t err = rd_kafka_last_error();
        printf("kafka_fdw: Failed to stop consuming: %s", rd_kafka_err2str(err));
    }
    rd_kafka_topic_destroy(kafka_topic_handle);
    rd_kafka_destroy(kafka_handle);
}