# kafka_debug

This is a minimal test case for
https://github.com/edenhill/librdkafka/issues/1544

Setup is done in `init_kafka.sh` which needs the ENV variables

* KAFKA_PRODUCER pointing to the `kafka-console-producer.sh` that ships with kafka
* KAFKA_TOPICS pointing to the `kafka-topics.sh` that ships with kafka
* KAFKA_CONFIG pointing to the `kafka-configs.sh` that ships with kafka
* KAFKA_CONSUMER pointing to the `kafka-console-consumer.sh` that ships with kafka

and some broker config change:

```
delete.topic.enable=true
log.retention.check.interval.ms=6000
```

It creates a topic `retention_test` with minimal retention of 10ms.
adds 3 messages
resets the retention
adds another 3 messages

The executable kafka_test proves that any offset < 3 doesn't read any messages 
although `auto.offset.reset` is set to `earliest`.
