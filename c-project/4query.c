/*
 * OPERATION 4: Query Function
 * This function searches for a student record by ID and displays it if found.
*/

#define _CRT_SECURE_NO_WARNINGS
#include "student_db.h"

void queryRecord(void) {
    int searchId;
    int i;
    int found;
    int pos;

    printf("Enter student ID to search: "); // prompt for student ID
    scanf("%d", &searchId);

    if (index_get(searchId, &pos)) {
        printf("\nFound Record:\n");
        printf("ID: %d\nName: %s\nProgramme: %s\nMark: %.2f\n",
            records[pos].id, records[pos].name,
            records[pos].programme, records[pos].mark);
        audit_log("QUERY", NULL, &records[pos], "FOUND");
        return;
    }

    found = 0;
    i = 0;
    while (i < recordCount) { // search through records
        if (records[i].id == searchId) {
            printf("\nFound Record:\n");
            printf("ID: %d\nName: %s\nProgramme: %s\nMark: %.2f\n",
                records[i].id, records[i].name,
                records[i].programme, records[i].mark);
            found = 1;
            audit_log("QUERY", NULL, &records[i], "FOUND");
            break;
        }
        i = i + 1;
    }

    if (found == 0) { // error if record is not found
        printf("Record not found.\n");
        audit_log("QUERY", NULL, NULL, "NOT_FOUND");
    }
}
