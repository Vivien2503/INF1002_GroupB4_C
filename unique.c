#define _CRT_SECURE_NO_WARNINGS
/*
OPERATIONS FILE (WITH ENHANCEMENTS)
 Purpose: Student Database Management System that is able to carry out 9 operations

 Description:
 - Loads the Database from a text file in tsv format and displays it in a formatted table
 - Allows user to Insert, Query, Update, Delete records
 - Sorts records and displays summary statistics
 - Saves the modified database back to the text file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>   /** CHANGE: needed for audit timestamps */

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

/** CHANGE: forward declarations for enhancement helpers (audit + fast lookup) */
void audit_open(void);
void audit_close(void);
void audit_log(const char* op,
               const StudentRecord* before_opt,
               const StudentRecord* after_opt,
               const char* status);

void index_build(const StudentRecord* recs, int count);
int  index_get(int id, int* out_pos);
void index_put(int id, int pos);
void index_rebuild(const StudentRecord* recs, int count);

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

    /** CHANGE: open audit file at program start */
    audit_open();

    while (1) { // (1) for infinitely looping menu
        showMenu();
        if (scanf("%d", &choice) != 1) {          /** CHANGE: basic input guard */
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
        else if (choice == 8) {
            sortRecords();
        }
        else if (choice == 9) {
            showSummary();
        }
        else {
            printf("Invalid choice! Please enter a number between 1 and 9.\n");
        }
    }

    audit_close();  /** CHANGE: currently unreachable, but safe if you add Exit later */
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
        audit_log("OPEN", NULL, NULL, "FAIL");   /** CHANGE: audit failure */
        return;
    }

    recordCount = 0; // clears previous records first before loading

    fgets(line, sizeof(line), fp); // skips the first header line

    while (recordCount < MAX_RECORDS) { //loop continues while max records not reached
        if (fgets(line, sizeof(line), fp) == NULL) {
            break;
        }

        /** FIX: Removed whitespace normalization - it was breaking multi-word fields */
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

    /** CHANGE: build fast lookup index after loading */
    index_build(records, recordCount);

    printf("Successfully loaded %d records from '%s'\n\n", recordCount, FILENAME);
    audit_log("OPEN", NULL, NULL, "SUCCESS");   /** CHANGE: audit success */
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
    int pos;   /** CHANGE: used for fast duplicate check */

    printf("Enter student ID: ");
    scanf("%d", &newId);
    getchar();

    duplicate = 0;     // check for duplicate ID

    /** CHANGE: use hash index if available */
    if (index_get(newId, &pos)) {
        duplicate = 1;
    } else {
        i = 0;
        while (i < recordCount) {
            if (records[i].id == newId) {
                duplicate = 1;
                break;
            }
            i = i + 1;
        }
    }

    if (duplicate == 1) { // exit and back to menu if duplicate found
        printf("Error: Student ID already exists. Insertion cancelled.\n");
        audit_log("INSERT", NULL, NULL, "FAIL(DUPLICATE)");  /** CHANGE */
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

    /** CHANGE: update index and audit */
    index_put(newId, recordCount);
    audit_log("INSERT", NULL, &records[recordCount], "SUCCESS");

    recordCount = recordCount + 1; // +1 record count if successful
    printf("Record added successfully!\n");
}

// OPERATION 4 QUERY RECORD -> Allows the user to search for a student record by ID
void queryRecord() {
    int searchId;
    int i;
    int found;
    int pos;   /** CHANGE */

    printf("Enter student ID to search: ");
    scanf("%d", &searchId);

    /** CHANGE: use fast lookup first */
    if (index_get(searchId, &pos)) {
        printf("\nFound Record:\n");
        printf("ID: %d\nName: %s\nProgramme: %s\nMark: %.2f\n",
            records[pos].id, records[pos].name,
            records[pos].programme, records[pos].mark);

        /** CHANGE: log successful query (FOUND via index) */
        audit_log("QUERY", NULL, &records[pos], "FOUND");
        return;
    }

    found = 0;
    i = 0;
    while (i < recordCount) { // loop to find matching ID
        if (records[i].id == searchId) {
            printf("\nFound Record:\n");
            printf("ID: %d\nName: %s\nProgramme: %s\nMark: %.2f\n",
                records[i].id, records[i].name,
                records[i].programme, records[i].mark);
            found = 1;

            /** CHANGE: log successful query (FOUND via linear scan) */
            audit_log("QUERY", NULL, &records[i], "FOUND");
            break;
        }
        i = i + 1;
    }

    if (found == 0) {
        printf("Record not found.\n");

        /** CHANGE: log failed query (NOT_FOUND) */
        audit_log("QUERY", NULL, NULL, "NOT_FOUND");
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
    int pos;   /** CHANGE */

    printf("Enter student ID to update: ");
    scanf("%d", &searchId);
    getchar();

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
        printf("Record not found.\n");
        return;
    }

    /** CHANGE: snapshot before for audit */
    StudentRecord before = records[i];

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

    /** CHANGE: audit update */
    audit_log("UPDATE", &before, &records[i], "SUCCESS");
}

// OPERATION 6 DELETE RECORD -> Allows the user to delete a student record through ID
void deleteRecord() {
    int searchId;
    int i;
    int j;
    int found;
    char confirm;
    int pos;   /** CHANGE */

    printf("Enter student ID to delete: ");
    scanf("%d", &searchId);

    found = 0;
    if (index_get(searchId, &pos)) {
        found = 1;
        i = pos;
    } else {
        i = 0;
        while (i < recordCount) {
            if (records[i].id == searchId) { // if ID is found:
                found = 1;
                break;
            }
            i = i + 1;
        }
    }

    if (found == 0) {
        printf("Record not found.\n");
        return;
    }

    printf("Are you sure you want to delete this record? (y/n): "); // delete confirmation
    scanf(" %c", &confirm);

    if (confirm == 'y' || confirm == 'Y') {
        /** CHANGE: audit before delete */
        audit_log("DELETE", &records[i], NULL, "SUCCESS");

        j = i;
        while (j < recordCount - 1) { // shift records after deletion
            records[j] = records[j + 1];
            j = j + 1;
        }
        recordCount = recordCount - 1;

        /** CHANGE: rebuild index after delete */
        index_rebuild(records, recordCount);

        printf("Record deleted successfully.\n");
    }
    else {
        printf("Deletion cancelled.\n");
    }
}

// OPERATION 7 SAVE DATABASE -> Saves current records to the database's .txt file
void saveDatabase() {
    FILE* file;
    int i;

    file = fopen(FILENAME, "w"); // open file for internal editing

    if (file == NULL) {
        printf("Error saving file!\n"); // error message if file cant be opened
        audit_log("SAVE", NULL, NULL, "FAIL");   /** CHANGE */
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
    audit_log("SAVE", NULL, NULL, "SUCCESS");   /** CHANGE */
}

/* ===== existing helpers for sort & summary ===== */

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

// Operation 8: Sort Student Records (ASCENDING/DESCENDING)
void sortRecords()
{
    /* Ensure data is loaded */
    if (recordCount == 0) {
        printf("No records loaded. Attempting to open database...\n");
        openDatabase();
        if (recordCount == 0) {
            return;
        }
    }

    printf("Commands:\n"
        "  SHOW ALL SORT BY ID [DESC]\n"
        "  SHOW ALL SORT BY MARK [DESC]\n"
        "  EXIT\n");

    char input[256], normalized[256];

    for (;;) {
        printf("> ");

        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        trim_newline(input);
        if (input[0] == '\0') {
            continue;
        }

        /* Normalize the command to uppercase */
        strncpy(normalized, input, sizeof(normalized));
        normalized[sizeof(normalized) - 1] = '\0';
        strtoupper(normalized);

        /* Exit option */
        if (!strcmp(normalized, "EXIT") || !strcmp(normalized, "QUIT")) {
            break;
        }

        /* Must contain SORT BY */
        char* sortPos = strstr(normalized, "SORT BY");
        if (!sortPos) {
            puts("Unrecognized command. Use: SHOW ALL SORT BY ID|MARK [DESC].");
            continue;
        }

        /* Move pointer to field token */
        sortPos += strlen("SORT BY");
        while (isspace((unsigned char)*sortPos)) sortPos++;

        char field[16] = { 0 };
        char order[16] = { 0 };

        /* Extract field and order */
        char* token = strtok(sortPos, " \t[]");
        if (token) strncpy(field, token, sizeof(field) - 1);

        token = strtok(NULL, " \t[]");
        if (token) strncpy(order, token, sizeof(order) - 1);

        int sortById = (!strcmp(field, "ID"));
        int sortByMark = (!strcmp(field, "MARK"));

        if (!sortById && !sortByMark) {
            printf("Unknown sort field '%s'. Use ID or MARK.\n", field);
            continue;
        }

        int descending = (!strcmp(order, "DESC"));

        /* Perform sorting */
        qsort(records, recordCount, sizeof(records[0]),
            sortById ? cmp_record_id_asc : cmp_record_mark_asc);

        if (descending) {
            reverse_records_inplace();
        }

        /* Rebuild index after ordering */
        index_rebuild(records, recordCount);

        /* Write audit log */
        char status[32];
        snprintf(status, sizeof(status), "%s %s",
            sortById ? "ID" : "MARK",
            descending ? "DESC" : "ASC");
        audit_log("SORT", NULL, NULL, status);

        /* Display result */
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

    /** CHANGE: audit summary execution */
    audit_log("SUMMARY", NULL, NULL, "SUCCESS");
}

/* ================= Enhancement Section: fast lookup + audit ============== */

#define HSIZE 257   /** CHANGE: hash table size (prime, > MAX_RECORDS) */

typedef struct {
    int key;
    int pos;
} Slot;

static Slot indexTable[HSIZE];

static unsigned hmix(unsigned x){
    x ^= x >> 16; x *= 0x7feb352d;
    x ^= x >> 15; x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}
static void index_clear(void){
    for (int i=0;i<HSIZE;i++){
        indexTable[i].key = 0;
        indexTable[i].pos = -1;
    }
}

/** CHANGE: build hash index from current records */
void index_build(const StudentRecord* recs, int count){
    index_clear();
    for (int i=0;i<count;i++){
        unsigned h = hmix((unsigned)recs[i].id) % HSIZE;
        for (int step=0; step<HSIZE; step++, h=(h+1)%HSIZE){
            if (indexTable[h].key == 0 || indexTable[h].key == recs[i].id){
                indexTable[h].key = recs[i].id;
                indexTable[h].pos = i;
                break;
            }
        }
    }
}

/** CHANGE: lookup ID -> position (returns 1 if found, 0 otherwise) */
int index_get(int id, int* out_pos){
    unsigned h = hmix((unsigned)id) % HSIZE;
    for (int step=0; step<HSIZE; step++, h=(h+1)%HSIZE){
        if (indexTable[h].key == 0) return 0;
        if (indexTable[h].key == id){
            if (out_pos) *out_pos = indexTable[h].pos;
            return 1;
        }
    }
    return 0;
}

/** CHANGE: insert/overwrite a single ID mapping */
void index_put(int id, int pos){
    unsigned h = hmix((unsigned)id) % HSIZE;
    for (int step=0; step<HSIZE; step++, h=(h+1)%HSIZE){
        if (indexTable[h].key == 0 || indexTable[h].key == id){
            indexTable[h].key = id;
            indexTable[h].pos = pos;
            return;
        }
    }
}

/** CHANGE: rebuild index after deletes or sorting */
void index_rebuild(const StudentRecord* recs, int count){
    index_build(recs, count);
}

/* --------------------------- Audit log --------------------------- */

static FILE* audit_fp = NULL;

void audit_open(void){
    if (!audit_fp) audit_fp = fopen("audit_log.txt","a");
}
void audit_close(void){
    if (audit_fp){
        fclose(audit_fp);
        audit_fp = NULL;
    }
}
static void ts_now(char* buf, size_t n){
    time_t t = time(NULL);
    struct tm* m = localtime(&t);
    strftime(buf, n, "%Y-%m-%d %H:%M:%S", m);
}
static void fmt_rec(const StudentRecord* s, char* out, size_t n){
    if (!s){
        snprintf(out, n, "(null)");
        return;
    }
    snprintf(out, n, "{ID=%d,Name=\"%s\",Programme=\"%s\",Mark=%.2f}",
             s->id, s->name, s->programme, s->mark);
}
void audit_log(const char* op,
               const StudentRecord* before_opt,
               const StudentRecord* after_opt,
               const char* status){
    if (!audit_fp) return;
    char T[32], B[160], A[160];
    ts_now(T, sizeof T);
    fmt_rec(before_opt, B, sizeof B);
    fmt_rec(after_opt,  A, sizeof A);
    fprintf(audit_fp, "[%s] %s %s -> %s : %s\n", T, op, B, A, status);
    fflush(audit_fp);
}
