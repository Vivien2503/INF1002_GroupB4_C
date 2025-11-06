#include <stdio.h>
#include <string.h>

#define MAX_RECORDS 100

typedef struct {
    int id;
    char name[50];
    char programme[50];
    float mark;
} StudentRecord;

StudentRecord records[MAX_RECORDS];
int recordCount = 0;

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

int main() {
    // Sample test data
    records[0] = (StudentRecord){ 2301234, "Joshua Chen", "Software Engineering", 70.5 };
    records[1] = (StudentRecord){ 2201234, "Isaac Teo", "Computer Science", 63.4 };
    records[2] = (StudentRecord){ 2304567, "John Levoy", "Digital Supply Chain", 85.9 };
    recordCount = 3;

    showSummary();
    return 0;
}
