#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ANSI Escape Codes for UI Colors */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_WHITE   "\033[1;37m"
#define COLOR_BOLD    "\033[1m"

/* Files */
#define FILE_STUDENTS "students.dat"
#define FILE_ROOMS    "rooms.dat"
#define FILE_CONFIG   "config.dat"
#define FILE_AUDIT    "audit.log"

/* System Parameters */
#define HOSTEL_COUNT 6
#define XOR_KEY 0x5A

/* Student Structure as required by specification */
typedef struct {
  /* Personal Details */
  char studentId[10], name[50], dob[12], gender[10];
  char aadhar[15], bloodGroup[5], religion[20], caste[20];
  char nationality[20], email[50], phone[15];

  /* Academic Details */
  char course[30], branch[30], semester[5], rollNo[15];

  /* Parent Details */
  char fatherName[50], motherName[50];
  char parentPhone[15], parentEmail[50];
  char parentOccupation[30], parentIncome[20];

  /* Guardian Details */
  char guardianName[50], guardianRelation[20];
  char guardianPhone[15], guardianAddress[100];

  /* Address */
  char permanentAddress[100], city[30], state[30], pincode[10];

  /* Room Allotment */
  char hostelName[30], roomNo[10], roomType[15];
  char allotmentDate[12], checkoutDate[12];
  float feePaid, feePending;
  int isAllotted;
} Student;

/* Room Structure */
typedef struct {
  char hostelName[30];
  char roomNo[10];
  char roomType[15];      /* "Single", "Double", "Triple" */
  int capacity;           /* 1, 2, or 3 */
  int occupancy;          /* Current allotted count */
  int isUnderMaintenance; /* 0 = Active, 1 = Maintenance */
} Room;

/* System Configuration Structure */
typedef struct {
  char adminUsername[30];
  char adminPasswordEncrypted[64];
  int roomsPerHostel;     /* Configurable total rooms per hostel block */
  float feeSingle;        /* Base fee for Single occupancy */
  float feeDouble;        /* Base fee for Double occupancy */
  float feeTriple;        /* Base fee for Triple occupancy */
} SystemConfig;

#endif /* COMMON_H */
