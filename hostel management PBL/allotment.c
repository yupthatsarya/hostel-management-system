#include "allotment.h"
#include "utils.h"
#include "audit.h"
#include "auth.h"

void displayAvailableRooms(void) {
    FILE *fp = fopen(FILE_ROOMS, "rb");
    int h;
    Room r;
    int found;
    
    if (fp == NULL) {
        printf("\n" COLOR_RED "Rooms database not initialized! Contact administrator." COLOR_RESET "\n");
        return;
    }
    
    printf("\n" COLOR_BOLD "=== LIST OF AVAILABLE ROOMS ===" COLOR_RESET "\n");
    
    for (h = 0; h < HOSTEL_COUNT; h++) {
        fseek(fp, 0, SEEK_SET);
        found = 0;
        
        printf("\n" COLOR_CYAN "%s" COLOR_RESET "\n", HOSTELS[h]);
        printf("%-10s | %-12s | %-10s | %-10s | %-12s\n", "Room No.", "Type", "Capacity", "Occupancy", "Status");
        printDivider('-', 62);
        
        while (fread(&r, sizeof(Room), 1, fp) == 1) {
            if (strcmp(r.hostelName, HOSTELS[h]) == 0) {
                if (r.occupancy < r.capacity && r.isUnderMaintenance == 0) {
                    printf("%-10s | %-12s | %-10d | %-10d | " COLOR_GREEN "%-12s" COLOR_RESET "\n",
                           r.roomNo, r.roomType, r.capacity, r.occupancy, "Available");
                    found++;
                }
            }
        }
        if (found == 0) {
            printf(COLOR_YELLOW "No rooms available in this block." COLOR_RESET "\n");
        }
    }
    fclose(fp);
}

