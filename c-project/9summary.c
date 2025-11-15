/*
 * OPERATION 9: Summary Function
 * This function displays summary statistics of the student records.
 * The summary includes total number of students, average mark, highest mark, and lowest mark.
*/

#include "student_db.h"

void showSummary(void) {
    if (recordCount == 0) {
        printf("No records loaded. Opening database...\n");
        openDatabase();
        if (recordCount == 0) {
            printf("Still no records found.\n");
            return;
        }
    }

    float total = 0;
    float highest = records[0].mark;
    float lowest = records[0].mark;
    int highIndex = 0, lowIndex = 0;

    for (int i = 0; i < recordCount; i++) {
        total += records[i].mark;
        if (records[i].mark > highest) {
            highest = records[i].mark;
            highIndex = i;
        }
        if (records[i].mark < lowest) {
            lowest = records[i].mark;
            lowIndex = i;
        }
    }

    printf("\n=== Summary Statistics ===\n");
    printf("Total students: %d\n", recordCount);
    printf("Average mark: %.2f\n", total / recordCount);
    printf("Highest mark: %.2f (%s)\n", highest, records[highIndex].name);
    printf("Lowest mark: %.2f (%s)\n", lowest, records[lowIndex].name);

    audit_log("SUMMARY", NULL, NULL, "SUCCESS");
}
