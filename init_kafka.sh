: ${KAFKA_PRODUCER:="/usr/local/bin/kafka-console-producer"}
: ${KAFKA_TOPICS:="/usr/local/bin/kafka-topics"}
: ${KAFKA_CONFIG:="/usr/local/bin/kafka-configs"}
: ${KAFKA_CONSUMER:="/usr/local/bin/kafka-console-consumer"}

TOPIC=retention_test

# delete topic if it might exist
$KAFKA_TOPICS --zookeeper localhost:2181 --delete --topic $TOPIC

$KAFKA_TOPICS --zookeeper localhost:2181 --create --topic $TOPIC \
              --partitions 1 --replication-factor 1 \
              --config retention.ms=10 \
              --config file.delete.delay.ms=10

$KAFKA_PRODUCER --broker-list localhost:9092 --topic $TOPIC <<-EOF
1 some random message these should be retained
2 some random message these should be retained
3 some random message these should be retained
EOF

sleep 3
$KAFKA_CONFIG --zookeeper localhost:2181 --entity-type topics --alter --delete-config retention.ms --entity-name $TOPIC


$KAFKA_PRODUCER --broker-list localhost:9092 --topic $TOPIC <<-EOF
4 some random message these should be visible at offset >=3
5 some random message these should be visible at offset >=3
6 some random message these should be visible at offset >=3
EOF

# see that only last messages exist
$KAFKA_CONSUMER --bootstrap-server localhost:9092 --topic contrib_regress_retained --from-beginning --timeout-ms 100