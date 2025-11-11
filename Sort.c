#define _CRT_SECURE_NO_WARNINGS
/* Sort.c - command-line utility to load, sort and display student records. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

#define MAX_LINE 512
#define MAX_ID 64
#define MAX_NAME 128

typedef struct { char id[MAX_ID]; char name[MAX_NAME]; double mark; } Student;

static void trim(char *s) {
    char *p = s;
    while (*p && isspace((unsigned char)*p)) ++p;
    if (p != s) memmove(s, p, strlen(p) + 1);
    size_t n = strlen(s);
    while (n && isspace((unsigned char)s[--n])) s[n] = '\0';
}
static void unquote(char *s) { size_t n=strlen(s); if (n>=2 && s[0]=='"' && s[n-1]=='"') { memmove(s,s+1,n-2); s[n-2]=0; } }
static int isnumeric(const char *s) { if (!s||!*s) return 0; if (*s=='+'||*s=='-') ++s; if (!*s) return 0; while(*s) if (!isdigit((unsigned char)*s++)) return 0; return 1; }

static int cmp_id_asc(const void *a,const void *b){
    const Student *A=a,*B=b;
    if (isnumeric(A->id) && isnumeric(B->id)) {
        long va=strtol(A->id,NULL,10), vb=strtol(B->id,NULL,10);
        return (va<vb)?-1:(va>vb)?1:0;
    }
    return strcmp(A->id,B->id);
}
static int cmp_mark_asc(const void *a,const void *b){
    double da=((const Student*)a)->mark, db=((const Student*)b)->mark;
    return (da<db)?-1:(da>db)?1:0;
}

static Student *load_students(const char *fn, size_t *out_count){
    FILE *f = fopen(fn,"r"); if(!f) return NULL;
    char line[MAX_LINE];
    for(int i=0;i<4 && fgets(line,sizeof line,f);++i); 
    Student *arr=NULL; size_t cap=0, cnt=0;
    while(fgets(line,sizeof line,f)){
        trim(line); if(!*line) continue;
        char idb[MAX_ID]={0}, nameb[MAX_NAME]={0}, markb[64]={0};
        double mark=0; int have_mark=0;
        if (strchr(line,',')){
            char *first=strchr(line,','), *last=strrchr(line,',');
            size_t idlen = (size_t)(first - line); if(idlen>=MAX_ID) idlen=MAX_ID-1;
            memcpy(idb,line,idlen); idb[idlen]=0; trim(idb);
            trim(last+1); strncpy(markb,last+1,sizeof markb-1); markb[sizeof markb-1]=0; trim(markb); mark=strtod(markb,NULL); have_mark=1;
            char *name_end = (first==last)? last : strchr(first+1,','); if(!name_end) name_end=last;
            size_t nlen = (size_t)(name_end - (first+1)); if(nlen>=MAX_NAME) nlen=MAX_NAME-1;
            memcpy(nameb, first+1, nlen); nameb[nlen]=0; trim(nameb); unquote(nameb);
        } else {
            char buf[MAX_LINE]; strncpy(buf,line,sizeof buf-1); buf[sizeof buf-1]=0;
            char *toks[64]; int tc=0;
            char *p = strtok(buf," \t"); while(p && tc < (int)(sizeof toks/sizeof *toks)) { toks[tc++]=p; p=strtok(NULL," \t"); }
            if(tc<2) continue;
            strncpy(idb,toks[0],MAX_ID-1); idb[MAX_ID-1]=0;
            strncpy(markb,toks[tc-1],sizeof markb-1); markb[sizeof markb-1]=0; trim(markb); mark=strtod(markb,NULL); have_mark=1;
            size_t pos=0;
            for(int i=1;i<tc-1 && pos+1<MAX_NAME;i++){
                if(pos) nameb[pos++]=' ';
                strncpy(nameb+pos,toks[i],MAX_NAME-1-pos);
                pos += strlen(nameb+pos);
            }
            nameb[pos]=0; trim(nameb);
        }
        if(!have_mark) continue;
        if (strcasecmp(idb,"ID")==0 || strcasecmp(idb,"STUDENT")==0) continue;
        if (cnt>=cap){ size_t nc = cap?cap*2:16; Student *t = realloc(arr,nc*sizeof *t); if(!t){ free(arr); fclose(f); return NULL; } arr=t; cap=nc; }
        strncpy(arr[cnt].id,idb,MAX_ID-1); arr[cnt].id[MAX_ID-1]=0;
        if(!*nameb) strncpy(arr[cnt].name,"(unknown)",MAX_NAME-1); else strncpy(arr[cnt].name,nameb,MAX_NAME-1);
        arr[cnt].name[MAX_NAME-1]=0;
        arr[cnt].mark = mark; ++cnt;
    }
    fclose(f); *out_count = cnt; return arr;
}

static void print_students(const Student *a, size_t n){
    puts("+------------+--------------------------------+--------+");
    puts("| ID         | Name                           |  Mark  |");
    puts("+------------+--------------------------------+--------+");
    for(size_t i=0;i<n;++i) printf("| %-10s | %-30s | %6.1f |\n", a[i].id, a[i].name, a[i].mark);
    puts("+------------+--------------------------------+--------+");
}

static void strtoupper(char *s){ for(;*s;++s) *s=(char)toupper((unsigned char)*s); }

// Operation 8: SORT -> Sorting of Students Record (ASCENDING/DESCENDING)
int main(int argc, char **argv){
    const char *fn = argc>=2? argv[1] : "Sample-CMS.txt";
    size_t n=0; Student *students = load_students(fn,&n);
    if(!students){ fprintf(stderr,"Failed to open/parse '%s'\n",fn); return 1; }
    if(n==0){ fprintf(stderr,"No student records in '%s'\n",fn); free(students); return 1; }
    printf("Loaded %zu student(s) from '%s'.\nCommands:\n  SHOW ALL SORT BY ID [DESC]\n  SHOW ALL SORT BY MARK [DESC]\n  EXIT\n", n, fn);

    char line[MAX_LINE], up[MAX_LINE];
    while(1){
        printf("> "); if(!fgets(line,sizeof line,stdin)) break;
        trim(line); if(!*line) continue;
        strncpy(up,line,sizeof up-1); up[sizeof up-1]=0; strtoupper(up);
        if (strstr(up,"EXIT")==up || strcmp(up,"QUIT")==0) break;
        char *pos = strstr(up,"SORT BY"); if(!pos){ puts("Unrecognized command. Use 'SHOW ALL SORT BY ID|MARK [DESC]'."); continue; }
        pos += strlen("SORT BY"); while(isspace((unsigned char)*pos)) ++pos;
        char field[16]={0}, order[16]={0};
        char *tok = strtok(pos," \t[]"); if(tok) strncpy(field,tok,sizeof field-1);
        tok = strtok(NULL," \t[]"); if(tok) strncpy(order,tok,sizeof order-1);
        int byId = (strcmp(field,"ID")==0), byMark = (strcmp(field,"MARK")==0);
        if(!byId && !byMark){ printf("Unknown field '%s'. Use ID or MARK.\n", field); continue; }
        int desc = (strcmp(order,"DESC")==0);

        Student *copy = malloc(n * sizeof *copy); if(!copy){ fprintf(stderr,"Memory alloc failed\n"); break; }
        memcpy(copy, students, n * sizeof *copy);
        qsort(copy, n, sizeof *copy, byId ? cmp_id_asc : cmp_mark_asc);
        if(desc) for(size_t i=0;i<n/2;++i){ Student t=copy[i]; copy[i]=copy[n-1-i]; copy[n-1-i]=t; }
        print_students(copy,n); free(copy);
    }

    free(students); puts("Goodbye."); return 0;
}