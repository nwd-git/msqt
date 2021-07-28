#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <signal.h>
#include <string.h>
#include <sqlite3.h>
#include "db_utils.h"
#include "config_utils.h"
#include "mosq.h"

volatile sig_atomic_t daemonize = 1;
struct config conf;
struct mosquitto *mosq;
struct sqlite3 *sql;
struct topic *head;

int connectMosq(struct config *conf);

int main(void)
{
    int rc =0;  
    int counter = 0;
    rc = check_database("/usr/share/data.db", sql);
    if(rc != 0){
        return rc;
    }
    rc = sqlite3_open("/usr/share/data.db", &sql);
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
    while(daemonize){
        rc = mosquitto_loop(mosq, -1, 1);
        if(rc && counter != 10){
            printf("Connection error!\n");
            sleep(10);
            rc = mosquitto_reconnect(mosq);
            counter++;
        } else if(counter > 10){
            cleanup(mosq);
        }
    }
    sqlite3_close(sql);
    cleanup(mosq);
    return rc;
}