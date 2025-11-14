#define _CRT_SECURE_NO_WARNINGS
/*
OPERATIONS FILE (WITH ENHANCEMENTS)
 Purpose: Student Database Management System that is able to carry out 7 basic operations

 Description:
 - Loads the Database from a text file in tsv format and displays it in a formatted table
    - Allows user to Insert, Query, Update, Delete records
    - Saves the modified database back to the text file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_RECORDS 100
#define MAX_NAME_LEN 40
#define MAX_PROG_LEN 40
#define FILENAME "Sample-CMS.txt" // to load and save database called Sample-CMS.txt

typedef struct { // database stuff, struct helps to group different data types
    int id;
    char name[MAX_NAME_LEN];
    char programme[MAX_PROG_LEN];
    float mark;
} StudentRecord;

StudentRecord records[MAX_RECORDS];
int recordCount = 0; // records tracker

void openDatabase();
void showAll();
void insertRecord();
void queryRecord();
void updateRecord();
void deleteRecord();
void saveDatabase();
void sortRecords();
void showSummary();


void showMenu() {
    printf("\n=== Student Database Management System ===\n");
    printf("1. Open Database\n");
    printf("2. Show All Records\n");
    printf("3. Insert Record\n");
    printf("4. Query Record\n");
    printf("5. Update Record\n");
    printf("6. Delete Record\n");
    printf("7. Save Database\n");
    printf("8. Sort Records\n");
    printf("9. Show Summary Statistics\n");
    printf("Enter your choice (1-9): ");
}

int main() {
    int choice;

    while (1) { // (1) for infinitely looping menu
        showMenu();
        scanf("%d", &choice);
        getchar();

        if (choice == 1) {
            openDatabase();
        }
        else if (choice == 2) {
            showAll();
        }
        else if (choice == 3) {
            insertRecord();
        }
        else if (choice == 4) {
            queryRecord();
        }
        else if (choice == 5) {
            updateRecord();
        }
        else if (choice == 6) {
            deleteRecord();
        }
        else if (choice == 7) {
            saveDatabase();
        }
        else if (choice == 8) {
            sortRecords();
        }
        else if (choice == 9) {
            showSummary();
        }
        else {
            printf("Invalid choice! Please enter a number between 1 and 8.\n");
        }
    }
    return 0;
}

// OPERATION 1 OPEN DATABASE -> Opens the database file and loads the records
void openDatabase() {
    FILE* fp;
    char line[256];
    int readResult;

    fp = fopen(FILENAME, "r"); // file pointer

    if (fp == NULL) {
        printf("Error opening file!\n"); // error message
        return;
    }

    recordCount = 0; // clears previous records first before loading

    fgets(line, sizeof(line), fp); // skips the header

    while (recordCount < MAX_RECORDS) { //loop continues while max records not reached
        if (fgets(line, sizeof(line), fp) == NULL) {
            break;
        }

        readResult = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%f", // reads the record fields in the order (id>name>prog>mark)
            &records[recordCount].id,
            records[recordCount].name,
            records[recordCount].programme,
            &records[recordCount].mark);

        if (readResult == 4) { // ensures that all fields are properly read (the 4 variables)
            recordCount = recordCount + 1;
        }
    }

    fclose(fp);
    printf("Successfully loaded %d records from '%s'\n\n", recordCount, FILENAME);
}

// OPERATION 2 SHOW ALL -> Displays the student records in a formatted table
void showAll() {
    int i;

    printf("\nID\tName\t\tProgramme\t\tMark\n");
    printf("----------------------------------------\n");

    i = 0;
    while (i < recordCount) {
        printf("%d\t%-15s\t%-25s\t%.1f\n",
            records[i].id, records[i].name,
            records[i].programme, records[i].mark);
        i = i + 1;
    }
}

// OPERATION 3 INSERT RECORD -> Allows user to add new student records
void insertRecord() {
    int newId;
    int i;
    int j;
    int duplicate;

    printf("Enter student ID: ");
    scanf("%d", &newId);
    getchar();

    duplicate = 0;     // check for duplicate ID
    i = 0;
    while (i < recordCount) {
        if (records[i].id == newId) {
            duplicate = 1;
            break;
        }
        i = i + 1;
    }

    if (duplicate == 1) { // exit and back to menu if duplicate found
        printf("Error: Student ID already exists. Insertion cancelled.\n");
        return;
    }

    records[recordCount].id = newId; // stores new ID to next slot

    printf("Enter name: ");
    fgets(records[recordCount].name, MAX_NAME_LEN, stdin);

    i = 0; // removal of newline character when user presses enter
    while (records[recordCount].name[i] != '\0') {
        if (records[recordCount].name[i] == '\n') {
            records[recordCount].name[i] = '\0';
            break;
        }
        i = i + 1;
    }

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

    printf("Enter mark: ");
    scanf("%f", &records[recordCount].mark);

    recordCount = recordCount + 1; // +1 record count if successful
    printf("Record added successfully!\n");
}

// OPERATION 4 QUERY RECORD -> Allows the user to search for a student record by ID
void queryRecord() {
    int searchId;
    int i;
    int found;

    printf("Enter student ID to search: ");
    scanf("%d", &searchId);

    found = 0;
    i = 0;
    while (i < recordCount) { // loop to find matching ID
        if (records[i].id == searchId) {
            printf("\nFound Record:\n");
            printf("ID: %d\nName: %s\nProgramme: %s\nMark: %.2f\n",
                records[i].id, records[i].name,
                records[i].programme, records[i].mark);
            found = 1;
            break;
        }
        i = i + 1;
    }

    if (found == 0) {
        printf("Record not found.\n");
    }
}

// OPERATION 5 UPDATE RECORD -> Allows the user to update a student record
void updateRecord() {
    int searchId;
    int i;
    int j;
    int found;
    char input[MAX_NAME_LEN];
    float newMark;

    printf("Enter student ID to update: ");
    scanf("%d", &searchId);
    getchar();

    found = 0;
    i = 0;
    while (i < recordCount) {
        if (records[i].id == searchId) {
            found = 1;

            printf("Enter new name (or press enter to skip): ");
            fgets(input, MAX_NAME_LEN, stdin);

            if (input[0] != '\n') { //if user did not press enter, update name
                j = 0;
                while (input[j] != '\0') { //if user pressed enter, remove newline
                    if (input[j] == '\n') {
                        input[j] = '\0';
                        break;
                    }
                    j = j + 1;
                }

                j = 0; // copys the new string to the record
                while (input[j] != '\0') {
                    records[i].name[j] = input[j];
                    j = j + 1;
                }
                records[i].name[j] = '\0';
            }

            printf("Enter new programme (or press enter to skip): ");
            fgets(input, MAX_PROG_LEN, stdin);

            if (input[0] != '\n') {
                j = 0;
                while (input[j] != '\0') {
                    if (input[j] == '\n') {
                        input[j] = '\0';
                        break;
                    }
                    j = j + 1;
                }

                j = 0;
                while (input[j] != '\0') {
                    records[i].programme[j] = input[j];
                    j = j + 1;
                }
                records[i].programme[j] = '\0';
            }

            printf("Enter new mark (or -1 to skip): ");
            scanf("%f", &newMark);

            if (newMark >= 0) {
                records[i].mark = newMark;
            }

            printf("Record updated successfully.\n");
            break;
        }
        i = i + 1;
    }

    if (found == 0) {
        printf("Record not found.\n");
    }
}

// OPERATION 6 DELETE RECORD -> Allows the user to delete a student record through ID
void deleteRecord() {
    int searchId;
    int i;
    int j;
    int found;
    char confirm;

    printf("Enter student ID to delete: ");
    scanf("%d", &searchId);

    found = 0;
    i = 0;
    while (i < recordCount) {
        if (records[i].id == searchId) { // if ID is found:
            found = 1;

            printf("Are you sure you want to delete this record? (y/n): "); // delete confirmation
            scanf(" %c", &confirm);

            if (confirm == 'y' || confirm == 'Y') {
                j = i;
                while (j < recordCount - 1) { // shift records after deletion
                    records[j] = records[j + 1];
                    j = j + 1;
                }
                recordCount = recordCount - 1;
                printf("Record deleted successfully.\n");
            }
            else {
                printf("Deletion cancelled.\n");
            }
            break;
        }
        i = i + 1;
    }

    if (found == 0) {
        printf("Record not found.\n");
    }
}

// OPERATION 7 SAVE DATABASE -> Saves current records to the database's .txt file
void saveDatabase() {
    FILE* file;
    int i;

    file = fopen(FILENAME, "w"); // open file for internal editing

    if (file == NULL) {
        printf("Error saving file!\n"); // error message if file cant be opened
        return;
    }

    fprintf(file, "Database Name: Sample-CMS\n");
    fprintf(file, "Authors: Assistant Prof Oran Zane Devilly\n");
    fprintf(file, "\n");
    fprintf(file, "Table Name: StudentRecords\n");
    fprintf(file, "ID\tName\t\tProgramme\t\tMark\n");

    i = 0;
    while (i < recordCount) {
        fprintf(file, "%d\t%-15s\t%-23s\t%.1f\n", // fixed formatting to follow sample
            records[i].id,
            records[i].name,
            records[i].programme,
            records[i].mark);
        i = i + 1;
    }

    fclose(file);
    printf("Database saved successfully.\n");
}

static int cmp_record_id_asc(const void* a, const void* b) {
    const StudentRecord* A = a;
    const StudentRecord* B = b;
    return (A->id < B->id) ? -1 : (A->id > B->id) ? 1 : 0;
}
static int cmp_record_mark_asc(const void* a, const void* b) {
    const StudentRecord* A = a;
    const StudentRecord* B = b;
    return (A->mark < B->mark) ? -1 : (A->mark > B->mark) ? 1 : 0;
}
static void reverse_records_inplace(void) {
    for (int i = 0, j = recordCount - 1; i < j; ++i, --j) {
        StudentRecord tmp = records[i];
        records[i] = records[j];
        records[j] = tmp;
    }
}
static void trim_newline(char* s) {
    size_t n = strlen(s);
    while (n && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = '\0';
}
static void strtoupper(char* s) { for (; *s; ++s) *s = (char)toupper((unsigned char)*s); }

// Operation 8: SORT -> Sorting of Students Record (ASCENDING/DESCENDING)
void sortRecords() {
    if (recordCount == 0) {
        printf("No records loaded. Opening database...\n");
        openDatabase();
        if (recordCount == 0) {

            return;
        }
    }
    char line[256], up[256];
    printf("Commands:\n  SHOW ALL SORT BY ID [DESC]\n  SHOW ALL SORT BY MARK [DESC]\n  EXIT\n");
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof line, stdin)) break;
        trim_newline(line);
        if (line[0] == '\0') continue;
        strncpy(up, line, sizeof up - 1); up[sizeof up - 1] = '\0';
        strtoupper(up);
        if (strcmp(up, "EXIT") == 0 || strcmp(up, "QUIT") == 0) break;
        char* pos = strstr(up, "SORT BY");
        if (!pos) { puts("Unrecognized command. Use 'SHOW ALL SORT BY ID|MARK [DESC]'."); continue; }
        pos += strlen("SORT BY");
        while (isspace((unsigned char)*pos)) ++pos;
        char field[16] = { 0 }, order[16] = { 0 };
        char* tok = strtok(pos, " \t[]"); if (tok) { strncpy(field, tok, sizeof field - 1); field[sizeof field - 1] = '\0'; }
        tok = strtok(NULL, " \t[]"); if (tok) { strncpy(order, tok, sizeof order - 1); order[sizeof order - 1] = '\0'; }
        int byId = (strcmp(field, "ID") == 0);
        int byMark = (strcmp(field, "MARK") == 0);
        if (!byId && !byMark) { printf("Unknown sort field '%s'. Use ID or MARK.\n", field); continue; }
        int desc = (strcmp(order, "DESC") == 0);
        if (recordCount == 0) { puts("No records to sort."); continue; }

        /* sort the global records array in-place */
        qsort(records, recordCount, sizeof records[0], byId ? cmp_record_id_asc : cmp_record_mark_asc);
        if (desc) reverse_records_inplace();
        showAll();
    }
}

// OPERATION 9: SHOW SUMMARY STATISTICS
// Calculates total students, average, highest, and lowest marks 

void showSummary() {
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
}