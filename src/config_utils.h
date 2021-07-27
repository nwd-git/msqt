#ifndef CONFIG_UTILS
#define CONFIG_UTILS


struct config{
    char host[256];
    int port;
};

extern struct config conf;
int loadConfigurations(struct config *conf);

#endif