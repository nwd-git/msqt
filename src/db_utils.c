#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sqlite3.h>

struct sqlite3 *sql = NULL;

static int create_database(char *path)
{
    int rc = 0;
    FILE *fp;
    fp = fopen(path,"w");
    if(fp == NULL){
        rc = -1;
        return rc;
    }
    fclose(fp);
    return rc;
}

static int database_exists(char *path)
{
    int rc = 0;
    struct stat buf;
    rc = stat(path, &buf);
    return rc;
}

int check_database(char *path, struct sqlite3 *db)
{
    int rc = 0;
    char *err = NULL;
    rc = database_exists(path);
    if(rc == 0){
        fprintf(stderr,"Database file already exists\n");
        return rc;
    }
    rc = create_database(path);
    if( rc != 0){
        fprintf(stderr, "Couldn't create database file\n");
        return rc;
    }
    char *query = "CREATE TABLE Messages ( \
                            id integer primary key autoincrement not null,\
                            Topic varchar(250),\
                            Message varchar(250),\
                            Time timestamp default (datetime('now', 'localtime')) not null);";
    
    rc = sqlite3_open(path,&db);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Something went wrong trying to open database\n");
        sqlite3_close(db);
        return rc;
    }
    rc = sqlite3_exec(db, query, 0, NULL, &err);
    if(rc != SQLITE_OK){
        fprintf(stderr, "Couldn't create table %s\n", err);
        sqlite3_close(db);
        return rc;
    }
    fprintf(stdout, "Successfully created table\n");
    sqlite3_close(db);
    return rc;
}
