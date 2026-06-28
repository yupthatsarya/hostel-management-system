#ifndef AUDIT_H
#define AUDIT_H

#include "common.h"

/* Logs an action with a timestamp to the audit file */
void writeAuditLog(const char *username, const char *action);

/* Displays the entire audit trail in a formatted view */
void displayAuditTrail(void);

#endif /* AUDIT_H */
