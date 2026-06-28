#include "room.h"
#include "utils.h"
#include "audit.h"
#include "student.h"
#include "auth.h"

int vacateRoom(const char *currentAdminUser) {
    char studentId[15];
    Student stu;
    FILE *fpStu;
    FILE *fpRooms;
    char checkoutDate[12];
    int foundStu;
    long stuPos;
    char auditMsg[250];
    
    foundStu = 0;
    stuPos = 0;
    
    printHeader("VACATE ROOM");
    printf("Enter Student ID to Vacate: ");
    getSafeString(studentId, sizeof(studentId));
    
    fpStu = fopen(FILE_STUDENTS, "r+b");
    if (fpStu == NULL) {
        printStatusMessage("Student database is empty.", 0);
        pressAnyKey();
        return 0;
    }
    
    while (fread(&stu, sizeof(Student), 1, fpStu) == 1) {
        if (strcmp(stu.studentId, studentId) == 0) {
            foundStu = 1;
            break;
        }
        stuPos = ftell(fpStu);
    }
    
    if (!foundStu) {
        fclose(fpStu);
        printStatusMessage("Student ID not found!", 0);
        pressAnyKey();
        return 0;
    }
    
    if (!stu.isAllotted) {
        fclose(fpStu);
        printStatusMessage("Student is not currently allotted to any room!", 0);
        pressAnyKey();
        return 0;
    }
    
    do {
        printf("Enter Checkout Date (DD/MM/YYYY): ");
        getSafeString(checkoutDate, sizeof(checkoutDate));
        if (!validateDate(checkoutDate)) {
            printf(COLOR_RED "Invalid date format!" COLOR_RESET "\n");
        }
    } while (!validateDate(checkoutDate));
    
    /* Find room and decrement occupancy */
    fpRooms = fopen(FILE_ROOMS, "r+b");
    if (fpRooms != NULL) {
        Room r;
        long roomPos = 0;
        int roomFound = 0;
        while (fread(&r, sizeof(Room), 1, fpRooms) == 1) {
            if (strcmp(r.hostelName, stu.hostelName) == 0 &&
                strcmp(r.roomNo, stu.roomNo) == 0) {
                roomFound = 1;
                break;
            }
            roomPos = ftell(fpRooms);
        }
        if (roomFound) {
            if (r.occupancy > 0) {
                r.occupancy--;
            }
            fseek(fpRooms, roomPos, SEEK_SET);
            fwrite(&r, sizeof(Room), 1, fpRooms);
        }
        fclose(fpRooms);
    }
    
    /* Log to Audit before clearing student's allotment fields */
    sprintf(auditMsg, "Vacated Room %s in %s. Student: %s (%s). Checkout: %s",
            stu.roomNo, stu.hostelName, stu.name, stu.studentId, checkoutDate);
    writeAuditLog(currentAdminUser, auditMsg);
    
    /* Update Student */
    strcpy(stu.checkoutDate, checkoutDate);
    stu.isAllotted = 0;
    stu.feePaid = 0.0f;
    stu.feePending = 0.0f;
    
    fseek(fpStu, stuPos, SEEK_SET);
    fwrite(&stu, sizeof(Student), 1, fpStu);
    fclose(fpStu);
    
    printStatusMessage("Student vacated from room successfully!", 1);
    pressAnyKey();
    return 1;
}

