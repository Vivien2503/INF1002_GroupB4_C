/*
 * OPERATION 8: Sort Function
 * This function sorts the student records by ID or mark, in ascending order.
*/

#define _CRT_SECURE_NO_WARNINGS
#include "student_db.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

static void strtoupper(char* s) {
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}

void sortRecords(void) {
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

        qsort(records, recordCount, sizeof records[0], byId ? cmp_record_id_asc : cmp_record_mark_asc);
        if (desc) reverse_records_inplace();

        index_rebuild(records, recordCount);

        char status[32];
        snprintf(status, sizeof status, "%s %s", byId ? "ID" : "MARK", desc ? "DESC" : "ASC");
        audit_log("SORT", NULL, NULL, status);

        showAll();
    }
}
