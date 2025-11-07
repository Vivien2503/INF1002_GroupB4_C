#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define FILENAME "Sample-CMS.txt"
#define MAX_RECORDS 100
#define MAX_NAME 40
#define MAX_PROG 40
#define HSIZE 257

typedef struct { int id; char name[MAX_NAME]; char programme[MAX_PROG]; float mark; } Rec;

static Rec recs[MAX_RECORDS]; static int nrec = 0;
/* hash: ID -> index */
static int hkey[HSIZE]; /* 0 empty, -1 tombstone, else id */
static int hpos[HSIZE];

static unsigned hmix(unsigned x){ x^=x>>16; x*=0x7feb352d; x^=x>>15; x*=0x846ca68b; x^=x>>16; return x; }
static void hclear(void){ for(int i=0;i<HSIZE;i++){hkey[i]=0; hpos[i]=-1;} }
static int hput(int id,int pos){ unsigned h=hmix((unsigned)id)%HSIZE; for(int i=0;i<HSIZE;i++,h=(h+1)%HSIZE) if(hkey[h]==0||hkey[h]==-1||hkey[h]==id){hkey[h]=id;hpos[h]=pos;return 1;} return 0; }
static int hget(int id,int*pos){ unsigned h=hmix((unsigned)id)%HSIZE; for(int i=0;i<HSIZE;i++,h=(h+1)%HSIZE){ if(hkey[h]==0) return 0; if(hkey[h]==id){ *pos=hpos[h]; return 1; } } return 0; }
static void hbuild(void){ hclear(); for(int i=0;i<nrec;i++) hput(recs[i].id,i); }

/* audit */
static FILE* auditf=NULL;
static void audit_open(void){ if(!auditf) auditf=fopen("audit_log.txt","a"); }
static void audit_close(void){ if(auditf){ fclose(auditf); auditf=NULL; } }
static void ts(char*buf,size_t n){ time_t t=time(NULL); struct tm*m=localtime(&t); strftime(buf,n,"%Y-%m-%d %H:%M:%S",m); }
static void fmt(const Rec*s,char*out,size_t n){ if(!s){snprintf(out,n,"(null)");return;} snprintf(out,n,"{ID=%d,Name=\"%s\",Prog=\"%s\",Mark=%.2f}",s->id,s->name,s->programme,s->mark); }
static void audit(const char*op,const Rec*b,const Rec*a,const char*st){ if(!auditf) return; char T[32],B[160],A[160]; ts(T,32); fmt(b,B,160); fmt(a,A,160); fprintf(auditf,"[%s] %s %s -> %s : %s\n",T,op,B,A,st); fflush(auditf); }

/* perf */
typedef struct { unsigned long c; double ms; } P; static P p_open,p_show,p_ins,p_qry,p_upd,p_del,p_save,p_stat;
static clock_t t0; static P* cur=NULL;
static void begin(P* p){ cur=p; t0=clock(); }
static void endok(void){ if(!cur) return; cur->c++; cur->ms+=1000.0*(clock()-t0)/CLOCKS_PER_SEC; cur=NULL; }
static void stats(void){
    printf("\nOP      COUNT   TOTAL(ms)  AVG(ms)\n");
    #define PR(n,p) printf("%-6s %6lu   %9.2f  %7.2f\n",n,(unsigned long)(p).c,(p).ms,(p).c?(p).ms/(p).c:0.0)
    PR("OPEN",p_open); PR("SHOW",p_show); PR("INSERT",p_ins); PR("QUERY",p_qry);
    PR("UPDATE",p_upd); PR("DELETE",p_del); PR("SAVE",p_save); PR("STATS",p_stat);
    #undef PR
}

/* I/O helpers */
static void rstrip(char*s){ size_t L=strlen(s); while(L&& (s[L-1]=='\n'||s[L-1]=='\r')) s[--L]=0; }

static void openDB(void){
    begin(&p_open);
    FILE* f=fopen(FILENAME,"r"); if(!f){ puts("Error opening file!"); audit("OPEN",NULL,NULL,"FAIL"); endok(); return; }
    nrec=0; char line[256];
    /* skip until header row with columns */
    while(fgets(line,256,f)){ if(strstr(line,"ID")&&strstr(line,"Name")&&strstr(line,"Programme")&&strstr(line,"Mark")) break; }
    while(nrec<MAX_RECORDS && fgets(line,256,f)){
        int id; char name[MAX_NAME], prog[MAX_PROG]; float mark; 
        int n=sscanf(line,"%d\t%39[^\t]\t%39[^\t]\t%f",&id,name,prog,&mark);
        if(n==4){ recs[nrec].id=id; strncpy(recs[nrec].name,name,MAX_NAME-1); recs[nrec].name[MAX_NAME-1]=0;
                  strncpy(recs[nrec].programme,prog,MAX_PROG-1); recs[nrec].programme[MAX_PROG-1]=0; recs[nrec].mark=mark; nrec++; }
    }
    fclose(f); hbuild(); printf("Loaded %d records from '%s'\n",nrec,FILENAME); audit("OPEN",NULL,NULL,"SUCCESS"); endok();
}