int transferStudent(const char *currentAdminUser) {
    char studentId[15];
    Student stu;
    SystemConfig config;
    FILE *fpStu;
    FILE *fpRooms;
    int choiceHostel;
    int choiceType;
    char typeSelected[15];
    int i;
    int foundStu;
    long stuPos;
    char oldHostel[30];
    char oldRoomNo[10];
    const char *newHostelName;
    float newTotalFee;
    int availableCount;
    Room r;
    char newRoomNo[10];
    int newRoomFound;
    long newRoomPos;
    Room nr;
    Room or;
    long oldRoomPos;
    int oldRoomFound;
    char auditMsg[250];
    
    foundStu = 0;
    stuPos = 0;
    newTotalFee = 0.0f;
    availableCount = 0;
    newRoomFound = 0;
    newRoomPos = 0;
    oldRoomPos = 0;
    oldRoomFound = 0;
    
    if (!loadConfig(&config)) {
        printStatusMessage("Failed to load configuration.", 0);
        return 0;
    }

    printHeader("ROOM TRANSFER");
    printf("Enter Student ID to Transfer: ");
    getSafeString(studentId, sizeof(studentId));
    
    fpStu = fopen(FILE_STUDENTS, "r+b");
    if (fpStu == NULL) {
        printStatusMessage("Student database is empty.", 0);
        pressAnyKey();
        return 0;
    }
    
    while (fread(&stu, sizeof(Student), 1, fpStu) == 1) {
        if (strcmp(stu.studentId, studentId) == 0) {
            foundStu = 1;
            break;
        }
        stuPos = ftell(fpStu);
    }
    
    if (!foundStu) {
        fclose(fpStu);
        printStatusMessage("Student ID not found!", 0);
        pressAnyKey();
        return 0;
    }
    
    if (!stu.isAllotted) {
        fclose(fpStu);
        printStatusMessage("Student is not currently allotted to any room! Allot a room first.", 0);
        pressAnyKey();
        return 0;
    }
    
    strcpy(oldHostel, stu.hostelName);
    strcpy(oldRoomNo, stu.roomNo);
    
    printf("\nCurrently Allotted: %s, Room %s (%s)\n", oldHostel, oldRoomNo, stu.roomType);
    
    /* Choose new Block */
    printf("\nSelect NEW Dynasty Hostel Block:\n");
    for (i = 0; i < HOSTEL_COUNT; i++) {
        printf("%d. %s\n", i + 1, HOSTELS[i]);
    }
    printf("Select Option (1-6): ");
    choiceHostel = getSafeInt();
    if (choiceHostel < 1 || choiceHostel > 6) {
        fclose(fpStu);
        printStatusMessage("Invalid block selection!", 0);
        pressAnyKey();
        return 0;
    }
    newHostelName = HOSTELS[choiceHostel - 1];
    
    /* Choose new Type */
    printf("\nSelect NEW Room Occupancy Type:\n");
    printf("1. Single (Fee: INR %.2f)\n", config.feeSingle);
    printf("2. Double (Fee: INR %.2f)\n", config.feeDouble);
    printf("3. Triple (Fee: INR %.2f)\n", config.feeTriple);
    printf("Select Option (1-3): ");
    choiceType = getSafeInt();
    
    if (choiceType == 1) {
        strcpy(typeSelected, "Single");
        newTotalFee = config.feeSingle;
    } else if (choiceType == 2) {
        strcpy(typeSelected, "Double");
        newTotalFee = config.feeDouble;
    } else if (choiceType == 3) {
        strcpy(typeSelected, "Triple");
        newTotalFee = config.feeTriple;
    } else {
        fclose(fpStu);
        printStatusMessage("Invalid room type choice!", 0);
        pressAnyKey();
        return 0;
    }
    
    /* Open rooms.dat to check and list vacancies */
    fpRooms = fopen(FILE_ROOMS, "r+b");
    if (fpRooms == NULL) {
        fclose(fpStu);
        printStatusMessage("Rooms database file not found!", 0);
        pressAnyKey();
        return 0;
    }
    
    printf("\nAvailable %s Rooms in %s:\n", typeSelected, newHostelName);
    printf("%-10s | %-12s | %-8s | %-8s\n", "Room No.", "Type", "Capacity", "Occupied");
    printDivider('-', 45);
    
    while (fread(&r, sizeof(Room), 1, fpRooms) == 1) {
        if (strcmp(r.hostelName, newHostelName) == 0 &&
            strcmp(r.roomType, typeSelected) == 0 &&
            r.occupancy < r.capacity && r.isUnderMaintenance == 0) {
            printf("%-10s | %-12s | %-8d | %-8d\n", r.roomNo, r.roomType, r.capacity, r.occupancy);
            availableCount++;
        }
    }
    
    if (availableCount == 0) {
        fclose(fpStu);
        fclose(fpRooms);
        printStatusMessage("No vacant rooms of that type available in the chosen block!", 0);
        pressAnyKey();
        return 0;
    }
    
    printf("\nEnter NEW Room Number: ");
    getSafeString(newRoomNo, sizeof(newRoomNo));
    
    /* Find new room */
    fseek(fpRooms, 0, SEEK_SET);
    while (fread(&nr, sizeof(Room), 1, fpRooms) == 1) {
        if (strcmp(nr.hostelName, newHostelName) == 0 &&
            strcmp(nr.roomNo, newRoomNo) == 0) {
            newRoomFound = 1;
            break;
        }
        newRoomPos = ftell(fpRooms);
    }
    
    if (!newRoomFound) {
        fclose(fpStu);
        fclose(fpRooms);
        printStatusMessage("Selected room number does not exist!", 0);
        pressAnyKey();
        return 0;
    }
    
    if (strcmp(nr.roomType, typeSelected) != 0 || nr.isUnderMaintenance || nr.occupancy >= nr.capacity) {
        fclose(fpStu);
        fclose(fpRooms);
        printStatusMessage("Selected room is not valid, is full, or is under maintenance!", 0);
        pressAnyKey();
        return 0;
    }
    
    /* Update old room occupancy */
    fseek(fpRooms, 0, SEEK_SET);
    while (fread(&or, sizeof(Room), 1, fpRooms) == 1) {
        if (strcmp(or.hostelName, oldHostel) == 0 &&
            strcmp(or.roomNo, oldRoomNo) == 0) {
            oldRoomFound = 1;
            break;
        }
        oldRoomPos = ftell(fpRooms);
    }
    
    if (oldRoomFound) {
        if (or.occupancy > 0) or.occupancy--;
        fseek(fpRooms, oldRoomPos, SEEK_SET);
        fwrite(&or, sizeof(Room), 1, fpRooms);
    }
    
    /* Update new room occupancy */
    nr.occupancy++;
    fseek(fpRooms, newRoomPos, SEEK_SET);
    fwrite(&nr, sizeof(Room), 1, fpRooms);
    fclose(fpRooms);
    
    /* Update Student record */
    strcpy(stu.hostelName, newHostelName);
    strcpy(stu.roomNo, newRoomNo);
    strcpy(stu.roomType, typeSelected);
    
    /* Adjust fee balances */
    stu.feePending = newTotalFee - stu.feePaid;
    if (stu.feePending < 0) {
        printf(COLOR_YELLOW "Note: Student has a credit surplus of INR %.2f due to downgrade." COLOR_RESET "\n", -stu.feePending);
    }
    
    fseek(fpStu, stuPos, SEEK_SET);
    fwrite(&stu, sizeof(Student), 1, fpStu);
    fclose(fpStu);
    
    /* Write to Audit */
    sprintf(auditMsg, "Transferred Student %s (%s) from %s Room %s to %s Room %s",
            stu.name, stu.studentId, oldHostel, oldRoomNo, newHostelName, newRoomNo);
    writeAuditLog(currentAdminUser, auditMsg);
    
    printStatusMessage("Student transferred successfully!", 1);
    pressAnyKey();
    return 1;
}

