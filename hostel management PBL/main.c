#include "common.h"
#include "utils.h"
#include "auth.h"
#include "student.h"
#include "allotment.h"
#include "room.h"
#include "reports.h"
#include "audit.h"
#include "server.h"

int main(void) {
    char currentAdmin[30] = "";
    int choice;
    
    /* Initialize the system config and rooms on start */
    initializeSystem();
    
    while (1) {
        clearScreen();
        printf(COLOR_CYAN);
        printDivider('=', 50);
        printf("   " COLOR_BOLD "HOSTEL ROOM ALLOTMENT SYSTEM" COLOR_RESET COLOR_CYAN "\n");
        printf("   Powered by Indian Dynasty Blocks\n");
        printDivider('=', 50);
        printf(COLOR_RESET);
        
        if (strlen(currentAdmin) > 0) {
            printf("   Session Status: " COLOR_GREEN "ADMIN (%s)" COLOR_RESET "\n", currentAdmin);
        } else {
            printf("   Session Status: " COLOR_YELLOW "GUEST (Read-Only)" COLOR_RESET "\n");
        }
        printDivider('-', 50);
        
        printf("  1. Student Registration\n");
        printf("  2. Room Allotment\n");
        printf("  3. Room Management Menu\n");
        printf("  4. Reports & Search Menu\n");
        printf("  5. Settings & Admin Panel\n");
        printf("  6. Launch Web Dashboard (HTML/CSS/JS)\n");
        if (strlen(currentAdmin) > 0) {
            printf("  7. Logout Administrative Session\n");
        }
        printf("  0. Exit System\n");
        printf(COLOR_CYAN);
        printDivider('=', 50);
        printf(COLOR_RESET);
        printf("Select Option: ");
        
        choice = getSafeInt();
        
        if (choice == 0) {
            printf("\n" COLOR_GREEN "Thank you for using the Hostel Allotment System. Exiting..." COLOR_RESET "\n");
            break;
        }
        
        switch (choice) {
            case 1: /* Student Registration - Write operation, requires login */
                if (strlen(currentAdmin) == 0) {
                    printf("\nAuthentication required for Registration. Redirecting to Login...\n");
                    if (!adminLogin(currentAdmin)) {
                        pressAnyKey();
                        break;
                    }
                }
                registerStudent(currentAdmin);
                break;
                
            case 2: /* Room Allotment - Write operation, requires login */
                if (strlen(currentAdmin) == 0) {
                    printf("\nAuthentication required for Room Allotment. Redirecting to Login...\n");
                    if (!adminLogin(currentAdmin)) {
                        pressAnyKey();
                        break;
                    }
                }
                allotRoom(currentAdmin);
                break;
                
            case 3: /* Room Management Menu - Contains write operations, gated */
                if (strlen(currentAdmin) == 0) {
                    printf("\nAuthentication required for Room Management. Redirecting to Login...\n");
                    if (!adminLogin(currentAdmin)) {
                        pressAnyKey();
                        break;
                    }
                }
                /* Run Room Management Submenu */
                {
                    int rmChoice = -1;
                    while (rmChoice != 0) {
                        printHeader("ROOM MANAGEMENT");
                        printf("1. Vacate Student Room\n");
                        printf("2. Transfer Student Room/Block\n");
                        printf("3. View Room Occupancy Statistics\n");
                        printf("4. View All Residents in a Block\n");
                        printf("5. Toggle Room Maintenance Status\n");
                        printf("0. Return to Main Menu\n");
                        printf("\nSelect Option: ");
                        rmChoice = getSafeInt();
                        
                        switch (rmChoice) {
                            case 1: vacateRoom(currentAdmin); break;
                            case 2: transferStudent(currentAdmin); break;
                            case 3: viewRoomOccupancy(); break;
                            case 4: viewStudentsByBlock(); break;
                            case 5: manageRoomMaintenance(currentAdmin); break;
                            case 0: break;
                            default: printStatusMessage("Invalid option!", 0); pressAnyKey(); break;
                        }
                    }
                }
                break;
                
            case 4: /* Reports & Search - Read-only, open to guests */
                {
                    int repChoice = -1;
                    while (repChoice != 0) {
                        printHeader("REPORTS & SEARCH");
                        printf("1. Search Student ID/Name/Room\n");
                        printf("2. View All Allotted Students\n");
                        printf("3. View All Vacant Rooms\n");
                        printf("4. View Fee Defaulters\n");
                        printf("5. Filter Students by Course/Branch\n");
                        printf("0. Return to Main Menu\n");
                        printf("\nSelect Option: ");
                        repChoice = getSafeInt();
                        
                        switch (repChoice) {
                            case 1: searchStudent(); break;
                            case 2: reportAllAllottedStudents(); break;
                            case 3: reportAllVacantRooms(); break;
                            case 4: reportFeeDefaulters(); break;
                            case 5: reportStudentsByCourseBranch(); break;
                            case 0: break;
                            default: printStatusMessage("Invalid option!", 0); pressAnyKey(); break;
                        }
                    }
                }
                break;
                
            case 5: /* Settings & Admin Panel */
                if (strlen(currentAdmin) == 0) {
                    printf("\nAuthentication required for Settings & Admin Panel. Redirecting to Login...\n");
                    if (!adminLogin(currentAdmin)) {
                        pressAnyKey();
                        break;
                    }
                }
                {
                    int setChoice = -1;
                    while (setChoice != 0) {
                        printHeader("SETTINGS & ADMIN PANEL");
                        printf("1. Change Admin Password\n");
                        printf("2. Edit System Config (Rooms & Fees)\n");
                        printf("3. View System Audit Trail\n");
                        printf("4. Wipe/Reset All System Databases\n");
                        printf("0. Return to Main Menu\n");
                        printf("\nSelect Option: ");
                        setChoice = getSafeInt();
                        
                        switch (setChoice) {
                            case 1: changeAdminPassword(currentAdmin); break;
                            case 2: updateSystemConfig(currentAdmin); break;
                            case 3: 
                                clearScreen();
                                displayAuditTrail(); 
                                pressAnyKey(); 
                                break;
                            case 4: 
                                if (resetSystemData(currentAdmin)) {
                                    /* Logged out on wipe */
                                    strcpy(currentAdmin, "");
                                    setChoice = 0;
                                }
                                break;
                            case 0: break;
                            default: printStatusMessage("Invalid option!", 0); pressAnyKey(); break;
                        }
                    }
                }
                break;
                
            case 6:
                startWebServer(8080);
                pressAnyKey();
                break;
                
            case 7:
                if (strlen(currentAdmin) > 0) {
                    writeAuditLog(currentAdmin, "Logged out of administrative session.");
                    strcpy(currentAdmin, "");
                    printStatusMessage("Admin logged out successfully.", 1);
                } else {
                    printStatusMessage("Invalid choice!", 0);
                }
                pressAnyKey();
                break;
                
            default:
                printStatusMessage("Invalid option selected!", 0);
                pressAnyKey();
                break;
        }
    }
    return 0;
}