static void showAll(void){
    begin(&p_show);
    printf("\nID\tName\t\tProgramme\t\tMark\n-----------------------------------------------\n");
    for(int i=0;i<nrec;i++) printf("%d\t%-15s\t%-16s\t%.2f\n",recs[i].id,recs[i].name,recs[i].programme,recs[i].mark);
    endok();
}

static void insertRec(void){
    begin(&p_ins);
    if(nrec>=MAX_RECORDS){ puts("DB full."); audit("INSERT",NULL,NULL,"FAIL(FULL)"); endok(); return; }
    Rec s={0}; int pos;
    printf("Enter ID: "); scanf("%d",&s.id); getchar();
    if(hget(s.id,&pos)){ puts("ID exists."); audit("INSERT",NULL,NULL,"FAIL(DUP)"); endok(); return; }
    printf("Enter name: "); fgets(s.name,MAX_NAME,stdin); rstrip(s.name);
    printf("Enter programme: "); fgets(s.programme,MAX_PROG,stdin); rstrip(s.programme);
    printf("Enter mark: "); scanf("%f",&s.mark);
    recs[nrec]=s; hput(s.id,nrec); nrec++; puts("Inserted."); audit("INSERT",NULL,&s,"SUCCESS"); endok();
}

static void queryRec(void){
    begin(&p_qry);
    int id,pos; printf("Enter ID: "); scanf("%d",&id);
    if(!hget(id,&pos)){ puts("Not found."); endok(); return; }
    Rec*r=&recs[pos]; printf("\nID:%d\nName:%s\nProgramme:%s\nMark:%.2f\n",r->id,r->name,r->programme,r->mark); endok();
}

static void updateRec(void){
    begin(&p_upd);
    int id,pos; char buf[128]; printf("Enter ID: "); scanf("%d",&id); getchar();
    if(!hget(id,&pos)){ puts("Not found."); endok(); return; }
    Rec before=recs[pos];
    printf("New name (enter to skip): "); fgets(buf,128,stdin); if(buf[0]!='\n'){ rstrip(buf); strncpy(recs[pos].name,buf,MAX_NAME-1); recs[pos].name[MAX_NAME-1]=0; }
    printf("New programme (enter to skip): "); fgets(buf,128,stdin); if(buf[0]!='\n'){ rstrip(buf); strncpy(recs[pos].programme,buf,MAX_PROG-1); recs[pos].programme[MAX_PROG-1]=0; }
    printf("New mark (-1 to skip): "); float m; scanf("%f",&m); if(m>=0) recs[pos].mark=m;
    puts("Updated."); audit("UPDATE",&before,&recs[pos],"SUCCESS"); endok();
}

static void deleteRec(void){
    begin(&p_del);
    int id,pos; printf("Enter ID: "); scanf("%d",&id);
    if(!hget(id,&pos)){ puts("Not found."); endok(); return; }
    Rec before=recs[pos];
    printf("Confirm delete (y/n): "); char c; scanf(" %c",&c); if(c!='y'&&c!='Y'){ puts("Cancelled."); endok(); return; }
    for(int i=pos;i<nrec-1;i++) recs[i]=recs[i+1]; nrec--; hbuild(); puts("Deleted."); audit("DELETE",&before,NULL,"SUCCESS"); endok();
}

static void saveDB(void){
    begin(&p_save);
    FILE*f=fopen(FILENAME,"w"); if(!f){ puts("Save error."); audit("SAVE",NULL,NULL,"FAIL"); endok(); return; }
    fprintf(f,"Table Name: StudentRecords\n");
    fprintf(f,"ID\tName\t\tProgramme\t\tMark\n");
    for(int i=0;i<nrec;i++) fprintf(f,"%d\t%s\t%s\t%.2f\n",recs[i].id,recs[i].name,recs[i].programme,recs[i].mark);
    fclose(f); puts("Saved."); audit("SAVE",NULL,NULL,"SUCCESS"); endok();
}

static void menu(void){
    printf("\n1.Open  2.Show  3.Insert  4.Query  5.Update  6.Delete  7.Save  8.Stats  9.Exit\n> ");
}

int main(void){
    audit_open();
    while(1){
        int ch; menu(); if(scanf("%d",&ch)!=1){ int c; while((c=getchar())!='\n'&&c!=EOF){} continue; }
        int c; while((c=getchar())!='\n'&&c!=EOF){}
        if(ch==1) openDB();
        else if(ch==2) showAll();
        else if(ch==3) insertRec();
        else if(ch==4) queryRec();
        else if(ch==5) updateRec();
        else if(ch==6) deleteRec();
        else if(ch==7) saveDB();
        else if(ch==8){ begin(&p_stat); stats(); endok(); }
        else if(ch==9) break;
        else puts("Invalid.");
    }
    audit_close();
    return 0;
}
