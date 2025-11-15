/*
 * OPERATION 1: Open Function
 * This function opens the student database file, reads the records,
 * and loads them into memory.
*/

#define _CRT_SECURE_NO_WARNINGS
#include "student_db.h"
#include <stdlib.h>

void openDatabase(void) { // open and load database from file
    FILE* fp;
    char line[256];
    int readResult;

    fp = fopen(FILENAME, "r");

    if (fp == NULL) { // error handling for file open
        printf("Error opening file!\n");
        audit_log("OPEN", NULL, NULL, "FAIL");
        return;
    }

    recordCount = 0; 
    fgets(line, sizeof(line), fp);

    while (recordCount < MAX_RECORDS) { // read records until max records reached
        if (fgets(line, sizeof(line), fp) == NULL) {
            break;
        }

        readResult = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%f", // read ID, name, programme, mark
            &records[recordCount].id,
            records[recordCount].name,
            records[recordCount].programme,
            &records[recordCount].mark);

        if (readResult == 4) {
            recordCount = recordCount + 1;
        }
    }

    fclose(fp); // close the file after reading
    index_build(records, recordCount);

    printf("Successfully loaded %d records from '%s'\n\n", recordCount, FILENAME);
    audit_log("OPEN", NULL, NULL, "SUCCESS");
}
