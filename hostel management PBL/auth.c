#include "auth.h"
#include "utils.h"
#include "audit.h"

/* Forward declarations of local helpers */
static void resetRooms(int roomsCount);

int loadConfig(SystemConfig *config) {
    FILE *fp = fopen(FILE_CONFIG, "rb");
    if (fp == NULL) {
        return 0;
    }
    if (fread(config, sizeof(SystemConfig), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

int saveConfig(const SystemConfig *config) {
    FILE *fp = fopen(FILE_CONFIG, "wb");
    if (fp == NULL) {
        return 0;
    }
    if (fwrite(config, sizeof(SystemConfig), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

int initializeSystem(void) {
    SystemConfig config;
    FILE *fp;
    
    fp = fopen(FILE_CONFIG, "rb");
    if (fp == NULL) {
        /* Config doesn't exist, create defaults */
        strcpy(config.adminUsername, "admin");
        strcpy(config.adminPasswordEncrypted, "password123");
        encryptDecrypt(config.adminPasswordEncrypted, strlen(config.adminPasswordEncrypted));
        config.roomsPerHostel = 10;
        config.feeSingle = 50000.0f;
        config.feeDouble = 35000.0f;
        config.feeTriple = 25000.0f;
        
        if (!saveConfig(&config)) {
            return 0;
        }
        
        /* Seed the database with initial mock data */
        seedMockData(config.roomsPerHostel);
        
        writeAuditLog("SYSTEM", "Initialized default configuration and preloaded databases with test data.");
    } else {
        fclose(fp);
    }
    return 1;
}

int adminLogin(char *currentAdminUser) {
    char username[30];
    char password[30];
    char encryptedPass[30];
    SystemConfig config;

    if (!loadConfig(&config)) {
        printStatusMessage("Configuration load failed. Initializing default settings first.", 0);
        initializeSystem();
        if (!loadConfig(&config)) return 0;
    }

    printf("\n" COLOR_BOLD "--- ADMIN PANEL LOGIN ---" COLOR_RESET "\n");
    printf("Enter Admin Username: ");
    getSafeString(username, sizeof(username));
    
    printf("Enter Admin Password: ");
    getMaskedPassword(password, sizeof(password));

    strcpy(encryptedPass, password);
    encryptDecrypt(encryptedPass, strlen(encryptedPass));

    if (strcmp(username, config.adminUsername) == 0 &&
        strcmp(encryptedPass, config.adminPasswordEncrypted) == 0) {
        strcpy(currentAdminUser, username);
        writeAuditLog(username, "Logged into administrative interface.");
        printStatusMessage("Login Successful!", 1);
        return 1;
    }

    writeAuditLog("SYSTEM", "Unauthorized login attempt blocked.");
    printStatusMessage("Invalid Username or Password!", 0);
    return 0;
}

int changeAdminPassword(const char *currentAdminUser) {
    char currentPass[30];
    char currentPassEnc[30];
    char newPass[30];
    char confirmPass[30];
    SystemConfig config;

    if (!loadConfig(&config)) {
        printStatusMessage("Failed to load configuration.", 0);
        return 0;
    }

    printf("\n" COLOR_BOLD "--- CHANGE ADMIN PASSWORD ---" COLOR_RESET "\n");
    printf("Enter Current Password: ");
    getMaskedPassword(currentPass, sizeof(currentPass));

    strcpy(currentPassEnc, currentPass);
    encryptDecrypt(currentPassEnc, strlen(currentPassEnc));

    if (strcmp(currentPassEnc, config.adminPasswordEncrypted) != 0) {
        printStatusMessage("Incorrect current password!", 0);
        return 0;
    }

    printf("Enter New Password: ");
    getMaskedPassword(newPass, sizeof(newPass));
    
    if (strlen(newPass) < 6) {
        printStatusMessage("Password must be at least 6 characters long!", 0);
        return 0;
    }

    printf("Confirm New Password: ");
    getMaskedPassword(confirmPass, sizeof(confirmPass));

    if (strcmp(newPass, confirmPass) != 0) {
        printStatusMessage("Passwords do not match!", 0);
        return 0;
    }

    strcpy(config.adminPasswordEncrypted, newPass);
    encryptDecrypt(config.adminPasswordEncrypted, strlen(config.adminPasswordEncrypted));

    if (saveConfig(&config)) {
        writeAuditLog(currentAdminUser, "Successfully changed password.");
        printStatusMessage("Password updated successfully!", 1);
        return 1;
    }

    printStatusMessage("Error saving configuration changes.", 0);
    return 0;
}

int updateSystemConfig(const char *currentAdminUser) {
    SystemConfig config;
    int opt;
    char choice[10];
    int newCount;
    FILE *sFp;
    Student stu;
    float fS, fD, fT;

    if (!loadConfig(&config)) {
        printStatusMessage("Failed to load configuration.", 0);
        return 0;
    }

    printHeader("SYSTEM CONFIGURATION");
    printf("1. View Current Configuration\n");
    printf("2. Change Total Rooms per Hostel Block\n");
    printf("3. Modify Room Fee Structure\n");
    printf("0. Return to Admin Menu\n");
    printf("\nSelect Option: ");
    opt = getSafeInt();

    switch (opt) {
        case 1:
            printHeader("CURRENT SYSTEM CONFIGURATION");
            printf("Admin Username        : %s\n", config.adminUsername);
            printf("Rooms per Hostel Block: %d\n", config.roomsPerHostel);
            printf("Single Room Fee       : INR %.2f\n", config.feeSingle);
            printf("Double Room Fee       : INR %.2f\n", config.feeDouble);
            printf("Triple Room Fee       : INR %.2f\n", config.feeTriple);
            pressAnyKey();
            break;

        case 2:
            printf("\n" COLOR_RED "WARNING: Changing rooms per block will reset all room vacancy details!" COLOR_RESET "\n");
            printf("Are you sure you want to proceed? (Y/N): ");
            getSafeString(choice, sizeof(choice));
            if (choice[0] == 'y' || choice[0] == 'Y') {
                printf("Enter new total rooms per block (1 - 100): ");
                newCount = getSafeInt();
                if (newCount < 1 || newCount > 100) {
                    printStatusMessage("Invalid count range!", 0);
                    break;
                }
                config.roomsPerHostel = newCount;
                if (saveConfig(&config)) {
                    resetRooms(newCount);
                    /* Reset student allotment flag since old rooms are deleted */
                    sFp = fopen(FILE_STUDENTS, "r+b");
                    if (sFp != NULL) {
                        while (fread(&stu, sizeof(Student), 1, sFp) == 1) {
                            stu.isAllotted = 0;
                            strcpy(stu.hostelName, "");
                            strcpy(stu.roomNo, "");
                            stu.feePaid = 0.0f;
                            stu.feePending = 0.0f;
                            fseek(sFp, -((long)sizeof(Student)), SEEK_CUR);
                            fwrite(&stu, sizeof(Student), 1, sFp);
                            fflush(sFp);
                        }
                        fclose(sFp);
                    }
                    writeAuditLog(currentAdminUser, "Resized block capacities, all room assignments cleared.");
                    printStatusMessage("Total rooms updated and database re-initialized successfully!", 1);
                } else {
                    printStatusMessage("Failed to save changes.", 0);
                }
            } else {
                printStatusMessage("Operation cancelled.", 1);
            }
            pressAnyKey();
            break;

        case 3:
            printf("\n" COLOR_BOLD "--- MODIFY FEE STRUCTURE ---" COLOR_RESET "\n");
            printf("Enter New Single Room Fee (Current: %.2f): ", config.feeSingle);
            fS = getSafeFloat();
            printf("Enter New Double Room Fee (Current: %.2f): ", config.feeDouble);
            fD = getSafeFloat();
            printf("Enter New Triple Room Fee (Current: %.2f): ", config.feeTriple);
            fT = getSafeFloat();

            if (fS < 0 || fD < 0 || fT < 0) {
                printStatusMessage("Fees cannot be negative!", 0);
                break;
            }

            config.feeSingle = fS;
            config.feeDouble = fD;
            config.feeTriple = fT;

            if (saveConfig(&config)) {
                writeAuditLog(currentAdminUser, "Updated hostel fee structure.");
                printStatusMessage("Fee structure updated successfully!", 1);
            } else {
                printStatusMessage("Failed to save fee settings.", 0);
            }
            pressAnyKey();
            break;

        case 0:
            return 1;
        default:
            printStatusMessage("Invalid choice!", 0);
            pressAnyKey();
            break;
    }
    return 1;
}

int resetSystemData(const char *currentAdminUser) {
    char confirm[10];
    FILE *fpStu;
    SystemConfig config;
    FILE *fpAudit;

    printf("\n" COLOR_RED COLOR_BOLD "CRITICAL: You are about to wipe all student and room databases!" COLOR_RESET "\n");
    printf("Confirm wipe? Type 'WIPE': ");
    getSafeString(confirm, sizeof(confirm));

    if (strcmp(confirm, "WIPE") == 0) {
        /* Truncate students file */
        fpStu = fopen(FILE_STUDENTS, "wb");
        if (fpStu) fclose(fpStu);

        /* Re-initialize config and rooms */
        strcpy(config.adminUsername, "admin");
        strcpy(config.adminPasswordEncrypted, "password123");
        encryptDecrypt(config.adminPasswordEncrypted, strlen(config.adminPasswordEncrypted));
        config.roomsPerHostel = 10;
        config.feeSingle = 50000.0f;
        config.feeDouble = 35000.0f;
        config.feeTriple = 25000.0f;
        
        saveConfig(&config);
        seedMockData(config.roomsPerHostel);

        /* Delete audit logs and write starting fresh */
        fpAudit = fopen(FILE_AUDIT, "w");
        if (fpAudit) fclose(fpAudit);

        writeAuditLog("SYSTEM", "Global system database reset performed.");
        printStatusMessage("All data wiped successfully. Credentials reset to default admin/password123.", 1);
        pressAnyKey();
        return 1;
    }

    /* Reference currentAdminUser to avoid unused parameter warning */
    (void)currentAdminUser;
    
    printStatusMessage("Reset operation aborted.", 1);
    pressAnyKey();
    return 0;
}

/* Local helper function to initialize/overwrite the rooms database */
static void resetRooms(int roomsCount) {
    FILE *fp = fopen(FILE_ROOMS, "wb");
    int h, r;
    int pct;
    
    if (fp == NULL) return;

    for (h = 0; h < HOSTEL_COUNT; h++) {
        for (r = 1; r <= roomsCount; r++) {
            Room room;
            strcpy(room.hostelName, HOSTELS[h]);
            sprintf(room.roomNo, "%d", 100 + r);
            
            /* Determine capacity and type */
            pct = (r * 100) / roomsCount;
            if (pct <= 30) {
                strcpy(room.roomType, "Single");
                room.capacity = 1;
            } else if (pct <= 70) {
                strcpy(room.roomType, "Double");
                room.capacity = 2;
            } else {
                strcpy(room.roomType, "Triple");
                room.capacity = 3;
            }
            room.occupancy = 0;
            room.isUnderMaintenance = 0;
            
            fwrite(&room, sizeof(Room), 1, fp);
        }
    }
    fclose(fp);
}

/* Local helper function to seed default mock records */
void seedMockData(int roomsCount) {
    FILE *fpRooms;
    FILE *fpStudents;
    FILE *fpAudit;
    int h, r;
    int pct;

    /* 1. Seed Rooms */
    fpRooms = fopen(FILE_ROOMS, "wb");
    if (fpRooms != NULL) {
        for (h = 0; h < HOSTEL_COUNT; h++) {
            for (r = 1; r <= roomsCount; r++) {
                Room room;
                strcpy(room.hostelName, HOSTELS[h]);
                sprintf(room.roomNo, "%d", 100 + r);
                
                pct = (r * 100) / roomsCount;
                if (pct <= 30) {
                    strcpy(room.roomType, "Single");
                    room.capacity = 1;
                } else if (pct <= 70) {
                    strcpy(room.roomType, "Double");
                    room.capacity = 2;
                } else {
                    strcpy(room.roomType, "Triple");
                    room.capacity = 3;
                }
                room.occupancy = 0;
                room.isUnderMaintenance = 0;
                
                /* Test case maintenance */
                if (h == 0 && r == 3) {
                    room.isUnderMaintenance = 1;
                }
                
                /* Seed occupancies:
                   - STU-001: Maurya Block 101
                   - STU-002: Chola Block 104
                   - STU-003: Gupta Block 108
                   - STU-005: Mughal Block 102
                */
                if (h == 0 && r == 1) room.occupancy = 1;
                if (h == 1 && r == 4) room.occupancy = 1;
                if (h == 2 && r == 8) room.occupancy = 1;
                if (h == 3 && r == 2) room.occupancy = 1;
                
                fwrite(&room, sizeof(Room), 1, fpRooms);
            }
        }
        fclose(fpRooms);
    }

    /* 2. Seed Students */
    fpStudents = fopen(FILE_STUDENTS, "wb");
    if (fpStudents != NULL) {
        Student s[5];
        memset(s, 0, sizeof(s));

        /* Student 1 */
        strcpy(s[0].studentId, "STU-001");
        strcpy(s[0].name, "Rahul Sharma");
        strcpy(s[0].dob, "15/08/2004");
        strcpy(s[0].gender, "Male");
        strcpy(s[0].aadhar, "452136985214");
        strcpy(s[0].bloodGroup, "A+");
        strcpy(s[0].religion, "Hindu");
        strcpy(s[0].caste, "General");
        strcpy(s[0].nationality, "Indian");
        strcpy(s[0].email, "rahul.sharma@gmail.com");
        strcpy(s[0].phone, "9876543210");
        strcpy(s[0].course, "B.Tech");
        strcpy(s[0].branch, "CSE");
        strcpy(s[0].semester, "III");
        strcpy(s[0].rollNo, "2024CSE001");
        strcpy(s[0].fatherName, "Vijay Sharma");
        strcpy(s[0].motherName, "Rekha Sharma");
        strcpy(s[0].parentPhone, "9876543211");
        strcpy(s[0].parentEmail, "vijay.sharma@gmail.com");
        strcpy(s[0].parentOccupation, "Business");
        strcpy(s[0].parentIncome, "1200000");
        strcpy(s[0].guardianName, "NA");
        strcpy(s[0].guardianRelation, "NA");
        strcpy(s[0].guardianPhone, "0000000000");
        strcpy(s[0].guardianAddress, "NA");
        strcpy(s[0].permanentAddress, "12, Saket Extension");
        strcpy(s[0].city, "New Delhi");
        strcpy(s[0].state, "Delhi");
        strcpy(s[0].pincode, "110017");
        strcpy(s[0].hostelName, "Maurya Block");
        strcpy(s[0].roomNo, "101");
        strcpy(s[0].roomType, "Single");
        strcpy(s[0].allotmentDate, "10/07/2025");
        s[0].feePaid = 50000.0f;
        s[0].feePending = 0.0f;
        s[0].isAllotted = 1;

        /* Student 2 */
        strcpy(s[1].studentId, "STU-002");
        strcpy(s[1].name, "Priya Patel");
        strcpy(s[1].dob, "22/11/2004");
        strcpy(s[1].gender, "Female");
        strcpy(s[1].aadhar, "569874123654");
        strcpy(s[1].bloodGroup, "O+");
        strcpy(s[1].religion, "Hindu");
        strcpy(s[1].caste, "OBC");
        strcpy(s[1].nationality, "Indian");
        strcpy(s[1].email, "priya.patel@yahoo.com");
        strcpy(s[1].phone, "9821547896");
        strcpy(s[1].course, "B.Tech");
        strcpy(s[1].branch, "ECE");
        strcpy(s[1].semester, "III");
        strcpy(s[1].rollNo, "2024ECE042");
        strcpy(s[1].fatherName, "Harish Patel");
        strcpy(s[1].motherName, "Kiran Patel");
        strcpy(s[1].parentPhone, "9821547890");
        strcpy(s[1].parentEmail, "harish.patel@gmail.com");
        strcpy(s[1].parentOccupation, "Service");
        strcpy(s[1].parentIncome, "650000");
        strcpy(s[1].guardianName, "NA");
        strcpy(s[1].guardianRelation, "NA");
        strcpy(s[1].guardianPhone, "0000000000");
        strcpy(s[1].guardianAddress, "NA");
        strcpy(s[1].permanentAddress, "A-304, Green Heights");
        strcpy(s[1].city, "Ahmedabad");
        strcpy(s[1].state, "Gujarat");
        strcpy(s[1].pincode, "380015");
        strcpy(s[1].hostelName, "Chola Block");
        strcpy(s[1].roomNo, "104");
        strcpy(s[1].roomType, "Double");
        strcpy(s[1].allotmentDate, "12/07/2025");
        s[1].feePaid = 20000.0f;
        s[1].feePending = 15000.0f;
        s[1].isAllotted = 1;

        /* Student 3 */
        strcpy(s[2].studentId, "STU-003");
        strcpy(s[2].name, "Aarav Mehta");
        strcpy(s[2].dob, "05/01/2003");
        strcpy(s[2].gender, "Male");
        strcpy(s[2].aadhar, "789654123528");
        strcpy(s[2].bloodGroup, "B-");
        strcpy(s[2].religion, "Jain");
        strcpy(s[2].caste, "General");
        strcpy(s[2].nationality, "Indian");
        strcpy(s[2].email, "aarav.mehta@outlook.com");
        strcpy(s[2].phone, "7014236985");
        strcpy(s[2].course, "MBA");
        strcpy(s[2].branch, "Finance");
        strcpy(s[2].semester, "I");
        strcpy(s[2].rollNo, "2025MBA005");
        strcpy(s[2].fatherName, "Rajesh Mehta");
        strcpy(s[2].motherName, "Varsha Mehta");
        strcpy(s[2].parentPhone, "7014236980");
        strcpy(s[2].parentEmail, "rajesh.mehta@gmail.com");
        strcpy(s[2].parentOccupation, "CA");
        strcpy(s[2].parentIncome, "1800000");
        strcpy(s[2].guardianName, "Suresh Shah");
        strcpy(s[2].guardianRelation, "Uncle");
        strcpy(s[2].guardianPhone, "9966332211");
        strcpy(s[2].guardianAddress, "45, Marine Drive, Mumbai");
        strcpy(s[2].permanentAddress, "45, Marine Drive");
        strcpy(s[2].city, "Mumbai");
        strcpy(s[2].state, "Maharashtra");
        strcpy(s[2].pincode, "400002");
        strcpy(s[2].hostelName, "Gupta Block");
        strcpy(s[2].roomNo, "108");
        strcpy(s[2].roomType, "Triple");
        strcpy(s[2].allotmentDate, "28/07/2025");
        s[2].feePaid = 25000.0f;
        s[2].feePending = 0.0f;
        s[2].isAllotted = 1;

        /* Student 4 */
        strcpy(s[3].studentId, "STU-004");
        strcpy(s[3].name, "Ananya Rao");
        strcpy(s[3].dob, "19/04/2005");
        strcpy(s[3].gender, "Female");
        strcpy(s[3].aadhar, "123654789542");
        strcpy(s[3].bloodGroup, "AB+");
        strcpy(s[3].religion, "Hindu");
        strcpy(s[3].caste, "General");
        strcpy(s[3].nationality, "Indian");
        strcpy(s[3].email, "ananya.rao@gmail.com");
        strcpy(s[3].phone, "8899665544");
        strcpy(s[3].course, "B.Tech");
        strcpy(s[3].branch, "IT");
        strcpy(s[3].semester, "I");
        strcpy(s[3].rollNo, "2025IT011");
        strcpy(s[3].fatherName, "Venkatesh Rao");
        strcpy(s[3].motherName, "Laxmi Rao");
        strcpy(s[3].parentPhone, "8899665500");
        strcpy(s[3].parentEmail, "venkatesh.rao@gmail.com");
        strcpy(s[3].parentOccupation, "Professor");
        strcpy(s[3].parentIncome, "950000");
        strcpy(s[3].guardianName, "NA");
        strcpy(s[3].guardianRelation, "NA");
        strcpy(s[3].guardianPhone, "0000000000");
        strcpy(s[3].guardianAddress, "NA");
        strcpy(s[3].permanentAddress, "Flat 502, Orchid Villa");
        strcpy(s[3].city, "Hyderabad");
        strcpy(s[3].state, "Telangana");
        strcpy(s[3].pincode, "500081");
        strcpy(s[3].hostelName, "");
        strcpy(s[3].roomNo, "");
        strcpy(s[3].roomType, "");
        strcpy(s[3].allotmentDate, "");
        s[3].feePaid = 0.0f;
        s[3].feePending = 0.0f;
        s[3].isAllotted = 0;

        /* Student 5 */
        strcpy(s[4].studentId, "STU-005");
        strcpy(s[4].name, "Kabir Singh");
        strcpy(s[4].dob, "30/09/2002");
        strcpy(s[4].gender, "Male");
        strcpy(s[4].aadhar, "963258741258");
        strcpy(s[4].bloodGroup, "O-");
        strcpy(s[4].religion, "Sikh");
        strcpy(s[4].caste, "General");
        strcpy(s[4].nationality, "Indian");
        strcpy(s[4].email, "kabir.singh@gmail.com");
        strcpy(s[4].phone, "7744112233");
        strcpy(s[4].course, "MBBS");
        strcpy(s[4].branch, "General Medicine");
        strcpy(s[4].semester, "V");
        strcpy(s[4].rollNo, "2023MED087");
        strcpy(s[4].fatherName, "Bhupinder Singh");
        strcpy(s[4].motherName, "Jaspreet Kaur");
        strcpy(s[4].parentPhone, "7744112200");
        strcpy(s[4].parentEmail, "bhupinder.singh@gmail.com");
        strcpy(s[4].parentOccupation, "Agriculturist");
        strcpy(s[4].parentIncome, "1500000");
        strcpy(s[4].guardianName, "NA");
        strcpy(s[4].guardianRelation, "NA");
        strcpy(s[4].guardianPhone, "0000000000");
        strcpy(s[4].guardianAddress, "NA");
        strcpy(s[4].permanentAddress, "Farmhouse 4, GT Road");
        strcpy(s[4].city, "Amritsar");
        strcpy(s[4].state, "Punjab");
        strcpy(s[4].pincode, "143001");
        strcpy(s[4].hostelName, "Mughal Block");
        strcpy(s[4].roomNo, "102");
        strcpy(s[4].roomType, "Single");
        strcpy(s[4].allotmentDate, "05/07/2024");
        s[4].feePaid = 40000.0f;
        s[4].feePending = 10000.0f;
        s[4].isAllotted = 1;

        fwrite(s, sizeof(Student), 5, fpStudents);
        fclose(fpStudents);
    }

    /* 3. Seed Audit Log */
    fpAudit = fopen(FILE_AUDIT, "w");
    if (fpAudit != NULL) {
        fprintf(fpAudit, "[2026-05-26 10:00:00] User: SYSTEM          | Action: Seeded default administrative configuration and rooms.\n");
        fprintf(fpAudit, "[2026-05-26 10:15:00] User: admin           | Action: Registered students STU-001 to STU-005.\n");
        fprintf(fpAudit, "[2026-05-26 10:20:00] User: admin           | Action: Allotted rooms and logged initial payments.\n");
        fclose(fpAudit);
    }
}
