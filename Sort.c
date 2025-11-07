#define _CRT_SECURE_NO_WARNINGS
/*
 Module: Sort.c
 Purpose: Command-line utility to load and sort student records and display results.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _MSC_VER
/* Map POSIX strcasecmp to MSVC name */
#define strcasecmp _stricmp
#endif

#define MAX_LINE 512
#define MAX_ID 64
#define MAX_NAME 128

typedef struct {
    char id[MAX_ID];
    char name[MAX_NAME];
    double mark;
} Student;

static void trim_newline(char *s) {
    size_t n = strlen(s);
    if (n == 0) return;
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        --n;
    }
}

static void trim_spaces(char *s) {
    char *p = s;
    while (*p && isspace((unsigned char)*p)) ++p;
    if (p != s) memmove(s, p, strlen(p) + 1);
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) { s[n - 1] = '\0'; --n; }
}

static void unquote(char *s) {
    size_t n = strlen(s);
    if (n >= 2 && s[0] == '"' && s[n - 1] == '"') {
        memmove(s, s + 1, n - 2);
        s[n - 2] = '\0';
    }
}

static int is_numeric_str(const char *s) {
    if (!s || *s == '\0') return 0;
    const char *p = s;
    if (*p == '+' || *p == '-') ++p;
    if (*p == '\0') return 0;
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
static int cmp_id_desc(const void *a, const void *b) { return cmp_id_asc(b, a); }

static int cmp_mark_asc(const void *a, const void *b) {
    const Student *sa = (const Student*)a;
    const Student *sb = (const Student*)b;
    if (sa->mark < sb->mark) return -1;
    if (sa->mark > sb->mark) return 1;
    return 0;
}
static int cmp_mark_desc(const void *a, const void *b) { return cmp_mark_asc(b, a); }

static Student *load_students(const char *filename, size_t *out_count) {
    FILE *f = fopen(filename, "r");
    if (!f) return NULL;

    Student *arr = NULL;
    size_t cap = 0, cnt = 0;
    char line[MAX_LINE];

    /* Ignore the first 4 header lines in the database file */
    for (int i = 0; i < 4 && fgets(line, sizeof(line), f); ++i) {
        /* discard */
    }

    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        char *t = line;
        while (*t && isspace((unsigned char)*t)) ++t;
        if (*t == '\0') continue;

        char idbuf[MAX_ID] = {0};
        char namebuf[MAX_NAME] = {0};
        char markbuf[64] = {0};
        double parsed_mark = 0.0;
        int have_mark = 0;

        if (strchr(line, ',') != NULL) {
            char *first_comma = strchr(line, ',');
            if (!first_comma) continue;
            size_t idlen = (size_t)(first_comma - line);
            if (idlen >= MAX_ID) idlen = MAX_ID - 1;
            memcpy(idbuf, line, idlen);
            idbuf[idlen] = '\0';
            trim_spaces(idbuf);

            char *last_comma = strrchr(line, ',');
            if (!last_comma) continue;
            char *mark_start = last_comma + 1;
            while (*mark_start && isspace((unsigned char)*mark_start)) ++mark_start;
            strncpy(markbuf, mark_start, sizeof(markbuf) - 1);
            trim_newline(markbuf);
            trim_spaces(markbuf);
            parsed_mark = strtod(markbuf, NULL);
            have_mark = 1;

            char *name_start = first_comma + 1;
            char *second_comma = strchr(name_start, ',');
            if (!second_comma) continue;
            size_t namelen = (size_t)(second_comma - name_start);
            if (namelen >= MAX_NAME) namelen = MAX_NAME - 1;
            memcpy(namebuf, name_start, namelen);
            namebuf[namelen] = '\0';
            trim_spaces(namebuf);
            unquote(namebuf);
        } else {
            char *start = line;
            while (*start && isspace((unsigned char)*start)) ++start;
            if (*start == '\0') continue;
            char *p = start;
            while (*p && !isspace((unsigned char)*p)) ++p;
            size_t idlen = (size_t)(p - start);
            if (idlen >= MAX_ID) idlen = MAX_ID - 1;
            memcpy(idbuf, start, idlen);
            idbuf[idlen] = '\0';

            char *q = line + strlen(line) - 1;
            while (q > start && isspace((unsigned char)*q)) --q;
            if (q <= start) continue;
            char *mark_end = q + 1;
            while (q > start && !isspace((unsigned char)*q)) --q;
            char *mark_start = (q == start && !isspace((unsigned char)*q)) ? q : q + 1;
            size_t marklen = (size_t)(mark_end - mark_start);
            if (marklen >= sizeof(markbuf)) marklen = sizeof(markbuf) - 1;
            memcpy(markbuf, mark_start, marklen);
            markbuf[marklen] = '\0';
            trim_spaces(markbuf);
            parsed_mark = strtod(markbuf, NULL);
            have_mark = 1;

            char *name_start = p;
            while (name_start < mark_start && isspace((unsigned char)*name_start)) ++name_start;
            char *name_end = mark_start;
            while (name_end > name_start && isspace((unsigned char)name_end[-1])) --name_end;
            size_t namelen = (size_t)(name_end - name_start);
            if (namelen >= MAX_NAME) namelen = MAX_NAME - 1;
            memcpy(namebuf, name_start, namelen);
            namebuf[namelen] = '\0';
            trim_spaces(namebuf);
        }

        if (!have_mark) continue;
        if (strcasecmp(idbuf, "ID") == 0 || strcasecmp(idbuf, "STUDENT") == 0) continue;

        if (cnt >= cap) {
            size_t newcap = cap ? cap * 2 : 16;
            Student *tmp = (Student*)realloc(arr, newcap * sizeof(Student));
            if (!tmp) { free(arr); fclose(f); return NULL; }
            arr = tmp;
            cap = newcap;
        }

        strncpy(arr[cnt].id, idbuf, MAX_ID - 1); arr[cnt].id[MAX_ID - 1] = '\0';
        if (namebuf[0] == '\0') strncpy(arr[cnt].name, "(unknown)", MAX_NAME - 1);
        else strncpy(arr[cnt].name, namebuf, MAX_NAME - 1);
        arr[cnt].name[MAX_NAME - 1] = '\0';
        arr[cnt].mark = parsed_mark;
        cnt++;
    }

    fclose(f);
    *out_count = cnt;
    return arr;
}

