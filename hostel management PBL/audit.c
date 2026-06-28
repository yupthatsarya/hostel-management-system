#include "audit.h"
#include "utils.h"
#include <time.h>

void writeAuditLog(const char *username, const char *action) {
    FILE *fp = fopen(FILE_AUDIT, "a");
    time_t rawtime;
    struct tm *timeinfo;
    char timeStr[26];

    if (fp == NULL) {
        return;
    }

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    /* Format time string: e.g. "2026-05-26 12:00:00" */
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

    fprintf(fp, "[%s] User: %-15s | Action: %s\n", timeStr, username, action);
    fclose(fp);
}

void displayAuditTrail(void) {
    FILE *fp = fopen(FILE_AUDIT, "r");
    char line[256];
    int count = 0;

    if (fp == NULL) {
        printf("\n" COLOR_YELLOW "No audit logs found yet." COLOR_RESET "\n");
        return;
    }

    printf("\n" COLOR_BOLD "--- SYSTEM AUDIT TRAIL ---" COLOR_RESET "\n");
    printDivider('-', 75);

    while (fgets(line, sizeof(line), fp) != NULL) {
        printf("%s", line);
        count++;
        if (count % 20 == 0) {
            printf("\n" COLOR_CYAN "[Press Enter to see more logs...]" COLOR_RESET "\n");
            getchar();
        }
    }

    printDivider('-', 75);
    printf("Total log entries displayed: %d\n", count);
    fclose(fp);
}
