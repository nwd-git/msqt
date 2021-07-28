#include <stdio.h>
#include <stdlib.h>
#include <uci.h>
#include <string.h>
#include "config_utils.h"
#include "mosq.h"

static int loadConfigProperty(struct uci_context *ctx, struct uci_section *s, struct config *conf)
{
    int rc = 0;
    if(strcmp(s->type, "topic") == 0){
        int qos;
        const char *name = uci_lookup_option_string(ctx,s,"name");
        const char *qosString = uci_lookup_option_string(ctx,s,"qos");
        if(qosString != NULL){
            qos = atoi(qosString);
        }else{
            qos = 1;
        }
        insert(name,qos);
    } else if(strcmp(s->type, "settings") == 0){
        const char *host = uci_lookup_option_string(ctx,s,"host");
        const char *port = uci_lookup_option_string(ctx,s,"port");
        conf->port = atoi(port);
        strcpy(conf->host,host);
    }

    return rc; 
}

int loadConfigurations(struct config *conf)
{
    int rc = 0;
    struct uci_context *c;
    struct uci_package *pkg = NULL;
    struct uci_element *e;
    c = uci_alloc_context();
    if (uci_load(c,"msqt", &pkg) != UCI_OK){
        uci_perror (c, "get_config_entry Error");
        rc = -1;
        return rc;
    }
    uci_foreach_element(&pkg->sections,e){
        struct uci_section *section = uci_to_section(e);
        loadConfigProperty(c,section,conf);
    }
    uci_free_context(c);
    return rc;
}

