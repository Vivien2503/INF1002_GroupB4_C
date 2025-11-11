/*
OPERATIONS FILE
 Purpose: Student Database Management System that is able to carry out 7 basic operations
 
 Description:
 - Loads the Database from a text file in tsv format and displays it in a formatted table
    - Allows user to Insert, Query, Update, Delete records
    - Saves the modified database back to the text file
*/

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <time.h>  /* for audit timestamps */

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

/* ===== Forward declarations for enhancement helpers (implemented at bottom) ===== */
void fl_audit_open(void);
void fl_audit_close(void);
void fl_audit(const char* op, const StudentRecord* before_or_null,
              const StudentRecord* after_or_null, const char* status);

void fl_index_build(const StudentRecord* records, int count);
int  fl_index_get(int id, int* out_pos);
void fl_index_put(int id, int pos);
void fl_index_rebuild(const StudentRecord* records, int count);

/* ============================== Original Interface ============================== */
void openDatabase();
void showAll();
void insertRecord();
void queryRecord();
void updateRecord();
void deleteRecord();
void saveDatabase();

void showMenu() {
    printf("\n=== Student Database Management System ===\n");
    printf("1. Open Database\n");
    printf("2. Show All Records\n");
    printf("3. Insert Record\n");
    printf("4. Query Record\n");
    printf("5. Update Record\n");
    printf("6. Delete Record\n");
    printf("7. Save Database\n");
    printf("Enter your choice (1-7): ");
}

int main() {
    int choice;

    /* open audit log once */
    fl_audit_open();
    
    while (1) { // (1) for infinitely looping menu
        showMenu();
        if (scanf("%d", &choice) != 1) {
            int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }
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
        else {
            printf("Invalid choice! Please enter a number between 1 and 7.\n");
        }
    }

    /* (unreachable in current loop, but safe if you later add an Exit) */
    fl_audit_close();
    return 0;
}

/* OPERATION 1 OPEN DATABASE -> Opens the database file and loads the records
   NOTE: tolerant loader — reads whole file and only counts lines that parse 4 fields */
void openDatabase() {
    FILE *fp;
    char line[256];
    int readResult;
    
    fp = fopen(FILENAME, "r"); // file pointer
    
    if (fp == NULL) {
        printf("Error opening file!\n"); // error message
        fl_audit("OPEN", NULL, NULL, "FAIL");
        return;
    }
    
    recordCount = 0; // clears previous records first before loading
    
    /* Read all lines; ignore headers/blank lines; only accept rows with 4 fields */
    while (recordCount < MAX_RECORDS) {
        if (fgets(line, sizeof(line), fp) == NULL) {
            break;
        }

        readResult = sscanf(line, "%d\t%[^\t]\t%[^\t]\t%f", // (id>name>prog>mark)
                   &records[recordCount].id,
                   records[recordCount].name,
                   records[recordCount].programme,
                   &records[recordCount].mark);
        
        if (readResult == 4) { // ensures all fields are properly read
            recordCount = recordCount + 1;
        }
        /* else: it was likely a header or malformed line; safely ignored */
    }
    
    fclose(fp);

    /* Build fast lookup index for O(1) ID-based operations */
    fl_index_build(records, recordCount);

    printf("Successfully loaded %d records from '%s'\n\n", recordCount, FILENAME);
    fl_audit("OPEN", NULL, NULL, "SUCCESS");
}

/* OPERATION 2 SHOW ALL -> Displays the student records in a formatted table */
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

