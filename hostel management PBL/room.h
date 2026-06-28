#ifndef ROOM_H
#define ROOM_H

#include "common.h"

/* Vacates a student from their currently allotted room */
int vacateRoom(const char *currentAdminUser);

/* Transfers a student to another room or hostel block */
int transferStudent(const char *currentAdminUser);

/* Displays detailed room occupancy across all blocks */
void viewRoomOccupancy(void);

/* Views all students in a selected hostel block */
void viewStudentsByBlock(void);

/* Marks a room under maintenance or returns it to service */
int manageRoomMaintenance(const char *currentAdminUser);

#endif /* ROOM_H */
