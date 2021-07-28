#ifndef DB_UTILS
#define DB_UTILS

int check_database(char *path, struct sqlite3 *db);

extern struct sqlite3 *sql;

#endif