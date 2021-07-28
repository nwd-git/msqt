#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <signal.h>
#include <string.h>
#include <sqlite3.h>
#include "db_utils.h"
#include "config_utils.h"
#include "mosq.h"

int insertIntoDatabase(char *topic, char *payload)
{
    int rc = 0;
    char *query = sqlite3_mprintf("insert into Messages(Topic, Message) values ('%q', '%q');", topic, payload);
    sqlite3_exec(sql,query,NULL, 0, NULL);
    sqlite3_free(query);
    return rc;
}

void messageCallback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
    fprintf(stdout, "Got message from %s \n",message->topic);
    insertIntoDatabase(message->topic,(char *) message->payload);
}

void connectCallback(struct mosquitto *mosq, void *data, int res)
{
    fprintf(stdout, "Successfully connected\n");  
}

void insert(const char *name, int qos)
{
    struct topic *link = (struct topic*) malloc(sizeof(struct topic));
	
    strncpy(link->name, name, strlen(name) + 1 );
    link->qos = qos;
    link->next = head;
    head = link;
}


int connectMosq(struct config *conf)
{
    int rc = 0;
    int counter = 0;
    struct topic *current = head;

    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);
    if(!mosq){
        rc = -1;
        return rc;
    }

    mosquitto_connect_callback_set(mosq, connectCallback);
    mosquitto_message_callback_set(mosq, messageCallback);

    rc = mosquitto_connect(mosq, conf->host, conf->port, 60);
    if(rc != MOSQ_ERR_SUCCESS){
        fprintf(stderr, "Couldn't connect to Mosquitto");
        cleanup(mosq);
    }
    while(current != NULL){
        rc = mosquitto_subscribe(mosq, NULL, current->name, current->qos);
        if(rc != MOSQ_ERR_SUCCESS){
            fprintf(stderr, "Couldn't subscribe to %s", current->name);
        }
        current = current->next;
    }
    return rc;
}

int cleanup(struct mosquitto *mosq)
{
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
}