#ifndef UTILS_H
#define UTILS_H

#include "common.h"

/* Global Hostel Names list */
extern const char* HOSTELS[HOSTEL_COUNT];

/* Security & Encryption */
void encryptDecrypt(char *data, int len);

/* Input Handling Utilities (Buffer Overflow Protection) */
void getSafeString(char *buffer, int maxLen);
void getMaskedPassword(char *password, int maxLen);
int getSafeInt(void);
float getSafeFloat(void);
void clearInputBuffer(void);

/* String & Format Validations */
int validatePhone(const char *phone);
int validateAadhar(const char *aadhar);
int validateEmail(const char *email);
int validateDate(const char *date);
int validateNumeric(const char *str);

/* UI & Console Utilities */
void clearScreen(void);
void pressAnyKey(void);
void printDivider(char ch, int length);
void printHeader(const char *title);
void printStatusMessage(const char *message, int success);

#endif /* UTILS_H */
