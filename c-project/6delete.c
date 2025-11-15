/*
 * OPERATION 6: Delete Function
 * This function deletes a student record by ID.
*/

#define _CRT_SECURE_NO_WARNINGS
#include "student_db.h"

void deleteRecord(void) {
    int searchId;
    int i;
    int j;
    int found;
    char confirm;
    int pos;

    printf("Enter student ID to delete: "); // prompt for student ID
    scanf("%d", &searchId);

    found = 0;
    if (index_get(searchId, &pos)) {
        found = 1;
        i = pos;
    } else {
        i = 0;
        while (i < recordCount) {
            if (records[i].id == searchId) {
                found = 1;
                break;
            }
            i = i + 1;
        }
    }

    if (found == 0) {
        printf("Record not found.\n"); // error if record not found
        return;
    }

    printf("Are you sure you want to delete this record? (y/n): "); // prompt for confirmation
    scanf(" %c", &confirm);

    if (confirm == 'y' || confirm == 'Y') { // deletion confirmation
        audit_log("DELETE", &records[i], NULL, "SUCCESS");

        j = i;
        while (j < recordCount - 1) {
            records[j] = records[j + 1];
            j = j + 1;
        }
        recordCount = recordCount - 1;

        index_rebuild(records, recordCount);

        printf("Record deleted successfully.\n");
    }
    else {
        printf("Deletion cancelled.\n");
    }
}
