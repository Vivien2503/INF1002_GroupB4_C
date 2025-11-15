/*
 *This is the main header file for the Student Database Management System.
 * It contains the structure definitions and functions used.
*/

#ifndef STUDENT_DB_H
#define STUDENT_DB_H

#include <stdio.h>

#define MAX_RECORDS 100
#define MAX_NAME_LEN 40
#define MAX_PROG_LEN 40
#define FILENAME "Sample-CMS.txt"

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char programme[MAX_PROG_LEN];
    float mark;
} StudentRecord;

// Global database declarations (defined in main.c)
extern StudentRecord records[MAX_RECORDS];
extern int recordCount;

// functions for student database management
void openDatabase(void);
void showAll(void);
void insertRecord(void);
void queryRecord(void);
void updateRecord(void);
void deleteRecord(void);
void saveDatabase(void);
void sortRecords(void);
void showSummary(void);

// audit functions
void audit_open(void);
void audit_close(void);
void audit_log(const char* op,
               const StudentRecord* before_opt,
               const StudentRecord* after_opt,
               const char* status);

// fastlookup index functions
void index_build(const StudentRecord* recs, int count);
int  index_get(int id, int* out_pos);
void index_put(int id, int pos);
void index_rebuild(const StudentRecord* recs, int count);

#endif
