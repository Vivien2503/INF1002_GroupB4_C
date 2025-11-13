/*
OPERATIONS FILE
 Purpose: Student Database Management System that is able to carry out 7 basic operations
 
 Description:
 - Loads the Database from a text file in tsv format and displays it in a formatted table
    - Allows user to Insert, Query, Update, Delete records
    - Saves the modified database back to the text file
*/

#define _CRT_SECURE_NO_WARNINGS       /** CHANGE: Allow fopen/scanf on MSVC without warnings */
#include <stdio.h>
#include <string.h>
#include <time.h>                     /** CHANGE: Needed for audit log timestamps */

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

/** =========================================================================
 *  CHANGE: Forward declarations for enhancement helpers (audit + fast lookup)
 *  Implementations are at the bottom of this file in the "Enhancement Section".
 *  ========================================================================== */
void fl_audit_open(void);
void fl_audit_close(void);
void fl_audit(const char* op,
              const StudentRecord* before_or_null,
              const StudentRecord* after_or_null,
              const char* status);

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
    
    /** CHANGE: Open audit log once at program start */
    fl_audit_open();
    
    while (1) { // (1) for infinitely looping menu
        showMenu();
        if (scanf("%d", &choice) != 1) {              /** CHANGE: basic input guard */
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
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

    /** CHANGE: Currently unreachable, but safe if you add an Exit option later */
    fl_audit_close();
    return 0;
}

/* OPERATION 1 OPEN DATABASE -> Opens the database file and loads the records
 *
 * CHANGE:
 * - Loader is now tolerant: it reads ALL lines and only accepts rows that parse
 *   into 4 fields (ID, Name, Programme, Mark).
 * - This means your multi-line header from saveDatabase() is handled safely.
 * - After loading, it builds the fast lookup index.
 * - Logs OPEN outcome into audit_log.txt.
 */
void openDatabase() {
    FILE *fp;
    char line[256];
    int readResult;
    
    fp = fopen(FILENAME, "r"); // file pointer
    
    if (fp == NULL) {
        printf("Error opening file!\n"); // error message
        fl_audit("OPEN", NULL, NULL, "FAIL");      /** CHANGE: audit failure */
        return;
    }
    
    recordCount = 0; // clears previous records first before loading
    
    while (recordCount < MAX_RECORDS) { // loop continues while max records not reached
        if (fgets(line, sizeof(line), fp) == NULL) {
            break;
        }

        /** CHANGE: Accept both tabs and spaces by normalising whitespace */
        char norm[256];
        int j = 0, inws = 0;
        for (int i = 0; line[i] && j < (int)sizeof(norm) - 1; ++i) {
            if (line[i] == ' ' || line[i] == '\t') {
                if (!inws) { norm[j++] = '\t'; inws = 1; }
            } else {
                norm[j++] = line[i];
                inws = 0;
            }
        }
        norm[j] = '\0';

        readResult = sscanf(norm, "%d\t%[^\t]\t%[^\t]\t%f", // reads the record fields
                   &records[recordCount].id,
                   records[recordCount].name,
                   records[recordCount].programme,
                   &records[recordCount].mark);
        
        if (readResult == 4) { // ensures that all fields are properly read
            recordCount = recordCount + 1;
        }
        /* else: header or malformed line -> ignored */
    }
    
    fclose(fp);

    /** CHANGE: Build fast lookup index after loading */
    fl_index_build(records, recordCount);

    printf("Successfully loaded %d records from '%s'\n\n", recordCount, FILENAME);
    fl_audit("OPEN", NULL, NULL, "SUCCESS");       /** CHANGE: audit success */
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
    int pos;                                  /** CHANGE: used with fast lookup */
    
    if (recordCount >= MAX_RECORDS) {         /** CHANGE: capacity guard for audit */
        printf("Error: database full.\n");
        fl_audit("INSERT", NULL, NULL, "FAIL(FULL)");
        return;
    }

    printf("Enter student ID: ");
    scanf("%d", &newId);
    getchar();
    
    /** CHANGE: Use hash index for fast duplicate ID check */
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

    /** CHANGE: Update hash index with new ID -> position mapping */
    fl_index_put(newId, recordCount);

    /** CHANGE: Audit INSERT with after-state */
    fl_audit("INSERT", NULL, &records[recordCount], "SUCCESS");

    recordCount = recordCount + 1; // +1 record count if successful
    printf("Record added successfully!\n");
}

// OPERATION 4 QUERY RECORD -> Allows the user to search for a student record by ID
void queryRecord() {
    int searchId;
    int i;
    int found;
    int pos;                                  /** CHANGE: used with fast lookup */
    
    printf("Enter student ID to search: ");
    scanf("%d", &searchId);

    /** CHANGE: Use hash index instead of linear scan */
    if (!fl_index_get(searchId, &pos)) {
        printf("Record not found.\n");
        return;
    }

    printf("\nFound Record:\n");
    printf("ID: %d\nName: %s\nProgramme: %s\nMark: %.2f\n",
           records[pos].id, records[pos].name,
           records[pos].programme, records[pos].mark);
}

// OPERATION 5 UPDATE RECORD -> Allows the user to update a student record
void updateRecord() {
    int searchId;
    int i;
    int j;
    int found;
    char input[MAX_NAME_LEN];
    float newMark;
    int pos;                                  /** CHANGE: used with fast lookup */
    
    printf("Enter student ID to update: ");
    scanf("%d", &searchId);
    getchar();

    /** CHANGE: Fast lookup to find the record position */
    if (!fl_index_get(searchId, &pos)) {
        printf("Record not found.\n");
        return;
    }

    /** CHANGE: Snapshot "before" for audit logging */
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

    /** CHANGE: Audit UPDATE with before/after snapshots */
    fl_audit("UPDATE", &before, &records[pos], "SUCCESS");
}

// OPERATION 6 DELETE RECORD -> Allows the user to delete a student record through ID
void deleteRecord() {
    int searchId;
    int i;
    int j;
    int found;
    char confirm;
    int pos;                                  /** CHANGE: used with fast lookup */
    
    printf("Enter student ID to delete: ");
    scanf("%d", &searchId);

    /** CHANGE: Use fast lookup to get record position */
    if (!fl_index_get(searchId, &pos)) {
        printf("Record not found.\n");
        return;
    }

    printf("Are you sure you want to delete this record? (y/n): "); // delete confirmation
    scanf(" %c", &confirm);
            
    if (confirm == 'y' || confirm == 'Y') {
        /** CHANGE: Audit before deletion */
        fl_audit("DELETE", &records[pos], NULL, "SUCCESS");

        j = pos;
        while (j < recordCount - 1) { // shift records after deletion
            records[j] = records[j + 1];
            j = j + 1;
        }
        recordCount = recordCount - 1;

        /** CHANGE: rebuild index because positions changed */
        fl_index_rebuild(records, recordCount);

        printf("Record deleted successfully.\n");
    }
    else {
        printf("Deletion cancelled.\n");
    }
}

// OPERATION 7 SAVE DATABASE -> Saves current records to the database's .txt file
void saveDatabase() {
    FILE *file;
    int i;
    
    file = fopen(FILENAME, "w"); // open file for internal editing
    
    if (file == NULL) {
        printf("Error saving file!\n"); // error message if file cant be opened
        fl_audit("SAVE", NULL, NULL, "FAIL");       /** CHANGE: audit failure */
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
    fl_audit("SAVE", NULL, NULL, "SUCCESS");        /** CHANGE: audit success */
}

/* =============================================================================
   Enhancement Section (kept together at the bottom)
   - (2) Fast Lookup: open-addressing hash index (ID -> array index)
   - (1) Audit Log: append-only file with timestamps and before/after snapshots
   ========================================================================== */

/** CHANGE: Fast lookup (hash index) implementation */
#ifndef HSIZE
#define HSIZE 257  /* prime; fine for <=100 rows */
#endif

typedef struct {
    int key;
    int pos;
} FL_Slot;

static FL_Slot fl_table[HSIZE];

static unsigned fl_hmix(unsigned x){
    x ^= x >> 16; x *= 0x7feb352d;
    x ^= x >> 15; x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}
static void fl_clear(void){
    int i;
    for (i = 0; i < HSIZE; i++) {
        fl_table[i].key = 0;
        fl_table[i].pos = -1;
    }
}

void fl_index_build(const StudentRecord* arr, int count){
    int i;
    fl_clear();
    for (i = 0; i < count; i++) {
        unsigned h = fl_hmix((unsigned)arr[i].id) % HSIZE;
        int step;
        for (step = 0; step < HSIZE; step++, h = (h + 1) % HSIZE) {
            if (fl_table[h].key == 0 || fl_table[h].key == arr[i].id) {
                fl_table[h].key = arr[i].id;
                fl_table[h].pos = i;
                break;
            }
        }
    }
}

int fl_index_get(int id, int* out_pos){
    unsigned h = fl_hmix((unsigned)id) % HSIZE;
    int step;
    for (step = 0; step < HSIZE; step++, h = (h + 1) % HSIZE) {
        if (fl_table[h].key == 0) return 0;
        if (fl_table[h].key == id) {
            if (out_pos) *out_pos = fl_table[h].pos;
            return 1;
        }
    }
    return 0;
}

void fl_index_put(int id, int pos){
    unsigned h = fl_hmix((unsigned)id) % HSIZE;
    int step;
    for (step = 0; step < HSIZE; step++, h = (h + 1) % HSIZE) {
        if (fl_table[h].key == 0 || fl_table[h].key == id) {
            fl_table[h].key = id;
            fl_table[h].pos = pos;
            return;
        }
    }
}

void fl_index_rebuild(const StudentRecord* arr, int count){
    fl_index_build(arr, count);
}

/** CHANGE: Audit log implementation */
static FILE* fl_audit_fp = NULL;

void fl_audit_open(void){
    if (!fl_audit_fp) fl_audit_fp = fopen("audit_log.txt","a");
}

void fl_audit_close(void){
    if (fl_audit_fp) {
        fclose(fl_audit_fp);
        fl_audit_fp = NULL;
    }
}

static void fl_ts_now(char* buf, size_t n){
    time_t t = time(NULL);
    struct tm* m = localtime(&t);
    strftime(buf, n, "%Y-%m-%d %H:%M:%S", m);
}

static void fl_fmt_rec(const StudentRecord* s, char* out, size_t n){
    if (!s) {
        snprintf(out, n, "(null)");
        return;
    }
    snprintf(out, n, "{ID=%d,Name=\"%s\",Programme=\"%s\",Mark=%.2f}",
             s->id, s->name, s->programme, s->mark);
}

void fl_audit(const char* op,
              const StudentRecord* before_or_null,
              const StudentRecord* after_or_null,
              const char* status){
    if (!fl_audit_fp) return;
    {
        char T[32], B[160], A[160];
        fl_ts_now(T, sizeof T);
        fl_fmt_rec(before_or_null, B, sizeof B);
        fl_fmt_rec(after_or_null,  A, sizeof A);
        fprintf(fl_audit_fp, "[%s] %s %s -> %s : %s\n", T, op, B, A, status);
        fflush(fl_audit_fp);
    }
}