void viewRoomOccupancy(void) {
    FILE *fp = fopen(FILE_ROOMS, "rb");
    int h;
    Room r;
    int totalRooms;
    int occupiedBeds;
    int capacityBeds;
    char statusStr[50];
    
    if (fp == NULL) {
        printf("\n" COLOR_RED "Rooms database not initialized!" COLOR_RESET "\n");
        return;
    }
    
    printHeader("ROOM OCCUPANCY REGISTER");
    
    for (h = 0; h < HOSTEL_COUNT; h++) {
        fseek(fp, 0, SEEK_SET);
        totalRooms = 0;
        occupiedBeds = 0;
        capacityBeds = 0;
        
        printf("\n" COLOR_BOLD "=== %s ===" COLOR_RESET "\n", HOSTELS[h]);
        printf("%-10s | %-12s | %-10s | %-10s | %-15s\n", "Room No.", "Type", "Capacity", "Occupied", "Status");
        printDivider('-', 65);
        
        while (fread(&r, sizeof(Room), 1, fp) == 1) {
            if (strcmp(r.hostelName, HOSTELS[h]) == 0) {
                totalRooms++;
                capacityBeds += r.capacity;
                occupiedBeds += r.occupancy;
                
                if (r.isUnderMaintenance) {
                    strcpy(statusStr, COLOR_RED "Maintenance" COLOR_RESET);
                } else if (r.occupancy >= r.capacity) {
                    strcpy(statusStr, COLOR_YELLOW "Full" COLOR_RESET);
                } else {
                    strcpy(statusStr, COLOR_GREEN "Vacant" COLOR_RESET);
                }
                
                printf("%-10s | %-12s | %-10d | %-10d | %-15s\n",
                       r.roomNo, r.roomType, r.capacity, r.occupancy, statusStr);
            }
        }
        
        printf("\nSummary for %s: Total Rooms: %d | Beds Occupied: %d/%d (%.1f%%)\n",
               HOSTELS[h], totalRooms, occupiedBeds, capacityBeds, 
               capacityBeds > 0 ? (float)occupiedBeds * 100.0f / (float)capacityBeds : 0.0f);
        printDivider('=', 65);
    }
    
    fclose(fp);
    pressAnyKey();
}

void viewStudentsByBlock(void) {
    int choice;
    int i;
    FILE *fp;
    Student stu;
    int count;
    const char *selectedHostel;
    
    count = 0;
    
    printHeader("VIEW BLOCK RESIDENTS");
    printf("Select Hostel Block:\n");
    for (i = 0; i < HOSTEL_COUNT; i++) {
        printf("%d. %s\n", i + 1, HOSTELS[i]);
    }
    printf("Select Option (1-6): ");
    choice = getSafeInt();
    if (choice < 1 || choice > 6) {
        printStatusMessage("Invalid selection!", 0);
        pressAnyKey();
        return;
    }
    selectedHostel = HOSTELS[choice - 1];
    
    fp = fopen(FILE_STUDENTS, "rb");
    if (fp == NULL) {
        printStatusMessage("No students registered in system.", 0);
        pressAnyKey();
        return;
    }
    
    printHeader(selectedHostel);
    printf("%-10s | %-20s | %-8s | %-10s | %-12s | %-15s\n", 
           "ID", "Name", "Room No", "Room Type", "Course", "Phone");
    printDivider('-', 85);
    
    while (fread(&stu, sizeof(Student), 1, fp) == 1) {
        if (stu.isAllotted && strcmp(stu.hostelName, selectedHostel) == 0) {
            printf("%-10s | %-20s | %-8s | %-10s | %-12s | %-15s\n",
                   stu.studentId, stu.name, stu.roomNo, stu.roomType, stu.course, stu.phone);
            count++;
        }
    }
    
    printDivider('-', 85);
    printf("Total allotted residents found: %d\n", count);
    fclose(fp);
    pressAnyKey();
}

