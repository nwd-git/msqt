#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <signal.h>
#include <uci.h>
#include <string.h>

volatile sig_atomic_t daemonize = 1;

struct config{
    char host[256];
    int port;
};

struct topic {
    char name[256];
    int qos;
};

struct topicList {
    struct topic t;
    struct topicList *next;
};

struct topicList *head = NULL;

int getConfigs(struct config *conf);

void insertFirst(struct topic data);

int getConfigProperty(struct uci_context *ctx, struct uci_section *s,struct config *conf);

void messageCallback (struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

void connectCallback (struct mosquitto *mosq, void *data, int res);

int connectMosq(struct config *conf);

int main(void)
{
    int rc =0;
    struct config conf = {0};
    getConfigs(&conf);
    connectMosq(&conf); 
    return rc;
}

int getConfigs(struct config *conf)
{
    int rc = 0;
    struct uci_context *c;
    struct uci_package *pkg = NULL;
    struct uci_element *e;
    struct uci_element *i;
    struct uci_ptr ptr;

    c = uci_alloc_context ();
    if (uci_load(c,"msqt", &pkg) != UCI_OK)
    {
        uci_perror (c, "get_config_entry Error");
        rc = -1;
        return rc;
    }
    uci_foreach_element(&pkg->sections,e){
        struct uci_section *section = uci_to_section(e);
        getConfigProperty(c,section,conf);
    }
    uci_free_context(c);
    return rc;
}

int getConfigProperty(struct uci_context *ctx, struct uci_section *s, struct config *conf)
{
    int rc = 0;
    
    if(strcmp(s->type, "topic") == 0){
        const char *name = uci_lookup_option_string(ctx,s,"name");
        const char *qos = uci_lookup_option_string(ctx,s,"qos");
        struct topic t = {0};
        strcpy(t.name,name);
        if(qos != NULL){
            t.qos = atoi(qos);  
        } else{
            t.qos =0;
        }
        insertFirst(t);
    } else if(strcmp(s->type, "settings") == 0){
        const char *host = uci_lookup_option_string(ctx,s,"host");
        const char *port = uci_lookup_option_string(ctx,s,"port");
        conf->port = atoi(port);
        strcpy(conf->host,host);
    }

    return rc; 
}

void messageCallback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
    fprintf(stdout, "Got message \"%s\" from %s \n",(char *) message->payload,message->topic);
}

void connectCallback(struct mosquitto *mosq, void *data, int res)
{
    fprintf(stdout, "Successfully connected\n");
    
}

void insertFirst(struct topic data) {
   struct topicList *link = (struct topicList*) malloc(sizeof(struct topicList));
	
   link->t = data;
	
   link->next = head;

   head = link;
}

int connectMosq(struct config *conf)
{
    struct mosquitto *mosq;
    int rc = 0;
    int counter = 0;
    struct topicList *etalol = head;

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
        goto cleanup;
    }
    while(etalol != NULL){
        rc = mosquitto_subscribe(mosq, NULL, etalol->t.name, etalol->t.qos);
        if(rc != MOSQ_ERR_SUCCESS){
            fprintf(stdout, "Couldn't subscribe to %s", etalol->t.name);
        }
        etalol = etalol->next;
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