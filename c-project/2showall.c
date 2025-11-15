/*
 * OPERATION 2: Show All Function
 * This function displays all student records currently loaded in memory.
*/

#include "student_db.h"

void showAll(void) { // display all student records
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
