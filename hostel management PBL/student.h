#ifndef STUDENT_H
#define STUDENT_H

#include "common.h"

/* Registers a new student, performs validations, and persists record */
int registerStudent(const char *currentAdminUser);

/* Utility to check if a student exists by ID and load their record */
int findStudentById(const char *id, Student *foundStudent);

#endif /* STUDENT_H */
