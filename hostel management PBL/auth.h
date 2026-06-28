#ifndef AUTH_H
#define AUTH_H

#include "common.h"

/* Initialise configurations and database files if not present */
int initializeSystem(void);

/* Performs authentication, returns 1 on success, 0 on failure */
int adminLogin(char *currentAdminUser);

/* Admin changes their own password */
int changeAdminPassword(const char *currentAdminUser);

/* Configures total rooms per hostel and room fees */
int updateSystemConfig(const char *currentAdminUser);

/* Resets all system databases back to default state */
int resetSystemData(const char *currentAdminUser);

/* Safe loading of config */
int loadConfig(SystemConfig *config);

/* Safe saving of config */
int saveConfig(const SystemConfig *config);

/* Seeds default room and student records for demo/reset purposes */
void seedMockData(int roomsCount);

#endif /* AUTH_H */

