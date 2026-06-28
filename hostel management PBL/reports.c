#include "reports.h"
#include "utils.h"
#include "student.h"
#include <ctype.h>

/* Case-insensitive substring matching helper */
static int matchesSubstringCaseInsensitive(const char *haystack, const char *needle) {
    int i, j;
    int needleLen, haystackLen;

    if (needle == NULL || haystack == NULL) return 0;
    needleLen = strlen(needle);
    if (needleLen == 0) return 1;
    haystackLen = strlen(haystack);
    if (haystackLen < needleLen) return 0;
    
    for (i = 0; i <= haystackLen - needleLen; i++) {
        int found = 1;
        for (j = 0; j < needleLen; j++) {
            if (tolower((unsigned char)haystack[i + j]) != tolower((unsigned char)needle[j])) {
                found = 0;
                break;
            }
        }
        if (found) return 1;
    }
    return 0;
}

/* Stylized print of student's complete record card */
static void printStudentDossier(const Student *s) {
    printf("\n" COLOR_CYAN);
    printDivider('=', 65);
    printf(" %sSTUDENT PROFILE DOSSIER: %s%-34s%s\n", COLOR_BOLD, s->studentId, s->name, COLOR_RESET COLOR_CYAN);
    printDivider('=', 65);
    printf(COLOR_RESET);

    printf(COLOR_BOLD " [1] Personal Details" COLOR_RESET "\n");
    printf("   Date of Birth: %-12s Gender: %-12s Blood Group: %-5s\n", s->dob, s->gender, s->bloodGroup);
    printf("   Aadhar No    : %-15s Nationality: %-15s\n", s->aadhar, s->nationality);
    printf("   Religion     : %-15s Caste / Cat : %-15s\n", s->religion, s->caste);
    printf("   Email Address: %-30s Phone: %s\n", s->email, s->phone);

    printf("\n" COLOR_BOLD " [2] Academic Details" COLOR_RESET "\n");
    printf("   Course       : %-15s Branch: %-15s Semester: %s\n", s->course, s->branch, s->semester);
    printf("   University Roll Number: %s\n", s->rollNo);

    printf("\n" COLOR_BOLD " [3] Parent & Guardian Details" COLOR_RESET "\n");
    printf("   Father's Name: %-25s Mother's Name: %s\n", s->fatherName, s->motherName);
    printf("   Parent Phone : %-15s Parent Email: %s\n", s->parentPhone, s->parentEmail);
    printf("   Parent Occ.  : %-15s Annual Income: INR %s\n", s->parentOccupation, s->parentIncome);
    printf("   Guardian Name: %-25s Relation    : %s\n", s->guardianName, s->guardianRelation);
    printf("   Guardian Ph. : %-15s Guardian Addr: %s\n", s->guardianPhone, s->guardianAddress);

    printf("\n" COLOR_BOLD " [4] Residential Address" COLOR_RESET "\n");
    printf("   Permanent Address: %s\n", s->permanentAddress);
    printf("   City             : %-15s State: %-15s Pincode: %s\n", s->city, s->state, s->pincode);

    printf("\n" COLOR_BOLD " [5] Hostel & Room Allotment" COLOR_RESET "\n");
    if (s->isAllotted) {
        printf("   Allotment Status: " COLOR_GREEN "ALLOTTED" COLOR_RESET "\n");
        printf("   Hostel Block    : %-20s Room No: %-8s Type: %s\n", s->hostelName, s->roomNo, s->roomType);
        printf("   Allotment Date  : %-12s Checkout Date: -\n", s->allotmentDate);
        printf("   Fees Paid       : INR %-12.2f Fees Pending : INR %.2f\n", s->feePaid, s->feePending);
    } else {
        printf("   Allotment Status: " COLOR_YELLOW "NOT ALLOTTED / VACATED" COLOR_RESET "\n");
        if (strlen(s->checkoutDate) > 0) {
            printf("   Last Block      : %-20s Room No  : %-8s Type: %s\n", s->hostelName, s->roomNo, s->roomType);
            printf("   Last Checkout   : %s\n", s->checkoutDate);
        }
    }
    printf(COLOR_CYAN);
    printDivider('=', 65);
    printf(COLOR_RESET "\n");
}

void searchStudent(void) {
    int opt;
    char searchStr[50];
    FILE *fp;
    Student s;
    int matches = 0;

    printHeader("STUDENT SEARCH ENGINE");
    printf("1. Search by Student ID\n");
    printf("2. Search by Name (Substring)\n");
    printf("3. Search by Room Number\n");
    printf("0. Return to main menu\n");
    printf("\nSelect Option: ");
    opt = getSafeInt();

    if (opt == 0) return;

    fp = fopen(FILE_STUDENTS, "rb");
    if (fp == NULL) {
        printStatusMessage("Student database is empty.", 0);
        pressAnyKey();
        return;
    }

    switch (opt) {
        case 1:
            printf("Enter exact Student ID (e.g. STU-001): ");
            getSafeString(searchStr, sizeof(searchStr));
            while (fread(&s, sizeof(Student), 1, fp) == 1) {
                if (strcmp(s.studentId, searchStr) == 0) {
                    printStudentDossier(&s);
                    matches++;
                    break;
                }
            }
            break;

        case 2:
            printf("Enter Name (or part of name): ");
            getSafeString(searchStr, sizeof(searchStr));
            while (fread(&s, sizeof(Student), 1, fp) == 1) {
                if (matchesSubstringCaseInsensitive(s.name, searchStr)) {
                    printStudentDossier(&s);
                    matches++;
                }
            }
            break;

        case 3:
            printf("Enter Room Number: ");
            getSafeString(searchStr, sizeof(searchStr));
            while (fread(&s, sizeof(Student), 1, fp) == 1) {
                if (s.isAllotted && strcmp(s.roomNo, searchStr) == 0) {
                    printStudentDossier(&s);
                    matches++;
                }
            }
            break;

        default:
            printStatusMessage("Invalid option selected!", 0);
            break;
    }

    if (matches == 0 && opt >= 1 && opt <= 3) {
        printStatusMessage("No matching records found.", 0);
    } else if (matches > 0) {
        printf("Matches found: %d\n", matches);
    }
    
    fclose(fp);
    pressAnyKey();
}