static void print_students(const Student *arr, size_t n) {
    printf("+------------+--------------------------------+--------+\n");
    printf("| ID         | Name                           |  Mark  |\n");
    printf("+------------+--------------------------------+--------+\n");
    for (size_t i = 0; i < n; ++i) {
        printf("| %-10s | %-30s | %6.1f |\n", arr[i].id, arr[i].name, arr[i].mark);
    }
    printf("+------------+--------------------------------+--------+\n");
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
        fprintf(stderr, "Failed to open or parse '%s'. Ensure file exists and uses CSV format: ID,Name,Mark (or ID,Name,Programme,Mark)\n", filename);
        return 1;
    }
    if (count == 0) {
        fprintf(stderr, "No student records loaded from '%s'. Ensure the file contains valid records.\n", filename);
        free(students);
        return 1;
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
        char u[MAX_LINE];
        strncpy(u, line, sizeof(u) - 1);
        u[sizeof(u) - 1] = '\0';
        strtoupper_inplace(u);

        if (u[0] == '\0') continue;
        if (strstr(u, "EXIT") == u || strcmp(u, "QUIT") == 0) break;

        char *by = strstr(u, "SORT BY");
        if (!by) {
            printf("Unrecognized command. Use 'SHOW ALL SORT BY ID|MARK [DESC]'.\n");
            continue;
        }

        by += strlen("SORT BY");
        while (isspace((unsigned char)*by)) ++by;
        if (*by == '\0') {
            printf("Specify field to sort by: ID or MARK.\n");
            continue;
        }

        char fieldtok[16] = {0};
        char ordertok[16] = {0};
        char *tok = strtok(by, " \t[]");
        if (tok) { strncpy(fieldtok, tok, sizeof(fieldtok) - 1); fieldtok[sizeof(fieldtok) - 1] = '\0'; }
        tok = strtok(NULL, " \t[]");
        if (tok) { strncpy(ordertok, tok, sizeof(ordertok) - 1); ordertok[sizeof(ordertok) - 1] = '\0'; }

        int sortById = 0, sortByMark = 0, desc = 0;
        if (strcmp(fieldtok, "ID") == 0) sortById = 1;
        else if (strcmp(fieldtok, "MARK") == 0) sortByMark = 1;
        else {
            printf("Unknown sort field '%s'. Use ID or MARK.\n", fieldtok);
            continue;
        }

        if (ordertok[0] != '\0') {
            if (strcmp(ordertok, "DESC") == 0) desc = 1;
            else if (strcmp(ordertok, "ASC") == 0) desc = 0;
            else { printf("Unknown order '%s'. Use ASC or DESC (default ASC).\n", ordertok); continue; }
        } else desc = 0;

        Student *copy = malloc(count * sizeof(Student));
        if (!copy) { fprintf(stderr, "Memory allocation failed\n"); break; }
        memcpy(copy, students, count * sizeof(Student));

        if (sortById) qsort(copy, count, sizeof(Student), desc ? cmp_id_desc : cmp_id_asc);
        else qsort(copy, count, sizeof(Student), desc ? cmp_mark_desc : cmp_mark_asc);

        print_students(copy, count);
        free(copy);
    }

    free(students);
    printf("Goodbye.\n");
    return 0;
}