/*
 *This file contains the main function for the Student Database Management System.
 *It includes the database management system's loop and the declaration statement.
 *
 *IMPORTANT PLEASE READ BELOW
 *TO RUN THE CODE, COPY THIS INTO CONSOLE AND ENTER: student_db main.c 1open.c 2showall.c 3insert.c 4query.c 5update.c 6delete.c 7save.c 8sort.c 9summary.c audit.c index.c
 *ENSURE THAT YOUR TERMINAL IS IN THE CORRECT DIRECTORY WHERE THE FILES ARE LOCATED
 *THEN, RUN THE PROGRAM WITH: ./student_db
*/


#define _CRT_SECURE_NO_WARNINGS
#include "student_db.h"
#include <ctype.h>
#include <string.h>

StudentRecord records[MAX_RECORDS];
int recordCount = 0;

void showDeclaration(void) { // printing of declaration statement at start of program
    printf("========================================\n");
    printf("              Declaration               \n");
    printf("========================================\n\n");
    printf("SIT's policy on copying does not allow the students to copy source code as well as assessment solutions\n");
    printf("from another person AI or other places. It is the students' responsibility to guarantee that their\n");
    printf("assessment solutions are their own work. Meanwhile, the students must also ensure that their work is\n");
    printf("not accessible by others. Where such plagiarism is detected, both of the assessments involved will\n");
    printf("receive ZERO mark.\n\n");
    printf("We hereby declare that:\n");
    printf("- We fully understand and agree to the abovementioned plagiarism policy.\n");
    printf("- We did not copy any code from others or from other places.\n");
    printf("- We did not share our codes with others or upload to any other places for public access and will\n");
    printf("  not do that in the future.\n");
    printf("- We agree that our project will receive Zero mark if there is any plagiarism detected.\n");
    printf("- We agree that we will not disclose any information or material of the group project to others or\n");
    printf("  upload to any other places for public access.\n");
    printf("We agree that we did not copy any code directly from AI generated sources\n\n");
    printf("Declared by: Group P13-7\n");
    printf("Team members:\n");
    printf("1. Amerik Tay\n");
    printf("2. Deng HuiQi\n");
    printf("3. Donavan Padrick Sitinjak\n");
    printf("4. Tristan Foo\n");
    printf("5. Vivien Tan\n");
    printf("Date: \n\n");
    printf("Press Enter to continue...");
    getchar();
    printf("\n");
}

void showMenu(void) { // management system menu display
    printf("\n=== Student Database Management System ===\n");
    printf("Available Commands:\n");
    printf("  OPEN     - Open Database\n");
    printf("  SHOWALL  - Show All Records\n");
    printf("  INSERT   - Insert Record\n");
    printf("  QUERY    - Query Record\n");
    printf("  UPDATE   - Update Record\n");
    printf("  DELETE   - Delete Record\n");
    printf("  SAVE     - Save Database\n");
    printf("  SORT     - Sort Records\n");
    printf("  SUMMARY  - Show Summary Statistics\n");
    printf("  QUIT     - Exit Program\n");
    printf("Enter command: ");
}

int main(void) {
    char command[50];

    showDeclaration();

    audit_open();

    while (1) { // menu loop
        showMenu();
        if (scanf("%49s", command) != 1) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }
        
        // case insensitive
        for (int i = 0; command[i]; i++) {
            command[i] = toupper(command[i]);
        }
        
        // clears input buffer
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {}

        if (strcmp(command, "OPEN") == 0) {
            openDatabase();
        }
        else if (strcmp(command, "SHOWALL") == 0) {
            showAll();
        }
        else if (strcmp(command, "INSERT") == 0) {
            insertRecord();
        }
        else if (strcmp(command, "QUERY") == 0) {
            queryRecord();
        }
        else if (strcmp(command, "UPDATE") == 0) {
            updateRecord();
        }
        else if (strcmp(command, "DELETE") == 0) {
            deleteRecord();
        }
        else if (strcmp(command, "SAVE") == 0) {
            saveDatabase();
        }
        else if (strcmp(command, "SORT") == 0) {
            sortRecords();
        }
        else if (strcmp(command, "SUMMARY") == 0) {
            showSummary();
        }
        else if (strcmp(command, "QUIT") == 0) {
            printf("Exiting program. Goodbye!\n");
            audit_log("EXIT", NULL, NULL, "SUCCESS");
            break;
        }
        else {
            printf("Invalid command! Please enter a valid command from the menu.\n");
        }
    }

    audit_close();
    return 0;
}
