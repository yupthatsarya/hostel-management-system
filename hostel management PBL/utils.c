#include "utils.h"
#include <ctype.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

const char* HOSTELS[HOSTEL_COUNT] = {
    "Maurya Block",
    "Chola Block",
    "Gupta Block",
    "Mughal Block",
    "Maratha Block",
    "Vijayanagara Block"
};

/* Security: Basic XOR encryption for demonstrating storage protection */
void encryptDecrypt(char *data, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (data[i] != '\0') {
            data[i] = data[i] ^ XOR_KEY;
        }
    }
}

/* Clear the stdin input buffer */
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Reads a line from stdin safely up to maxLen - 1 and trims newlines */
void getSafeString(char *buffer, int maxLen) {
    int len;
    if (fgets(buffer, maxLen, stdin) != NULL) {
        len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        } else {
            /* Input exceeded buffer size; consume remaining characters in stream */
            clearInputBuffer();
        }
    } else {
        buffer[0] = '\0';
    }
}

/* Reads a password from stdin masking input with asterisks on Windows */
void getMaskedPassword(char *password, int maxLen) {
    int i = 0;
    char ch;
#ifdef _WIN32
    while (1) {
        ch = (char)_getch();
        if (ch == '\r' || ch == '\n') {
            break;
        } else if (ch == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (i < maxLen - 1) {
            password[i++] = ch;
            printf("*");
        }
    }
    password[i] = '\0';
    printf("\n");
#else
    /* Non-Windows fallback: hide echo */
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    getSafeString(password, maxLen);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

/* Safely reads an integer, avoiding infinite loops on invalid input */
int getSafeInt(void) {
    char buffer[32];
    getSafeString(buffer, sizeof(buffer));
    if (strlen(buffer) == 0 || !validateNumeric(buffer)) {
        return -1; /* Sentinel value for invalid integer input */
    }
    return atoi(buffer);
}

/* Safely reads a floating point number, avoiding infinite loops on invalid input */
float getSafeFloat(void) {
    char buffer[32];
    int hasDot = 0;
    unsigned int i;
    getSafeString(buffer, sizeof(buffer));
    
    if (strlen(buffer) == 0) return -1.0f;
    
    for (i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == '.') {
            if (hasDot) return -1.0f; /* Multiple decimal points */
            hasDot = 1;
        } else if (!isdigit((unsigned char)buffer[i])) {
            return -1.0f;
        }
    }
    return (float)atof(buffer);
}

/* Validates that a phone number has exactly 10 digits */
int validatePhone(const char *phone) {
    int i;
    if (strlen(phone) != 10) return 0;
    for (i = 0; i < 10; i++) {
        if (!isdigit((unsigned char)phone[i])) return 0;
    }
    return 1;
}

/* Validates that an Aadhar number has exactly 12 digits */
int validateAadhar(const char *aadhar) {
    int i;
    if (strlen(aadhar) != 12) return 0;
    for (i = 0; i < 12; i++) {
        if (!isdigit((unsigned char)aadhar[i])) return 0;
    }
    return 1;
}

/* Validates that an email contains '@' and '.', and they are correctly positioned */
int validateEmail(const char *email) {
    const char *at = strchr(email, '@');
    const char *dot;
    if (!at || at == email || *(at + 1) == '\0') return 0;
    dot = strchr(at, '.');
    if (!dot || dot == at + 1 || *(dot + 1) == '\0') return 0;
    return 1;
}

/* Validates dates in DD/MM/YYYY format, verifying months, leap years, and dates */
int validateDate(const char *date) {
    int day, month, year;
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (strlen(date) != 10) return 0;
    if (date[2] != '/' || date[5] != '/') return 0;

    /* Verify all other positions are digits */
    if (!isdigit((unsigned char)date[0]) || !isdigit((unsigned char)date[1]) ||
        !isdigit((unsigned char)date[3]) || !isdigit((unsigned char)date[4]) ||
        !isdigit((unsigned char)date[6]) || !isdigit((unsigned char)date[7]) ||
        !isdigit((unsigned char)date[8]) || !isdigit((unsigned char)date[9])) {
        return 0;
    }

    day = atoi(date);
    month = atoi(date + 3);
    year = atoi(date + 6);

    if (year < 1900 || year > 2100) return 0;
    if (month < 1 || month > 12) return 0;

    /* Check leap year */
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        daysInMonth[2] = 29;
    }

    if (day < 1 || day > daysInMonth[month]) return 0;

    return 1;
}

/* Validates that a string contains only digits (numeric checking) */
int validateNumeric(const char *str) {
    int i = 0;
    if (str[0] == '\0') return 0;
    while (str[i] != '\0') {
        if (!isdigit((unsigned char)str[i])) return 0;
        i++;
    }
    return 1;
}

/* Clears terminal screen cross-platform */
void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[H\033[J");
#endif
}

/* Pauses CLI until user presses Enter */
void pressAnyKey(void) {
    printf("\n" COLOR_CYAN "Press Enter to continue..." COLOR_RESET "\n");
    clearInputBuffer();
}

/* Helper to print a line of characters */
void printDivider(char ch, int length) {
    int i;
    for (i = 0; i < length; i++) {
        putchar(ch);
    }
    putchar('\n');
}

/* Prints stylized header for modules */
void printHeader(const char *title) {
    clearScreen();
    printf(COLOR_CYAN);
    printDivider('=', 60);
    printf("  %s%-56s%s\n", COLOR_BOLD, title, COLOR_RESET COLOR_CYAN);
    printDivider('=', 60);
    printf(COLOR_RESET);
}

/* Standardized success or error status message display */
void printStatusMessage(const char *message, int success) {
    if (success) {
        printf("\n" COLOR_GREEN "[SUCCESS] %s" COLOR_RESET "\n", message);
    } else {
        printf("\n" COLOR_RED "[ERROR] %s" COLOR_RESET "\n", message);
    }
}
