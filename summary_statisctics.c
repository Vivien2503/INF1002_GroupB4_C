#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <direct.h>  // for _getcwd()

#define MAX_RECORDS 100

typedef struct {
    int id;
    char name[50];
    char programme[50];
    float mark;
} StudentRecord;

StudentRecord records[MAX_RECORDS];
int recordCount = 0;

void loadFile(char filename[]) {
    char cwd[256];
    _getcwd(cwd, sizeof(cwd));
    printf("Looking for file in: %s\n", cwd);

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file.\n");
        return;
    }

    char line[256];
    int i;

    for (i = 0; i < 4; i++) {
        fgets(line, sizeof(line), fp);
    }

    while (fgets(line, sizeof(line), fp) != NULL && recordCount < MAX_RECORDS) {
        StudentRecord s;
        if (sscanf(line, "%d\t%[^\t]\t%[^\t]\t%f", &s.id, s.name, s.programme, &s.mark) == 4) {
            records[recordCount] = s;
            recordCount++;
        }
    }

    fclose(fp);
}

void showSummary() {
    if (recordCount == 0) {
        printf("No records found.\n");
        return;
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
}
int main() {
    loadFile("Sample-CMS.txt");
    showSummary();
    return 0;
}