void reportAllAllottedStudents(void) {
    FILE *fp = fopen(FILE_STUDENTS, "rb");
    Student s;
    int count = 0;

    if (fp == NULL) {
        printStatusMessage("Student database is empty.", 0);
        pressAnyKey();
        return;
    }

    printHeader("ALL ALLOTTED STUDENTS");
    printf("%-10s | %-20s | %-18s | %-8s | %-8s | %-15s\n", 
           "ID", "Name", "Hostel Block", "Room No", "Type", "Phone");
    printDivider('-', 88);

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.isAllotted) {
            printf("%-10s | %-20s | %-18s | %-8s | %-8s | %-15s\n",
                   s.studentId, s.name, s.hostelName, s.roomNo, s.roomType, s.phone);
            count++;
        }
    }

    printDivider('-', 88);
    printf("Total Active Allotted Students: %d\n", count);
    fclose(fp);
    pressAnyKey();
}

void reportAllVacantRooms(void) {
    FILE *fp = fopen(FILE_ROOMS, "rb");
    Room r;
    int count = 0;

    if (fp == NULL) {
        printStatusMessage("Rooms database is empty.", 0);
        pressAnyKey();
        return;
    }

    printHeader("VACANT ROOMS REPORT");
    printf("%-18s | %-10s | %-10s | %-10s | %-10s | %-12s\n", 
           "Hostel Block", "Room No.", "Type", "Capacity", "Occupied", "Beds Vacant");
    printDivider('-', 80);

    while (fread(&r, sizeof(Room), 1, fp) == 1) {
        if (r.occupancy < r.capacity && r.isUnderMaintenance == 0) {
            printf("%-18s | %-10s | %-10s | %-10d | %-10d | " COLOR_GREEN "%-12d" COLOR_RESET "\n",
                   r.hostelName, r.roomNo, r.roomType, r.capacity, r.occupancy, r.capacity - r.occupancy);
            count++;
        }
    }

    printDivider('-', 80);
    printf("Total Rooms with Vacancies: %d\n", count);
    fclose(fp);
    pressAnyKey();
}

void reportFeeDefaulters(void) {
    FILE *fp = fopen(FILE_STUDENTS, "rb");
    Student s;
    int count = 0;

    if (fp == NULL) {
        printStatusMessage("Student database is empty.", 0);
        pressAnyKey();
        return;
    }

    printHeader("FEE DEFAULTERS REPORT (OUTSTANDING DUES)");
    printf("%-10s | %-20s | %-18s | %-8s | %-12s | %-12s\n", 
           "ID", "Name", "Hostel Block", "Room No", "Paid (INR)", "Dues (INR)");
    printDivider('-', 88);

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        if (s.isAllotted && s.feePending > 0.0f) {
            printf("%-10s | %-20s | %-18s | %-8s | %-12.2f | " COLOR_RED "%-12.2f" COLOR_RESET "\n",
                   s.studentId, s.name, s.hostelName, s.roomNo, s.feePaid, s.feePending);
            count++;
        }
    }

    printDivider('-', 88);
    printf("Total Fee Defaulters: %d\n", count);
    fclose(fp);
    pressAnyKey();
}

void reportStudentsByCourseBranch(void) {
    int opt;
    char searchStr[30];
    FILE *fp;
    Student s;
    int count = 0;

    printHeader("FILTER STUDENTS BY ACADEMICS");
    printf("1. View Students by Course\n");
    printf("2. View Students by Branch\n");
    printf("0. Return to menu\n");
    printf("\nSelect Option: ");
    opt = getSafeInt();

    if (opt != 1 && opt != 2) return;

    fp = fopen(FILE_STUDENTS, "rb");
    if (fp == NULL) {
        printStatusMessage("Student database is empty.", 0);
        pressAnyKey();
        return;
    }

    if (opt == 1) {
        printf("Enter Course Name (e.g. B.Tech): ");
        getSafeString(searchStr, sizeof(searchStr));
        printHeader("STUDENTS FILTERED BY COURSE");
    } else {
        printf("Enter Branch Name (e.g. CSE): ");
        getSafeString(searchStr, sizeof(searchStr));
        printHeader("STUDENTS FILTERED BY BRANCH");
    }

    printf("%-10s | %-20s | %-10s | %-10s | %-15s | %-12s\n", 
           "ID", "Name", "Course", "Branch", "Hostel Block", "Room No");
    printDivider('-', 86);

    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        int match = 0;
        if (opt == 1 && matchesSubstringCaseInsensitive(s.course, searchStr)) {
            match = 1;
        } else if (opt == 2 && matchesSubstringCaseInsensitive(s.branch, searchStr)) {
            match = 1;
        }

        if (match) {
            printf("%-10s | %-20s | %-10s | %-10s | %-15s | %-12s\n",
                   s.studentId, s.name, s.course, s.branch, 
                   s.isAllotted ? s.hostelName : "Unallotted", s.isAllotted ? s.roomNo : "-");
            count++;
        }
    }

    printDivider('-', 86);
    printf("Total matching students found: %d\n", count);
    fclose(fp);
    pressAnyKey();
}
