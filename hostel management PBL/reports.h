#ifndef REPORTS_H
#define REPORTS_H

#include "common.h"

/* Interactive interface to search students by ID, Name, or Room */
void searchStudent(void);

/* Tabular view of all currently allotted students */
void reportAllAllottedStudents(void);

/* Tabular list of all vacant rooms across blocks */
void reportAllVacantRooms(void);

/* List of students with outstanding balances (feePending > 0) */
void reportFeeDefaulters(void);

/* Grouped view of students by course or branch */
void reportStudentsByCourseBranch(void);

#endif /* REPORTS_H */
