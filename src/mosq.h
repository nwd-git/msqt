#ifndef MOSQ_UTILS
#define MOSQ_UTILS

extern struct mosquitto *mosq;

struct topic {
    char name[256];
    int qos;
    struct topic *next;
};

extern struct topic *head;

int insertIntoDatabase(char *topic, char *payload);

void messageCallback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

void connectCallback(struct mosquitto *mosq, void *data, int res);

void insert(const char *name, int qos);

int cleanup(struct mosquitto *mosq);

#endif