#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <signal.h>
#include <string.h>
#include <sqlite3.h>
#include "db_utils.h"
#include "config_utils.h"

volatile sig_atomic_t daemonize = 1;

struct sqlite3 *sql = NULL;

struct config conf;

struct topic {
    char name[256];
    int qos;
    struct topic *next;
};

struct topic *head = NULL;

void insert(char *name, int qos);

void messageCallback (struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

void connectCallback (struct mosquitto *mosq, void *data, int res);

int connectMosq(struct config *conf);

int createQuery(struct topic *topic, char *payload);

int main(void)
{
    int rc =0;  

    rc = check_database("/tmp/data.db", sql);
    if(rc != 0){
        return rc;
    }
    rc = sqlite3_open("/tmp/data.db", &sql);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Couldn't open database");
    }
    rc = loadConfigurations(&conf);
    if(rc != 0){
        fprintf(stderr, "Couldn't load configuration");
        return rc;
    }
    rc = connectMosq(&conf);
    if(rc != 0){
        fprintf(stderr,"Something went wrong");
        return rc;
    }
    sqlite3_close(sql);
    return rc;
}

void messageCallback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
    fprintf(stdout, "Got message from %s \n",message->topic);
    createQuery(message->topic,(char *) message->payload);
}

void connectCallback(struct mosquitto *mosq, void *data, int res)
{
    fprintf(stdout, "Successfully connected\n");  
}

void insert(char *name, int qos)
{
    struct topic *link = (struct topic*) malloc(sizeof(struct topic));
	
    strncpy(link->name, name, strlen(name) + 1 );
    link->qos = qos;
    link->next = head;
    head = link;
}

int createQuery(struct topic *topic, char *payload)
{
    int rc = 0;
    char *query = sqlite3_mprintf("insert into Messages(Topic, Message) values ('%q', '%q');", topic->name, payload);
    sqlite3_exec(sql,query,NULL, 0, NULL);
    return rc;
}

int connectMosq(struct config *conf)
{
    struct mosquitto *mosq;
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
        goto cleanup;
    }
    while(current != NULL){
        rc = mosquitto_subscribe(mosq, NULL, current->name, current->qos);
        if(rc != MOSQ_ERR_SUCCESS){
            fprintf(stderr, "Couldn't subscribe to %s", current->name);
        }
        current = current->next;
    }
    while(daemonize){
        rc = mosquitto_loop(mosq, -1, 1);
        if(rc && counter != 10){
            printf("Connection error!\n");
            sleep(10);
            rc = mosquitto_reconnect(mosq);
            counter++;
        } else if(counter > 10){
            goto cleanup;
        }
    }
    cleanup:
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup(); 
    return rc;
}