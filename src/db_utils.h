#ifndef DB_UTILS
#define DB_UTILS

int check_database(char *path, struct sqlite3 *db);

int insertToDB(struct topic *topic, char *payload);

#endif