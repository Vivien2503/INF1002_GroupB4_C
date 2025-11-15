/*
 * OPERATION 3: Insert Function
 * This function inserts a new student record into the database.
*/

#define _CRT_SECURE_NO_WARNINGS
#include "student_db.h"
#include <stdlib.h>

void insertRecord(void) {
    int newId;
    int i;
    int duplicate;
    int pos;

    printf("Enter student ID: ");
    scanf("%d", &newId);
    getchar();

    duplicate = 0;

    if (index_get(newId, &pos)) {
        duplicate = 1;
    } else {
        i = 0;
        while (i < recordCount) {
            if (records[i].id == newId) {
                duplicate = 1;
                break;
            }
            i = i + 1;
        }
    }

    if (duplicate == 1) { // error handling for duplicate ID
        printf("Error: Student ID already exists. Insertion cancelled.\n");
        audit_log("INSERT", NULL, NULL, "FAIL(DUPLICATE)");
        return;
    }

    records[recordCount].id = newId;
    // insert name
    printf("Enter name: ");
    fgets(records[recordCount].name, MAX_NAME_LEN, stdin);

    i = 0;
    while (records[recordCount].name[i] != '\0') {
        if (records[recordCount].name[i] == '\n') {
            records[recordCount].name[i] = '\0';
            break;
        }
        i = i + 1;
    }
    // insert programme
    printf("Enter programme: ");
    fgets(records[recordCount].programme, MAX_PROG_LEN, stdin);

    i = 0;
    while (records[recordCount].programme[i] != '\0') {
        if (records[recordCount].programme[i] == '\n') {
            records[recordCount].programme[i] = '\0';
            break;
        }
        i = i + 1;
    }
    // insert mark
    printf("Enter mark: ");
    scanf("%f", &records[recordCount].mark);

    index_put(newId, recordCount);
    audit_log("INSERT", NULL, &records[recordCount], "SUCCESS");

    recordCount = recordCount + 1;
    printf("Record added successfully!\n");
}
