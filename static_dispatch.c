typedef struct proto_subscriber_S {
    void (*notify)(struct proto_subscriber_S *self, int channel, void *message);
    void *data;
} proto_subscriber;
