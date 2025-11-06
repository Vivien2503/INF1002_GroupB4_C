#include <stdio.h>

// NOTE: This function uses the global variables defined in Operations.c:
// StudentRecord records[MAX_RECORDS];
// int recordCount;

// Function to display summary statistics of all student records
void showSummary() {
    if (recordCount == 0) {
        printf("No records loaded.\n");
        return;
    }

    float total = 0, highest = records[0].mark, lowest = records[0].mark;
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

    float average = total / recordCount;

    printf("\n=== Summary Statistics ===\n");
    printf("Total number of students: %d\n", recordCount);
    printf("Average mark: %.2f\n", average);
    printf("Highest mark: %.2f (%s)\n", highest, records[highIndex].name);
    printf("Lowest mark: %.2f (%s)\n", lowest, records[lowIndex].name);
}
