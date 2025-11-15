/*
 * OPERATION 7: Save Function
 * This function saves the student database to the .txt file.
*/

#define _CRT_SECURE_NO_WARNINGS
#include "student_db.h"

void saveDatabase(void) {
    FILE* file;
    int i;

    file = fopen(FILENAME, "w");

    if (file == NULL) {
        printf("Error saving file!\n");
        audit_log("SAVE", NULL, NULL, "FAIL");
        return;
    }
    // write header information
    fprintf(file, "Database Name: Sample-CMS\n");
    fprintf(file, "Authors: Assistant Prof Oran Zane Devilly\n");
    fprintf(file, "\n");
    fprintf(file, "Table Name: StudentRecords\n");
    fprintf(file, "ID\tName\t\tProgramme\t\tMark\n");

    i = 0;
    while (i < recordCount) { // writes each student record
        fprintf(file, "%d\t%-15s\t%-23s\t%.1f\n",
            records[i].id,
            records[i].name,
            records[i].programme,
            records[i].mark);
        i = i + 1;
    }

    fclose(file);
    printf("Database saved successfully.\n");
    audit_log("SAVE", NULL, NULL, "SUCCESS");
}
