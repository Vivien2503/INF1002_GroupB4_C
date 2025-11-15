/*
 *This file contains the audit log function.
 *Append-only file with timestamps and before/after snapshots
 *Commands logged will appear in audit_log.txt
*/


#define _CRT_SECURE_NO_WARNINGS
#include "student_db.h"
#include <time.h>
#include <string.h>

static FILE* audit_fp = NULL;

void audit_open(void) {
    if (!audit_fp) audit_fp = fopen("audit_log.txt", "a");
}

void audit_close(void) {
    if (audit_fp) {
        fclose(audit_fp);
        audit_fp = NULL;
    }
}

static void ts_now(char* buf, size_t n) {
    time_t t = time(NULL);
    struct tm* m = localtime(&t);
    strftime(buf, n, "%Y-%m-%d %H:%M:%S", m);
}

static void fmt_rec(const StudentRecord* s, char* out, size_t n) {
    if (!s) {
        snprintf(out, n, "(null)");
        return;
    }
    snprintf(out, n, "{ID=%d,Name=\"%s\",Programme=\"%s\",Mark=%.2f}",
             s->id, s->name, s->programme, s->mark);
}

void audit_log(const char* op,
               const StudentRecord* before_opt,
               const StudentRecord* after_opt,
               const char* status) {
    if (!audit_fp) return;
    char T[32], B[160], A[160];
    ts_now(T, sizeof T);
    fmt_rec(before_opt, B, sizeof B);
    fmt_rec(after_opt, A, sizeof A);
    fprintf(audit_fp, "[%s] %s %s -> %s : %s\n", T, op, B, A, status);
    fflush(audit_fp);
}