int manageRoomMaintenance(const char *currentAdminUser) {
    int choiceHostel;
    char roomNo[10];
    FILE *fp;
    Room r;
    long roomPos;
    int found;
    int i;
    const char *selectedHostel;
    char confirm[10];
    
    roomPos = 0;
    found = 0;
    
    printHeader("ROOM MAINTENANCE CONTROLLER");
    printf("Select Hostel Block:\n");
    for (i = 0; i < HOSTEL_COUNT; i++) {
        printf("%d. %s\n", i + 1, HOSTELS[i]);
    }
    printf("Select Option (1-6): ");
    choiceHostel = getSafeInt();
    if (choiceHostel < 1 || choiceHostel > 6) {
        printStatusMessage("Invalid selection!", 0);
        pressAnyKey();
        return 0;
    }
    selectedHostel = HOSTELS[choiceHostel - 1];
    
    printf("Enter Room Number: ");
    getSafeString(roomNo, sizeof(roomNo));
    
    fp = fopen(FILE_ROOMS, "r+b");
    if (fp == NULL) {
        printStatusMessage("Rooms database not found!", 0);
        pressAnyKey();
        return 0;
    }
    
    while (fread(&r, sizeof(Room), 1, fp) == 1) {
        if (strcmp(r.hostelName, selectedHostel) == 0 &&
            strcmp(r.roomNo, roomNo) == 0) {
            found = 1;
            break;
        }
        roomPos = ftell(fp);
    }
    
    if (!found) {
        fclose(fp);
        printStatusMessage("Room not found in selected block!", 0);
        pressAnyKey();
        return 0;
    }
    
    if (r.isUnderMaintenance) {
        printf("\nRoom %s in %s is currently " COLOR_RED "Under Maintenance" COLOR_RESET ".\n", r.roomNo, r.hostelName);
        printf("Return room to ACTIVE service? (Y/N): ");
        getSafeString(confirm, sizeof(confirm));
        if (confirm[0] == 'y' || confirm[0] == 'Y') {
            char logMsg[100];
            r.isUnderMaintenance = 0;
            fseek(fp, roomPos, SEEK_SET);
            fwrite(&r, sizeof(Room), 1, fp);
            
            sprintf(logMsg, "Returned Room %s in %s to ACTIVE service.", r.roomNo, r.hostelName);
            writeAuditLog(currentAdminUser, logMsg);
            printStatusMessage("Room status updated to ACTIVE successfully!", 1);
        } else {
            printStatusMessage("Operation aborted.", 1);
        }
    } else {
        char logMsg[100];
        printf("\nRoom %s in %s is currently " COLOR_GREEN "ACTIVE" COLOR_RESET ".\n", r.roomNo, r.hostelName);
        printf("Mark room under maintenance? (Y/N): ");
        getSafeString(confirm, sizeof(confirm));
        if (confirm[0] == 'y' || confirm[0] == 'Y') {
            if (r.occupancy > 0) {
                char warn[10];
                printf(COLOR_RED "WARNING: This room has %d occupant(s)!" COLOR_RESET "\n", r.occupancy);
                printf("Proceeding will lock them in a maintenance room. Proceed anyway? (Y/N): ");
                getSafeString(warn, sizeof(warn));
                if (warn[0] != 'y' && warn[0] != 'Y') {
                    fclose(fp);
                    printStatusMessage("Maintenance request cancelled. Transfer residents first.", 1);
                    pressAnyKey();
                    return 0;
                }
            }
            r.isUnderMaintenance = 1;
            fseek(fp, roomPos, SEEK_SET);
            fwrite(&r, sizeof(Room), 1, fp);
            
            sprintf(logMsg, "Placed Room %s in %s UNDER MAINTENANCE.", r.roomNo, r.hostelName);
            writeAuditLog(currentAdminUser, logMsg);
            printStatusMessage("Room status updated to UNDER MAINTENANCE successfully!", 1);
        } else {
            printStatusMessage("Operation aborted.", 1);
        }
    }
    
    fclose(fp);
    pressAnyKey();
    return 1;
}
