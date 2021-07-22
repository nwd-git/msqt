#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <signal.h>
#include <uci.h>
#include <string.h>

volatile sig_atomic_t daemonize = 1;

struct config{
    char host[256];
    char topic[256];
    int port;
};

struct topic {
    char name[256];
};

struct topicList {
    struct topic t;
    struct topicList *next;
};

struct topicList *head = NULL;

int getConfigProperty(char *path, char *string);

int getConfig(struct config *conf);

void messageCallback (struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

void connectCallback (struct mosquitto *mosq, void *data, int res);

int connectMosq(struct config *conf);

int main(void)
{
    int rc =0;
    struct config conf = {0};  
    getConfig(&conf);
    connectMosq(&conf);   
    return rc;
}

int getConfigProperty(char *path, char *string)
{
    int rc = 0;
    struct uci_context *c;
    struct uci_ptr ptr;

    c = uci_alloc_context ();
    if (uci_lookup_ptr (c, &ptr, path, false) != UCI_OK)
    {
        uci_perror (c, "get_config_entry Error");
        rc = -1;
        return rc;
    }
    strcpy(string, ptr.o->v.string);
    uci_free_context(c);
    return rc;
}

int getConfig(struct config *conf)
{
    int rc = 0;
    char port[256];
    char hostPath[] = "msqt.msqt_sct.host";
    char topicPath[] = "msqt.msqt_sct.topic";
    char portPath[] = "msqt.msqt_sct.port";

    getConfigProperty(hostPath, conf->host);
    getConfigProperty(topicPath, conf->topic);
    getConfigProperty(portPath, port);

    conf->port = atoi(port);

    return rc; 
}

void messageCallback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
    fprintf(stdout, "Got message \"%s\" from %s \n",(char *) message->payload,message->topic);
}

void connectCallback(struct mosquitto *mosq, void *data, int res)
{
    if(res != 0){
        fprintf(stderr, "Couldn't connect, rc=%d\n", res);
    } else fprintf(stdout, "Successfully connected\n");
    
}

void insertFirst(struct topic data) {
   struct topicList *link = (struct topicList*) malloc(sizeof(struct topicList));
	
   link->t = data;
	
   link->next = head;

   head = link;
}

void printList() {
   struct topicList *ptr = head;
	
   while(ptr != NULL) {
      printf(" ->%s",ptr->t.name);
      ptr = ptr->next;
   }
}

int connectMosq(struct config *conf)
{
    struct mosquitto *mosq;
    int rc = 0;
    struct topic t = {.name = "/tmp"};
    struct topic tt = {.name = "/tmp1"};
    insertFirst(t);
    insertFirst(tt);
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
    while(etalol != NULL){
        mosquitto_subscribe(mosq, NULL, etalol->t.name, 0);
        etalol = etalol->next;
    }
    while(daemonize){
        rc = mosquitto_loop(mosq, -1, 1);
        if(rc){
            printf("Connection error!\n");
            sleep(10);
            mosquitto_reconnect(mosq);
        } 
    }
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup(); 
    return rc;
}