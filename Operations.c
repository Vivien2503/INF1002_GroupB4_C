/*
OPERATIONS FILE
 Purpose: Student Database Management System that is able to carry out 7 basic operations
 
 Description:
 - Loads the Database from a text file in tsv format and displays it in a formatted table
    - Allows user to Insert, Query, Update, Delete records
    - Saves the modified database back to the text file
*/

#include <stdio.h>
#include <string.h>

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
        else {
            printf("Invalid choice! Please enter a number between 1 and 7.\n");
        }
    }
    return 0;
}

// OPERATION 1 OPEN DATABASE -> Opens the database file and loads the records
void openDatabase() {
    FILE *fp;
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
    FILE *file;
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