/* OPERATION 3 INSERT RECORD -> Allows user to add new student records */
void insertRecord() {
    int newId;
    int i;
    int j;
    int pos; /* for fast duplicate check */
    
    if (recordCount >= MAX_RECORDS) {
        printf("Error: database full.\n");
        fl_audit("INSERT", NULL, NULL, "FAIL(FULL)");
        return;
    }

    printf("Enter student ID: ");
    scanf("%d", &newId);
    getchar();
    
    /* Fast duplicate check via hash index */
    if (fl_index_get(newId, &pos)) {
        printf("Error: Student ID already exists. Insertion cancelled.\n");
        fl_audit("INSERT", NULL, NULL, "FAIL(DUPLICATE)");
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

    /* add to fast index (ID -> current position) */
    fl_index_put(newId, recordCount);

    /* audit with after-values */
    fl_audit("INSERT", NULL, &records[recordCount], "SUCCESS");

    recordCount = recordCount + 1; // +1 record count if successful
    printf("Record added successfully!\n");
}

/* OPERATION 4 QUERY RECORD -> Allows the user to search for a student record by ID */
void queryRecord() {
    int searchId;
    int pos;
    
    printf("Enter student ID to search: ");
    scanf("%d", &searchId);

    /* Fast lookup instead of O(n) scan */
    if (!fl_index_get(searchId, &pos)) {
        printf("Record not found.\n");
        return;
    }
    
    printf("\nFound Record:\n");
    printf("ID: %d\nName: %s\nProgramme: %s\nMark: %.2f\n",
           records[pos].id, records[pos].name,
           records[pos].programme, records[pos].mark);
}

/* OPERATION 5 UPDATE RECORD -> Allows the user to update a student record */
void updateRecord() {
    int searchId;
    int i;
    int j;
    int pos;
    char input[MAX_NAME_LEN];
    float newMark;
    
    printf("Enter student ID to update: ");
    scanf("%d", &searchId);
    getchar();

    /* Fast lookup to find the position */
    if (!fl_index_get(searchId, &pos)) {
        printf("Record not found.\n");
        return;
    }

    /* snapshot for audit (before) */
    {
        StudentRecord before = records[pos];

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
                records[pos].name[j] = input[j];
                j = j + 1;
            }
            records[pos].name[j] = '\0';
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
                records[pos].programme[j] = input[j];
                j = j + 1;
            }
            records[pos].programme[j] = '\0';
        }

        printf("Enter new mark (or -1 to skip): ");
        scanf("%f", &newMark);
        
        if (newMark >= 0) {
            records[pos].mark = newMark;
        }

        printf("Record updated successfully.\n");

        /* audit with before/after */
        fl_audit("UPDATE", &before, &records[pos], "SUCCESS");
    }
}

/* OPERATION 6 DELETE RECORD -> Allows the user to delete a student record through ID */
void deleteRecord() {
    int searchId;
    int i;
    int j;
    int pos;
    char confirm;
    
    printf("Enter student ID to delete: ");
    scanf("%d", &searchId);

    /* Fast lookup to find the position */
    if (!fl_index_get(searchId, &pos)) {
        printf("Record not found.\n");
        return;
    }
            
    printf("Are you sure you want to delete this record? (y/n): "); // delete confirmation
    scanf(" %c", &confirm);
            
    if (confirm == 'y' || confirm == 'Y') {
        /* audit before deletion */
        fl_audit("DELETE", &records[pos], NULL, "SUCCESS");

        j = pos;
        while (j < recordCount - 1) { // shift records after deletion
            records[j] = records[j + 1];
            j = j + 1;
        }
        recordCount = recordCount - 1;

        /* rebuild index because all positions after 'pos' shifted left by 1 */
        fl_index_rebuild(records, recordCount);

        printf("Record deleted successfully.\n");
    }
    else {
        printf("Deletion cancelled.\n");
    }
}

/* OPERATION 7 SAVE DATABASE -> Saves current records to the database's .txt file */
void saveDatabase() {
    FILE *file;
    int i;
    
    file = fopen(FILENAME, "w"); // open file for internal editing
    
    if (file == NULL) {
        printf("Error saving file!\n"); // error message if file cant be opened
        fl_audit("SAVE", NULL, NULL, "FAIL");
        return;
    }
    
    fprintf(file, "Database Name: Sample-CMS\n");
    fprintf(file, "Authors: Assistant Prof Oran Zane Devilly\n");
    fprintf(file, "\n");
    fprintf(file, "Table Name: StudentRecords\n");
    fprintf(file,