int allotRoom(const char *currentAdminUser) {
    char studentId[15];
    Student stu;
    SystemConfig config;
    FILE *fpStu, *fpRooms;
    int choiceHostel;
    char typeSelected[15];
    int choiceType;
    int i;
    int foundStu;
    long stuPos;
    const char *selectedHostel;
    float totalFee;
    int availableCount;
    Room r;
    char selectedRoomNo[10];
    int roomFound;
    long roomPos;
    char dateAllotted[12];
    float feePaid;
    float feePending;
    char logMsg[120];
    
    foundStu = 0;
    stuPos = 0;
    totalFee = 0.0f;
    availableCount = 0;
    roomFound = 0;
    roomPos = 0;
    feePaid = -1.0f;

    if (!loadConfig(&config)) {
        printStatusMessage("Failed to load configuration. Run system setup first.", 0);
        return 0;
    }

    printHeader("ROOM ALLOTMENT");
    printf("Enter Student ID to Allot: ");
    getSafeString(studentId, sizeof(studentId));
    
    /* Find Student */
    fpStu = fopen(FILE_STUDENTS, "r+b");
    if (fpStu == NULL) {
        printStatusMessage("Student database is empty. Register students first.", 0);
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
        printStatusMessage("Student ID not found in database!", 0);
        pressAnyKey();
        return 0;
    }
    
    if (stu.isAllotted) {
        fclose(fpStu);
        printf(COLOR_YELLOW "\nStudent is already allotted to %s, Room %s." COLOR_RESET "\n", stu.hostelName, stu.roomNo);
        pressAnyKey();
        return 0;
    }
    
    /* Choose Hostel */
    printf("\nSelect Dynasty Hostel Block:\n");
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
    selectedHostel = HOSTELS[choiceHostel - 1];
    
    /* Choose Room Type */
    printf("\nSelect Room Occupancy Type:\n");
    printf("1. Single (Fee: INR %.2f)\n", config.feeSingle);
    printf("2. Double (Fee: INR %.2f)\n", config.feeDouble);
    printf("3. Triple (Fee: INR %.2f)\n", config.feeTriple);
    printf("Select Option (1-3): ");
    choiceType = getSafeInt();
    
    if (choiceType == 1) {
        strcpy(typeSelected, "Single");
        totalFee = config.feeSingle;
    } else if (choiceType == 2) {
        strcpy(typeSelected, "Double");
        totalFee = config.feeDouble;
    } else if (choiceType == 3) {
        strcpy(typeSelected, "Triple");
        totalFee = config.feeTriple;
    } else {
        fclose(fpStu);
        printStatusMessage("Invalid room type choice!", 0);
        pressAnyKey();
        return 0;
    }
    
    /* Show matching available rooms */
    fpRooms = fopen(FILE_ROOMS, "r+b");
    if (fpRooms == NULL) {
        fclose(fpStu);
        printStatusMessage("Rooms database file not found!", 0);
        pressAnyKey();
        return 0;
    }
    
    printf("\nAvailable %s Rooms in %s:\n", typeSelected, selectedHostel);
    printf("%-10s | %-12s | %-8s | %-8s\n", "Room No.", "Type", "Capacity", "Occupied");
    printDivider('-', 45);
    
    while (fread(&r, sizeof(Room), 1, fpRooms) == 1) {
        if (strcmp(r.hostelName, selectedHostel) == 0 &&
            strcmp(r.roomType, typeSelected) == 0 &&
            r.occupancy < r.capacity && r.isUnderMaintenance == 0) {
            printf("%-10s | %-12s | %-8d | %-8d\n", r.roomNo, r.roomType, r.capacity, r.occupancy);
            availableCount++;
        }
    }
    
    if (availableCount == 0) {
        fclose(fpStu);
        fclose(fpRooms);
        printStatusMessage("No vacant rooms of this type in the selected block!", 0);
        pressAnyKey();
        return 0;
    }
    
    /* Select Room Number */
    printf("\nEnter Room Number to Allot: ");
    getSafeString(selectedRoomNo, sizeof(selectedRoomNo));
    
    /* Search and update the room */
    fseek(fpRooms, 0, SEEK_SET);
    while (fread(&r, sizeof(Room), 1, fpRooms) == 1) {
        if (strcmp(r.hostelName, selectedHostel) == 0 &&
            strcmp(r.roomNo, selectedRoomNo) == 0) {
            roomFound = 1;
            break;
        }
        roomPos = ftell(fpRooms);
    }
    
    if (!roomFound) {
        fclose(fpStu);
        fclose(fpRooms);
        printStatusMessage("Room number does not exist in this block!", 0);
        pressAnyKey();
        return 0;
    }
    
    if (strcmp(r.roomType, typeSelected) != 0) {
        fclose(fpStu);
        fclose(fpRooms);
        printStatusMessage("Room type mismatch! Select a room of the chosen occupancy.", 0);
        pressAnyKey();
        return 0;
    }
    
    if (r.isUnderMaintenance) {
        fclose(fpStu);
        fclose(fpRooms);
        printStatusMessage("Room is currently under maintenance!", 0);
        pressAnyKey();
        return 0;
    }
    
    if (r.occupancy >= r.capacity) {
        fclose(fpStu);
        fclose(fpRooms);
        printStatusMessage("Room is fully occupied!", 0);
        pressAnyKey();
        return 0;
    }
    
    /* Prompt Allotment Date & Fees */
    do {
        printf("Enter Allotment Date (DD/MM/YYYY): ");
        getSafeString(dateAllotted, sizeof(dateAllotted));
        if (!validateDate(dateAllotted)) {
            printf(COLOR_RED "Invalid date format!" COLOR_RESET "\n");
        }
    } while (!validateDate(dateAllotted));
    
    do {
        printf("Total Room Fee: INR %.2f\n", totalFee);
        printf("Enter Fee Amount Paid (INR): ");
        feePaid = getSafeFloat();
        if (feePaid < 0.0f || feePaid > totalFee) {
            printf(COLOR_RED "Invalid amount! Fee paid must be between 0 and %.2f." COLOR_RESET "\n", totalFee);
            feePaid = -1.0f;
        }
    } while (feePaid < 0.0f);
    
    feePending = totalFee - feePaid;
    
    /* Update Room in rooms.dat */
    r.occupancy++;
    fseek(fpRooms, roomPos, SEEK_SET);
    fwrite(&r, sizeof(Room), 1, fpRooms);
    fclose(fpRooms);
    
    /* Update Student in students.dat */
    strcpy(stu.hostelName, selectedHostel);
    strcpy(stu.roomNo, selectedRoomNo);
    strcpy(stu.roomType, typeSelected);
    strcpy(stu.allotmentDate, dateAllotted);
    strcpy(stu.checkoutDate, ""); /* Empty until vacate */
    stu.feePaid = feePaid;
    stu.feePending = feePending;
    stu.isAllotted = 1;
    
    fseek(fpStu, stuPos, SEEK_SET);
    fwrite(&stu, sizeof(Student), 1, fpStu);
    fclose(fpStu);
    
    /* Log to Audit */
    sprintf(logMsg, "Allotted Room %s (%s) in %s to student %s (%s)", 
            selectedRoomNo, typeSelected, selectedHostel, stu.name, stu.studentId);
    writeAuditLog(currentAdminUser, logMsg);
    
    printStatusMessage("Room allotted and transaction saved successfully!", 1);
    printf("\nReceipt Summary:\n");
    printf("Student Name: %s\n", stu.name);
    printf("Block       : %s | Room: %s (%s)\n", selectedHostel, selectedRoomNo, typeSelected);
    printf("Amount Paid : INR %.2f | Amount Pending: INR %.2f\n", feePaid, feePending);
    pressAnyKey();
    
    return 1;
}
