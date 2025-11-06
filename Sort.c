#define _CRT_SECURE_NO_WARNINGS
/*
 Module: Sort.c
 Purpose: Command-line utility to load and sort student records and display results.
 
 Description:
 - Loads student records from "Sample-CMS.txt" (or a path provided on the command line).
 - Accepts both the project's table-style Sample-CMS file and simple CSV lines where possible.
 - Provides an interactive prompt with commands:
     SHOW ALL SORT BY ID [DESC]
     SHOW ALL SORT BY MARK [DESC]
     EXIT
 - Sorts by numeric ID when possible, or lexicographically; sorts by mark numerically.
 - Outputs a formatted table of student records.

 Usage:
 - Build with MSVC: cl Sort.c
 - Run: Sort.exe [path/to/Sample-CMS.txt]
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 512
#define MAX_ID 64
#define MAX_NAME 128

typedef struct {
    char id[MAX_ID];
    char name[MAX_NAME];
    int mark;
} Student;

static void trim_newline(char *s) {
    size_t n = strlen(s);
    if (n && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[n - 1] = '\0';
    if (n > 1 && (s[n - 2] == '\r')) s[n - 2] = '\0';
}

static int is_numeric_str(const char *s) {
    if (*s == '\0') return 0;
    const char *p = s;
    if (*p == '+' || *p == '-') ++p;
    while (*p) {
        if (!isdigit((unsigned char)*p)) return 0;
        ++p;
    }
    return 1;
}

static int cmp_id_asc(const void *a, const void *b) {
    const Student *sa = (const Student*)a;
    const Student *sb = (const Student*)b;
    if (is_numeric_str(sa->id) && is_numeric_str(sb->id)) {
        long va = strtol(sa->id, NULL, 10);
        long vb = strtol(sb->id, NULL, 10);
        return (va < vb) ? -1 : (va > vb) ? 1 : 0;
    }
    return strcmp(sa->id, sb->id);
}
static int cmp_id_desc(const void *a, const void *b) { return -cmp_id_asc(a,b); }

static int cmp_mark_asc(const void *a, const void *b) {
    const Student *sa = (const Student*)a;
    const Student *sb = (const Student*)b;
    return (sa->mark < sb->mark) ? -1 : (sa->mark > sb->mark) ? 1 : 0;
}
static int cmp_mark_desc(const void *a, const void *b) { return -cmp_mark_asc(a,b); }

static Student *load_students(const char *filename, size_t *out_count) {
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;

    Student *arr = NULL;
    size_t cap = 0, cnt = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (line[0] == '\0') continue;

        // Attempt CSV parse: ID,Name,Mark
        char *p = line;
        // Skip leading spaces
        while (isspace((unsigned char)*p)) ++p;
        char *id = strtok(p, ",");
        char *name = strtok(NULL, ",");
        char *markstr = strtok(NULL, ",");

        if (!id || !name || !markstr) {
            // try whitespace separated fallback: id name mark (name may be one token)
            // We'll attempt sscanf as fallback
            char tid[MAX_ID], tname[MAX_NAME];
            int tmark;
            if (sscanf(line, "%63s %127[^\t\n] %d", tid, tname, &tmark) == 3) {
                id = tid; name = tname; markstr = NULL;
                // Allocate and use parsed fields
                if (cnt >= cap) {
                    size_t newcap = cap ? cap * 2 : 16;
                    Student *tmp = (Student*)realloc(arr, newcap * sizeof(Student));
                    if (!tmp) {
                        // realloc failed — free existing buffer to avoid leak and abort
                        free(arr);
                        fclose(f);
                        return NULL;
                    }
                    arr = tmp;
                    cap = newcap;
                }
                strncpy(arr[cnt].id, id, MAX_ID-1); arr[cnt].id[MAX_ID-1] = '\0';
                strncpy(arr[cnt].name, name, MAX_NAME-1); arr[cnt].name[MAX_NAME-1] = '\0';
                arr[cnt].mark = tmark;
                cnt++;
                continue;
            }
            // skip malformed line
            continue;
        }

        // Trim spaces
        while (id && *id && isspace((unsigned char)*id)) ++id;
        while (name && *name && isspace((unsigned char)*name)) ++name;
        while (markstr && *markstr && isspace((unsigned char)*markstr)) ++markstr;

        if (cnt >= cap) {
            size_t newcap = cap ? cap * 2 : 16;
            Student *tmp = (Student*)realloc(arr, newcap * sizeof(Student));
            if (!tmp) {
                free(arr);
                fclose(f);
                return NULL;
            }
            arr = tmp;
            cap = newcap;
        }
        strncpy(arr[cnt].id, id, MAX_ID-1); arr[cnt].id[MAX_ID-1] = '\0';
        // trim trailing spaces on name
        char *end = name + strlen(name) - 1;
        while (end > name && isspace((unsigned char)*end)) *end-- = '\0';
        strncpy(arr[cnt].name, name, MAX_NAME-1); arr[cnt].name[MAX_NAME-1] = '\0';
        arr[cnt].mark = atoi(markstr);
        cnt++;
    }

    fclose(f);
    *out_count = cnt;
    return arr;
}

static void print_students(const Student *arr, size_t n) {
    printf("+------+--------------------------------+------+\n");
    printf("| ID   | Name                           | Mark |\n");
    printf("+------+--------------------------------+------+\n");
    for (size_t i = 0; i < n; ++i) {
        printf("| %-4s | %-30s | %4d |\n", arr[i].id, arr[i].name, arr[i].mark);
    }
    printf("+------+--------------------------------+------+\n");
}

static void strtoupper_inplace(char *s) {
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}

int main(int argc, char **argv) {
    const char *filename = "Sample-CMS.txt";
    if (argc >= 2) filename = argv[1];

    size_t count = 0;
    Student *students = load_students(filename, &count);
    if (!students) {
        fprintf(stderr, "Failed to open or parse '%s'. Ensure file exists and uses CSV format: ID,Name,Mark\n", filename);
        // provide a small built-in sample so the tool is usable even without file
        fprintf(stderr, "Using in-memory sample data instead.\n");
        count = 5;
        students = malloc(count * sizeof(Student));
        if (!students) return 1;
        strcpy(students[0].id, "1001"); strcpy(students[0].name, "John Doe"); students[0].mark = 78;
        strcpy(students[1].id, "1002"); strcpy(students[1].name, "Jane Smith"); students[1].mark = 92;
        strcpy(students[2].id, "1003"); strcpy(students[2].name, "Alice Lee"); students[2].mark = 85;
        strcpy(students[3].id, "1004"); strcpy(students[3].name, "Bob Zhang"); students[3].mark = 70;
        strcpy(students[4].id, "A200"); strcpy(students[4].name, "Carol King"); students[4].mark = 88;
    }

    printf("Loaded %zu student(s) from '%s'.\n", count, filename);
    printf("Commands:\n");
    printf("  SHOW ALL SORT BY ID [DESC]\n");
    printf("  SHOW ALL SORT BY MARK [DESC]\n");
    printf("  EXIT\n");

    char line[MAX_LINE];
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        trim_newline(line);
        // copy uppercase for parsing but keep original if needed
        char u[MAX_LINE];
        strncpy(u, line, sizeof(u)-1);
        u[sizeof(u)-1] = '\0';
        strtoupper_inplace(u);

        if (u[0] == '\0') continue;
        if (strstr(u, "EXIT") == u || strcmp(u, "QUIT") == 0) break;

        // Check for "SHOW ALL SORT BY ID" or "SHOW ALL SORT BY MARK"
        char *by = strstr(u, "SORT BY");
        if (!by) {
            printf("Unrecognized command. Use 'SHOW ALL SORT BY ID|MARK [DESC]'.\n");
            continue;
        }

        // find field token following "SORT BY"
        by += strlen("SORT BY");
        while (isspace((unsigned char)*by)) ++by;
        int sortById = 0, sortByMark = 0, desc = 0;
        if (strncmp(by, "ID", 2) == 0) sortById = 1;
        else if (strncmp(by, "MARK", 4) == 0) sortByMark = 1;
        else {
            printf("Unknown sort field. Use ID or MARK.\n");
            continue;
        }

        // detect descending
        if (strstr(by, "DESC")) desc = 1;

        // make a copy to sort so original order can be preserved for subsequent commands
        Student *copy = malloc(count * sizeof(Student));
        if (!copy) { fprintf(stderr, "Memory allocation failed\n"); break; }
        memcpy(copy, students, count * sizeof(Student));

        if (sortById) {
            qsort(copy, count, sizeof(Student), desc ? cmp_id_desc : cmp_id_asc);
        } else {
            qsort(copy, count, sizeof(Student), desc ? cmp_mark_desc : cmp_mark_asc);
        }

        print_students(copy, count);
        free(copy);
    }

    free(students);
    printf("Goodbye.\n");
    return 0;
}