#include "student.h"
#include "utils.h"
#include "audit.h"

/* Generates the next sequential unique Student ID in format STU-NNN */
static void generateNextStudentId(char *idBuffer) {
    FILE *fp = fopen(FILE_STUDENTS, "rb");
    int maxId = 0;
    if (fp != NULL) {
        Student stu;
        while (fread(&stu, sizeof(Student), 1, fp) == 1) {
            if (strncmp(stu.studentId, "STU-", 4) == 0) {
                int idNum = atoi(stu.studentId + 4);
                if (idNum > maxId) {
                    maxId = idNum;
                }
            }
        }
        fclose(fp);
    }
    sprintf(idBuffer, "STU-%03d", maxId + 1);
}

int findStudentById(const char *id, Student *foundStudent) {
    FILE *fp = fopen(FILE_STUDENTS, "rb");
    Student temp;
    
    if (fp == NULL) {
        return 0;
    }
    while (fread(&temp, sizeof(Student), 1, fp) == 1) {
        if (strcmp(temp.studentId, id) == 0) {
            if (foundStudent != NULL) {
                *foundStudent = temp;
            }
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int registerStudent(const char *currentAdminUser) {
    Student s;
    FILE *fp;
    char nextId[20];
    char actionLog[120];

    printHeader("STUDENT REGISTRATION");
    generateNextStudentId(nextId);
    printf("Assigned Student ID: " COLOR_GREEN "%s" COLOR_RESET "\n", nextId);
    printf("Type " COLOR_YELLOW "CANCEL" COLOR_RESET " at the name prompt to abort.\n\n");

    /* Personal Details */
    printf("Enter Student Full Name: ");
    getSafeString(s.name, sizeof(s.name));
    if (strcmp(s.name, "CANCEL") == 0 || strlen(s.name) == 0) {
        printStatusMessage("Registration cancelled.", 1);
        return 0;
    }

    do {
        printf("Enter Date of Birth (DD/MM/YYYY): ");
        getSafeString(s.dob, sizeof(s.dob));
        if (!validateDate(s.dob)) {
            printf(COLOR_RED "Invalid date/format (must be DD/MM/YYYY and valid day/month)." COLOR_RESET "\n");
        }
    } while (!validateDate(s.dob));

    printf("Enter Gender (Male/Female/Other): ");
    getSafeString(s.gender, sizeof(s.gender));

    do {
        printf("Enter Aadhar Number (12 digits): ");
        getSafeString(s.aadhar, sizeof(s.aadhar));
        if (!validateAadhar(s.aadhar)) {
            printf(COLOR_RED "Invalid Aadhar! Must be exactly 12 digits." COLOR_RESET "\n");
        }
    } while (!validateAadhar(s.aadhar));

    printf("Enter Blood Group (e.g. A+, B-, O+): ");
    getSafeString(s.bloodGroup, sizeof(s.bloodGroup));

    printf("Enter Religion: ");
    getSafeString(s.religion, sizeof(s.religion));

    printf("Enter Caste / Category: ");
    getSafeString(s.caste, sizeof(s.caste));

    printf("Enter Nationality: ");
    getSafeString(s.nationality, sizeof(s.nationality));

    do {
        printf("Enter Email Address: ");
        getSafeString(s.email, sizeof(s.email));
        if (!validateEmail(s.email)) {
            printf(COLOR_RED "Invalid email format (must contain '@' and '.')." COLOR_RESET "\n");
        }
    } while (!validateEmail(s.email));

    do {
        printf("Enter Phone Number (10 digits): ");
        getSafeString(s.phone, sizeof(s.phone));
        if (!validatePhone(s.phone)) {
            printf(COLOR_RED "Invalid phone! Must be exactly 10 digits." COLOR_RESET "\n");
        }
    } while (!validatePhone(s.phone));

    /* Academic Details */
    printf("\n--- Academic Details ---\n");
    printf("Enter Course (e.g. B.Tech, MBA): ");
    getSafeString(s.course, sizeof(s.course));

    printf("Enter Branch / Specialization: ");
    getSafeString(s.branch, sizeof(s.branch));

    printf("Enter Semester: ");
    getSafeString(s.semester, sizeof(s.semester));

    printf("Enter University Roll Number: ");
    getSafeString(s.rollNo, sizeof(s.rollNo));

    /* Parent Details */
    printf("\n--- Parent Details ---\n");
    printf("Enter Father's Name: ");
    getSafeString(s.fatherName, sizeof(s.fatherName));

    printf("Enter Mother's Name: ");
    getSafeString(s.motherName, sizeof(s.motherName));

    do {
        printf("Enter Parent's Phone (10 digits): ");
        getSafeString(s.parentPhone, sizeof(s.parentPhone));
        if (!validatePhone(s.parentPhone)) {
            printf(COLOR_RED "Invalid phone! Must be exactly 10 digits." COLOR_RESET "\n");
        }
    } while (!validatePhone(s.parentPhone));

    do {
        printf("Enter Parent's Email: ");
        getSafeString(s.parentEmail, sizeof(s.parentEmail));
        if (!validateEmail(s.parentEmail)) {
            printf(COLOR_RED "Invalid email format." COLOR_RESET "\n");
        }
    } while (!validateEmail(s.parentEmail));

    printf("Enter Father/Parent Occupation: ");
    getSafeString(s.parentOccupation, sizeof(s.parentOccupation));

    do {
        printf("Enter Annual Parent Income (INR): ");
        getSafeString(s.parentIncome, sizeof(s.parentIncome));
        if (!validateNumeric(s.parentIncome)) {
            printf(COLOR_RED "Invalid entry! Income must be numeric." COLOR_RESET "\n");
        }
    } while (!validateNumeric(s.parentIncome));

    /* Guardian Details */
    printf("\n--- Guardian Details ---\n");
    printf("Enter Guardian Name (or type 'NA'): ");
    getSafeString(s.guardianName, sizeof(s.guardianName));

    printf("Enter Guardian Relation: ");
    getSafeString(s.guardianRelation, sizeof(s.guardianRelation));

    do {
        printf("Enter Guardian Phone (10 digits, or '0000000000'): ");
        getSafeString(s.guardianPhone, sizeof(s.guardianPhone));
        if (strcmp(s.guardianPhone, "0000000000") != 0 && !validatePhone(s.guardianPhone)) {
            printf(COLOR_RED "Invalid phone! Must be exactly 10 digits." COLOR_RESET "\n");
        }
    } while (strcmp(s.guardianPhone, "0000000000") != 0 && !validatePhone(s.guardianPhone));

    printf("Enter Guardian Address: ");
    getSafeString(s.guardianAddress, sizeof(s.guardianAddress));

    /* Address Details */
    printf("\n--- Permanent Address ---\n");
    printf("Enter Street Address: ");
    getSafeString(s.permanentAddress, sizeof(s.permanentAddress));

    printf("Enter City: ");
    getSafeString(s.city, sizeof(s.city));

    printf("Enter State: ");
    getSafeString(s.state, sizeof(s.state));

    do {
        printf("Enter Pincode (6 digits): ");
        getSafeString(s.pincode, sizeof(s.pincode));
        if (strlen(s.pincode) != 6 || !validateNumeric(s.pincode)) {
            printf(COLOR_RED "Invalid pincode! Must be exactly 6 digits." COLOR_RESET "\n");
        }
    } while (strlen(s.pincode) != 6 || !validateNumeric(s.pincode));

    /* Initialize Room Allotment Fields to Unallotted State */
    strcpy(s.studentId, nextId);
    strcpy(s.hostelName, "");
    strcpy(s.roomNo, "");
    strcpy(s.roomType, "");
    strcpy(s.allotmentDate, "");
    strcpy(s.checkoutDate, "");
    s.feePaid = 0.0f;
    s.feePending = 0.0f;
    s.isAllotted = 0;

    /* Persist to Binary File */
    fp = fopen(FILE_STUDENTS, "ab");
    if (fp == NULL) {
        printStatusMessage("Failed to open student database for writing.", 0);
        return 0;
    }
    
    fwrite(&s, sizeof(Student), 1, fp);
    fclose(fp);

    /* System Logging */
    sprintf(actionLog, "Registered student %s (%s)", s.name, s.studentId);
    writeAuditLog(currentAdminUser, actionLog);

    printStatusMessage("Student registered successfully!", 1);
    pressAnyKey();
    return 1;
}
