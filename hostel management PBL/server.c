#include "server.h"
#include "utils.h"
#include "auth.h"
#include "student.h"
#include "allotment.h"
#include "room.h"
#include "reports.h"
#include "audit.h"
#include "web_assets.h"
#include <ctype.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
    #define close_socket closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define close_socket close
#endif

/* Helper to parse URL encoded parameters */
static void getURLParameter(const char *url, const char *key, char *output, int max_len) {
    char search_key[120];
    const char *p;
    
    output[0] = '\0';
    sprintf(search_key, "%s=", key);
    p = strstr(url, search_key);
    if (!p) {
        sprintf(search_key, "?%s=", key);
        p = strstr(url, search_key);
        if (!p) {
            sprintf(search_key, "&%s=", key);
            p = strstr(url, search_key);
        }
    }
    
    if (p) {
        int i = 0;
        p += strlen(search_key) - 1;
        p++; /* Skip '=' */
        
        while (*p != '\0' && *p != '&' && *p != ' ' && *p != 'H' && i < max_len - 1) {
            if (*p == '+') {
                output[i++] = ' ';
            } else if (*p == '%' && isxdigit((unsigned char)*(p+1)) && isxdigit((unsigned char)*(p+2))) {
                char hex[3];
                hex[0] = *(p+1);
                hex[1] = *(p+2);
                hex[2] = '\0';
                output[i++] = (char)strtol(hex, NULL, 16);
                p += 2;
            } else {
                output[i++] = *p;
            }
            p++;
        }
        output[i] = '\0';
    }
}

/* Serializers */
static void serializeStudentJSON(const Student *s, char *buf) {
    sprintf(buf, 
        "{"
        "\"studentId\":\"%s\","
        "\"name\":\"%s\","
        "\"dob\":\"%s\","
        "\"gender\":\"%s\","
        "\"aadhar\":\"%s\","
        "\"bloodGroup\":\"%s\","
        "\"religion\":\"%s\","
        "\"caste\":\"%s\","
        "\"nationality\":\"%s\","
        "\"email\":\"%s\","
        "\"phone\":\"%s\","
        "\"course\":\"%s\","
        "\"branch\":\"%s\","
        "\"semester\":\"%s\","
        "\"rollNo\":\"%s\","
        "\"fatherName\":\"%s\","
        "\"motherName\":\"%s\","
        "\"parentPhone\":\"%s\","
        "\"parentEmail\":\"%s\","
        "\"parentOccupation\":\"%s\","
        "\"parentIncome\":\"%s\","
        "\"guardianName\":\"%s\","
        "\"guardianRelation\":\"%s\","
        "\"guardianPhone\":\"%s\","
        "\"guardianAddress\":\"%s\","
        "\"permanentAddress\":\"%s\","
        "\"city\":\"%s\","
        "\"state\":\"%s\","
        "\"pincode\":\"%s\","
        "\"hostelName\":\"%s\","
        "\"roomNo\":\"%s\","
        "\"roomType\":\"%s\","
        "\"allotmentDate\":\"%s\","
        "\"checkoutDate\":\"%s\","
        "\"feePaid\":%.2f,"
        "\"feePending\":%.2f,"
        "\"isAllotted\":%d"
        "}",
        s->studentId, s->name, s->dob, s->gender, s->aadhar, s->bloodGroup,
        s->religion, s->caste, s->nationality, s->email, s->phone,
        s->course, s->branch, s->semester, s->rollNo,
        s->fatherName, s->motherName, s->parentPhone, s->parentEmail, s->parentOccupation, s->parentIncome,
        s->guardianName, s->guardianRelation, s->guardianPhone, s->guardianAddress,
        s->permanentAddress, s->city, s->state, s->pincode,
        s->hostelName, s->roomNo, s->roomType, s->allotmentDate, s->checkoutDate,
        s->feePaid, s->feePending, s->isAllotted
    );
}

static void serializeRoomJSON(const Room *r, char *buf) {
    sprintf(buf,
        "{"
        "\"hostelName\":\"%s\","
        "\"roomNo\":\"%s\","
        "\"roomType\":\"%s\","
        "\"capacity\":%d,"
        "\"occupancy\":%d,"
        "\"isUnderMaintenance\":%d"
        "}",
        r->hostelName, r->roomNo, r->roomType, r->capacity, r->occupancy, r->isUnderMaintenance
    );
}

void startWebServer(int port) {
#ifdef _WIN32
    WSADATA wsaData;
#endif
    SOCKET server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int keep_running = 1;

    printHeader("LAUNCHING WEB CONSOLE");
    printf("Starting C-based HTTP service...\n");

#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printStatusMessage("Winsock initialization failed!", 0);
        pressAnyKey();
        return;
    }
#endif

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printStatusMessage("Socket creation failed!", 0);
#ifdef _WIN32
        WSACleanup();
#endif
        pressAnyKey();
        return;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1"); /* Bind to localhost only */
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        printStatusMessage("Socket binding failed! Port 8080 might be in use.", 0);
        close_socket(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        pressAnyKey();
        return;
    }

    if (listen(server_fd, 10) == SOCKET_ERROR) {
        printStatusMessage("Socket listening failed!", 0);
        close_socket(server_fd);
#ifdef _WIN32
        WSACleanup();
#endif
        pressAnyKey();
        return;
    }

    printf(COLOR_GREEN "Web console listening at http://127.0.0.1:%d/" COLOR_RESET "\n", port);
    printf("Opening default web browser...\n");

#ifdef _WIN32
    system("start http://127.0.0.1:8080/");
#elif __APPLE__
    system("open http://127.0.0.1:8080/");
#else
    system("xdg-open http://127.0.0.1:8080/");
#endif

    printf("\nKeep this window open. Press Ctrl+C in terminal or click 'Exit Web Console' in dashboard to stop server.\n");

    while (keep_running) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        SOCKET client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        
        if (client_fd != INVALID_SOCKET) {
            char buffer[5000];
            int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                
                /* Simple routing based on HTTP headers request line */
                if (strncmp(buffer, "GET / HTTP/1.1", 14) == 0 || strncmp(buffer, "GET /index.html HTTP/1.1", 24) == 0) {
                    char resp_header[200];
                    sprintf(resp_header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", (int)strlen(HTML_CONTENT));
                    send(client_fd, resp_header, strlen(resp_header), 0);
                    send(client_fd, HTML_CONTENT, strlen(HTML_CONTENT), 0);
                }
                else if (strncmp(buffer, "GET /style.css HTTP/1.1", 23) == 0) {
                    char resp_header[200];
                    sprintf(resp_header, "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", (int)strlen(CSS_CONTENT));
                    send(client_fd, resp_header, strlen(resp_header), 0);
                    send(client_fd, CSS_CONTENT, strlen(CSS_CONTENT), 0);
                }
                else if (strncmp(buffer, "GET /app.js HTTP/1.1", 20) == 0) {
                    char resp_header[200];
                    sprintf(resp_header, "HTTP/1.1 200 OK\r\nContent-Type: application/javascript\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", (int)strlen(JS_CONTENT));
                    send(client_fd, resp_header, strlen(resp_header), 0);
                    send(client_fd, JS_CONTENT, strlen(JS_CONTENT), 0);
                }
                else if (strncmp(buffer, "POST /api/login", 15) == 0) {
                    char user[50], pass[50], pass_enc[50];
                    SystemConfig config;
                    getURLParameter(buffer, "user", user, sizeof(user));
                    getURLParameter(buffer, "pass", pass, sizeof(pass));
                    
                    strcpy(pass_enc, pass);
                    encryptDecrypt(pass_enc, strlen(pass_enc));
                    
                    loadConfig(&config);
                    
                    if (strcmp(user, config.adminUsername) == 0 && strcmp(pass_enc, config.adminPasswordEncrypted) == 0) {
                        const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":true}";
                        send(client_fd, resp, strlen(resp), 0);
                        writeAuditLog(user, "Logged in via Web Console.");
                    } else {
                        const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false}";
                        send(client_fd, resp, strlen(resp), 0);
                        writeAuditLog("SYSTEM", "Failed web login attempt.");
                    }
                }
                else if (strncmp(buffer, "GET /api/config", 15) == 0) {
                    SystemConfig config;
                    char json[300];
                    loadConfig(&config);
                    
                    sprintf(json, 
                        "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n"
                        "{\"adminUsername\":\"%s\",\"feeSingle\":%.2f,\"feeDouble\":%.2f,\"feeTriple\":%.2f,\"roomsPerHostel\":%d}",
                        config.adminUsername, config.feeSingle, config.feeDouble, config.feeTriple, config.roomsPerHostel
                    );
                    send(client_fd, json, strlen(json), 0);
                }
                else if (strncmp(buffer, "GET /api/students", 17) == 0) {
                    FILE *fp = fopen(FILE_STUDENTS, "rb");
                    const char *resp_header = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n[";
                    send(client_fd, resp_header, strlen(resp_header), 0);
                    
                    if (fp != NULL) {
                        Student s;
                        int first = 1;
                        char s_json[2000];
                        while (fread(&s, sizeof(Student), 1, fp) == 1) {
                            if (!first) {
                                send(client_fd, ",", 1, 0);
                            }
                            serializeStudentJSON(&s, s_json);
                            send(client_fd, s_json, strlen(s_json), 0);
                            first = 0;
                        }
                        fclose(fp);
                    }
                    send(client_fd, "]", 1, 0);
                }
                else if (strncmp(buffer, "GET /api/rooms", 14) == 0) {
                    FILE *fp = fopen(FILE_ROOMS, "rb");
                    const char *resp_header = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n[";
                    send(client_fd, resp_header, strlen(resp_header), 0);
                    
                    if (fp != NULL) {
                        Room r;
                        int first = 1;
                        char r_json[500];
                        while (fread(&r, sizeof(Room), 1, fp) == 1) {
                            if (!first) {
                                send(client_fd, ",", 1, 0);
                            }
                            serializeRoomJSON(&r, r_json);
                            send(client_fd, r_json, strlen(r_json), 0);
                            first = 0;
                        }
                        fclose(fp);
                    }
                    send(client_fd, "]", 1, 0);
                }
                else if (strncmp(buffer, "POST /api/students/add", 22) == 0) {
                    Student s;
                    FILE *fpId;
                    int maxId = 0;
                    
                    memset(&s, 0, sizeof(Student));
                    
                    /* Auto-generate unique ID on backend */
                    fpId = fopen(FILE_STUDENTS, "rb");
                    if (fpId != NULL) {
                        Student temp;
                        while (fread(&temp, sizeof(Student), 1, fpId) == 1) {
                            if (strncmp(temp.studentId, "STU-", 4) == 0) {
                                int idNum = atoi(temp.studentId + 4);
                                if (idNum > maxId) maxId = idNum;
                            }
                        }
                        fclose(fpId);
                    }
                    sprintf(s.studentId, "STU-%03d", (maxId + 1) % 1000);
                    
                    /* Extract details from query params */
                    getURLParameter(buffer, "name", s.name, sizeof(s.name));
                    getURLParameter(buffer, "dob", s.dob, sizeof(s.dob));
                    getURLParameter(buffer, "gender", s.gender, sizeof(s.gender));
                    getURLParameter(buffer, "aadhar", s.aadhar, sizeof(s.aadhar));
                    getURLParameter(buffer, "blood", s.bloodGroup, sizeof(s.bloodGroup));
                    getURLParameter(buffer, "nationality", s.nationality, sizeof(s.nationality));
                    getURLParameter(buffer, "email", s.email, sizeof(s.email));
                    getURLParameter(buffer, "phone", s.phone, sizeof(s.phone));
                    getURLParameter(buffer, "religion", s.religion, sizeof(s.religion));
                    getURLParameter(buffer, "caste", s.caste, sizeof(s.caste));
                    
                    getURLParameter(buffer, "course", s.course, sizeof(s.course));
                    getURLParameter(buffer, "branch", s.branch, sizeof(s.branch));
                    getURLParameter(buffer, "semester", s.semester, sizeof(s.semester));
                    getURLParameter(buffer, "roll", s.rollNo, sizeof(s.rollNo));
                    
                    getURLParameter(buffer, "father", s.fatherName, sizeof(s.fatherName));
                    getURLParameter(buffer, "mother", s.motherName, sizeof(s.motherName));
                    getURLParameter(buffer, "parentPhone", s.parentPhone, sizeof(s.parentPhone));
                    getURLParameter(buffer, "parentEmail", s.parentEmail, sizeof(s.parentEmail));
                    getURLParameter(buffer, "parentOcc", s.parentOccupation, sizeof(s.parentOccupation));
                    getURLParameter(buffer, "parentIncome", s.parentIncome, sizeof(s.parentIncome));
                    
                    getURLParameter(buffer, "guardName", s.guardianName, sizeof(s.guardianName));
                    getURLParameter(buffer, "guardRel", s.guardianRelation, sizeof(s.guardianRelation));
                    getURLParameter(buffer, "guardPhone", s.guardianPhone, sizeof(s.guardianPhone));
                    getURLParameter(buffer, "guardAddr", s.guardianAddress, sizeof(s.guardianAddress));
                    
                    getURLParameter(buffer, "addr", s.permanentAddress, sizeof(s.permanentAddress));
                    getURLParameter(buffer, "city", s.city, sizeof(s.city));
                    getURLParameter(buffer, "state", s.state, sizeof(s.state));
                    getURLParameter(buffer, "pincode", s.pincode, sizeof(s.pincode));
                    
                    s.isAllotted = 0;
                    
                    /* Validate essential properties */
                    if (strlen(s.name) == 0 || !validatePhone(s.phone) || !validateAadhar(s.aadhar) || !validateEmail(s.email) || !validateDate(s.dob)) {
                        const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Input field validation checks failed.\"}";
                        send(client_fd, resp, strlen(resp), 0);
                    } else {
                        FILE *fpAdd = fopen(FILE_STUDENTS, "ab");
                        if (fpAdd != NULL) {
                            char log_msg[200];
                            char resp_json[250];
                            
                            fwrite(&s, sizeof(Student), 1, fpAdd);
                            fclose(fpAdd);
                            
                            sprintf(log_msg, "Registered student %s (%s) via Web Dashboard.", s.name, s.studentId);
                            writeAuditLog("admin", log_msg);
                            
                            sprintf(resp_json, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":true,\"studentId\":\"%s\"}", s.studentId);
                            send(client_fd, resp_json, strlen(resp_json), 0);
                        } else {
                            const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Database write failed.\"}";
                            send(client_fd, resp, strlen(resp), 0);
                        }
                    }
                }
                else if (strncmp(buffer, "POST /api/allot", 15) == 0) {
                    char studentId[20], hostelName[40], roomNo[10], date[20], paid_str[30];
                    float paid;
                    SystemConfig config;
                    
                    getURLParameter(buffer, "studentId", studentId, sizeof(studentId));
                    getURLParameter(buffer, "hostelName", hostelName, sizeof(hostelName));
                    getURLParameter(buffer, "roomNo", roomNo, sizeof(roomNo));
                    getURLParameter(buffer, "date", date, sizeof(date));
                    getURLParameter(buffer, "paid", paid_str, sizeof(paid_str));
                    
                    paid = (float)atof(paid_str);
                    loadConfig(&config);
                    
                    /* Validate date format */
                    if (!validateDate(date)) {
                        const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Invalid date format (must be DD/MM/YYYY).\"}";
                        send(client_fd, resp, strlen(resp), 0);
                    }
                    else {
                        FILE *fpStu = fopen(FILE_STUDENTS, "r+b");
                        FILE *fpR = fopen(FILE_ROOMS, "r+b");
                        int sFound = 0, rFound = 0;
                        long stuPos = 0, roomPos = 0;
                        Student stu;
                        Room r;
                        
                        if (fpStu && fpR) {
                            while (fread(&stu, sizeof(Student), 1, fpStu) == 1) {
                                if (strcmp(stu.studentId, studentId) == 0) {
                                    sFound = 1;
                                    break;
                                }
                                stuPos = ftell(fpStu);
                            }
                            
                            while (fread(&r, sizeof(Room), 1, fpR) == 1) {
                                if (strcmp(r.hostelName, hostelName) == 0 && strcmp(r.roomNo, roomNo) == 0) {
                                    rFound = 1;
                                    break;
                                }
                                roomPos = ftell(fpR);
                            }
                            
                            if (sFound && rFound && !stu.isAllotted && r.occupancy < r.capacity && !r.isUnderMaintenance) {
                                float totalFee = 0.0f;
                                if (strcmp(r.roomType, "Single") == 0) totalFee = config.feeSingle;
                                else if (strcmp(r.roomType, "Double") == 0) totalFee = config.feeDouble;
                                else totalFee = config.feeTriple;
                                
                                if (paid < 0 || paid > totalFee) {
                                    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Invalid fee payment bounds.\"}";
                                    send(client_fd, resp, strlen(resp), 0);
                                } else {
                                    char log_msg[150];
                                    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":true}";
                                    
                                    /* Update room */
                                    r.occupancy++;
                                    fseek(fpR, roomPos, SEEK_SET);
                                    fwrite(&r, sizeof(Room), 1, fpR);
                                    
                                    /* Update student */
                                    strcpy(stu.hostelName, hostelName);
                                    strcpy(stu.roomNo, roomNo);
                                    strcpy(stu.roomType, r.roomType);
                                    strcpy(stu.allotmentDate, date);
                                    strcpy(stu.checkoutDate, "");
                                    stu.feePaid = paid;
                                    stu.feePending = totalFee - paid;
                                    stu.isAllotted = 1;
                                    
                                    fseek(fpStu, stuPos, SEEK_SET);
                                    fwrite(&stu, sizeof(Student), 1, fpStu);
                                    
                                    sprintf(log_msg, "Allotted room %s in %s to %s via Web Dashboard.", roomNo, hostelName, stu.name);
                                    writeAuditLog("admin", log_msg);
                                    
                                    send(client_fd, resp, strlen(resp), 0);
                                }
                            } else {
                                const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Validation constraints failed (room full or student already allotted).\"}";
                                send(client_fd, resp, strlen(resp), 0);
                            }
                        } else {
                            const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Database access error.\"}";
                            send(client_fd, resp, strlen(resp), 0);
                        }
                        if (fpStu) fclose(fpStu);
                        if (fpR) fclose(fpR);
                    }
                }
                else if (strncmp(buffer, "POST /api/vacate", 16) == 0) {
                    char studentId[20], date[20];
                    getURLParameter(buffer, "studentId", studentId, sizeof(studentId));
                    getURLParameter(buffer, "date", date, sizeof(date));
                    
                    if (!validateDate(date)) {
                        const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Invalid date structure.\"}";
                        send(client_fd, resp, strlen(resp), 0);
                    } else {
                        FILE *fpStu = fopen(FILE_STUDENTS, "r+b");
                        FILE *fpR = fopen(FILE_ROOMS, "r+b");
                        int sFound = 0;
                        long stuPos = 0;
                        Student stu;
                        
                        if (fpStu) {
                            while (fread(&stu, sizeof(Student), 1, fpStu) == 1) {
                                if (strcmp(stu.studentId, studentId) == 0) {
                                    sFound = 1;
                                    break;
                                }
                                stuPos = ftell(fpStu);
                            }
                            
                            if (sFound && stu.isAllotted) {
                                char log_msg[150];
                                const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":true}";
                                
                                /* Decrement room count */
                                if (fpR) {
                                    Room r;
                                    long roomPos = 0;
                                    while (fread(&r, sizeof(Room), 1, fpR) == 1) {
                                        if (strcmp(r.hostelName, stu.hostelName) == 0 && strcmp(r.roomNo, stu.roomNo) == 0) {
                                            if (r.occupancy > 0) r.occupancy--;
                                            fseek(fpR, roomPos, SEEK_SET);
                                            fwrite(&r, sizeof(Room), 1, fpR);
                                            break;
                                        }
                                        roomPos = ftell(fpR);
                                    }
                                }
                                
                                /* Vacate student */
                                strcpy(stu.checkoutDate, date);
                                stu.isAllotted = 0;
                                stu.feePaid = 0.0f;
                                stu.feePending = 0.0f;
                                
                                fseek(fpStu, stuPos, SEEK_SET);
                                fwrite(&stu, sizeof(Student), 1, fpStu);
                                
                                sprintf(log_msg, "Vacated room %s in %s. Student: %s (%s) via Web Dashboard.", stu.roomNo, stu.hostelName, stu.name, stu.studentId);
                                writeAuditLog("admin", log_msg);
                                
                                send(client_fd, resp, strlen(resp), 0);
                            } else {
                                const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Student is not allotted a room.\"}";
                                send(client_fd, resp, strlen(resp), 0);
                            }
                        }
                        if (fpStu) fclose(fpStu);
                        if (fpR) fclose(fpR);
                    }
                }
                else if (strncmp(buffer, "POST /api/transfer", 18) == 0) {
                    char studentId[20], hostelName[40], roomNo[10], type[20];
                    SystemConfig config;
                    FILE *fpStu;
                    FILE *fpR;
                    int sFound = 0, nrFound = 0;
                    long stuPos = 0, nrPos = 0;
                    Student stu;
                    Room nr;
                    
                    getURLParameter(buffer, "studentId", studentId, sizeof(studentId));
                    getURLParameter(buffer, "hostelName", hostelName, sizeof(hostelName));
                    getURLParameter(buffer, "roomNo", roomNo, sizeof(roomNo));
                    getURLParameter(buffer, "type", type, sizeof(type));
                    
                    loadConfig(&config);
                    
                    fpStu = fopen(FILE_STUDENTS, "r+b");
                    fpR = fopen(FILE_ROOMS, "r+b");
                    
                    if (fpStu && fpR) {
                        while (fread(&stu, sizeof(Student), 1, fpStu) == 1) {
                            if (strcmp(stu.studentId, studentId) == 0) {
                                sFound = 1;
                                break;
                            }
                            stuPos = ftell(fpStu);
                        }
                        
                        while (fread(&nr, sizeof(Room), 1, fpR) == 1) {
                            if (strcmp(nr.hostelName, hostelName) == 0 && strcmp(nr.roomNo, roomNo) == 0) {
                                nrFound = 1;
                                break;
                            }
                            nrPos = ftell(fpR);
                        }
                        
                        if (sFound && nrFound && stu.isAllotted && nr.occupancy < nr.capacity && !nr.isUnderMaintenance) {
                            Room or;
                            long orPos = 0;
                            char old_block[40], old_room[10];
                            float newTotalFee = 0.0f;
                            char log_msg[350];
                            const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":true}";

                            /* Decrement old room count */
                            fseek(fpR, 0, SEEK_SET);
                            while (fread(&or, sizeof(Room), 1, fpR) == 1) {
                                if (strcmp(or.hostelName, stu.hostelName) == 0 && strcmp(or.roomNo, stu.roomNo) == 0) {
                                    if (or.occupancy > 0) or.occupancy--;
                                    fseek(fpR, orPos, SEEK_SET);
                                    fwrite(&or, sizeof(Room), 1, fpR);
                                    break;
                                }
                                orPos = ftell(fpR);
                            }
                            
                            /* Increment new room count */
                            nr.occupancy++;
                            fseek(fpR, nrPos, SEEK_SET);
                            fwrite(&nr, sizeof(Room), 1, fpR);
                            
                            /* Update student */
                            strcpy(old_block, stu.hostelName);
                            strcpy(old_room, stu.roomNo);
                            
                            strcpy(stu.hostelName, hostelName);
                            strcpy(stu.roomNo, roomNo);
                            strcpy(stu.roomType, type);
                            
                            if (strcmp(type, "Single") == 0) newTotalFee = config.feeSingle;
                            else if (strcmp(type, "Double") == 0) newTotalFee = config.feeDouble;
                            else newTotalFee = config.feeTriple;
                            
                            stu.feePending = newTotalFee - stu.feePaid;
                            
                            fseek(fpStu, stuPos, SEEK_SET);
                            fwrite(&stu, sizeof(Student), 1, fpStu);
                            
                            sprintf(log_msg, "Transferred student %s (%s) from %s room %s to %s room %s via Web Dashboard.",
                                    stu.name, stu.studentId, old_block, old_room, hostelName, roomNo);
                            writeAuditLog("admin", log_msg);
                            
                            send(client_fd, resp, strlen(resp), 0);
                        } else {
                            const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Room unavailable or student unallotted.\"}";
                            send(client_fd, resp, strlen(resp), 0);
                        }
                    }
                    if (fpStu) fclose(fpStu);
                    if (fpR) fclose(fpR);
                }
                else if (strncmp(buffer, "POST /api/maintenance", 21) == 0) {
                    char hostelName[40], roomNo[10];
                    FILE *fp;
                    int found = 0;
                    long rPos = 0;
                    Room r;
                    
                    getURLParameter(buffer, "hostelName", hostelName, sizeof(hostelName));
                    getURLParameter(buffer, "roomNo", roomNo, sizeof(roomNo));
                    
                    fp = fopen(FILE_ROOMS, "r+b");
                    if (fp) {
                        while (fread(&r, sizeof(Room), 1, fp) == 1) {
                            if (strcmp(r.hostelName, hostelName) == 0 && strcmp(r.roomNo, roomNo) == 0) {
                                found = 1;
                                break;
                            }
                            rPos = ftell(fp);
                        }
                        if (found) {
                            char log_msg[150];
                            const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":true}";
                            
                            r.isUnderMaintenance = !r.isUnderMaintenance;
                            fseek(fp, rPos, SEEK_SET);
                            fwrite(&r, sizeof(Room), 1, fp);
                            
                            sprintf(log_msg, "Toggled room %s in %s maintenance to %s via Web.", r.roomNo, r.hostelName, r.isUnderMaintenance ? "ON" : "OFF");
                            writeAuditLog("admin", log_msg);
                            
                            send(client_fd, resp, strlen(resp), 0);
                        } else {
                            const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Room not found.\"}";
                            send(client_fd, resp, strlen(resp), 0);
                        }
                        fclose(fp);
                    }
                }
                else if (strncmp(buffer, "POST /api/config/update", 23) == 0) {
                    char feeS_str[30], feeD_str[30], feeT_str[30];
                    float fS, fD, fT;
                    SystemConfig config;
                    const char *resp_success = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":true}";
                    
                    getURLParameter(buffer, "feeSingle", feeS_str, sizeof(feeS_str));
                    getURLParameter(buffer, "feeDouble", feeD_str, sizeof(feeD_str));
                    getURLParameter(buffer, "feeTriple", feeT_str, sizeof(feeT_str));
                    
                    fS = (float)atof(feeS_str);
                    fD = (float)atof(feeD_str);
                    fT = (float)atof(feeT_str);
                    
                    if (fS < 0 || fD < 0 || fT < 0) {
                        const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Fees cannot be negative.\"}";
                        send(client_fd, resp, strlen(resp), 0);
                    } else {
                        loadConfig(&config);
                        config.feeSingle = fS;
                        config.feeDouble = fD;
                        config.feeTriple = fT;
                        saveConfig(&config);
                        
                        writeAuditLog("admin", "Updated pricing configurations via Web Dashboard.");
                        send(client_fd, resp_success, strlen(resp_success), 0);
                    }
                }
                else if (strncmp(buffer, "POST /api/password/change", 25) == 0) {
                    char currentPass[50], newPass[50], currPassEnc[50];
                    SystemConfig config;
                    const char *resp_success = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":true}";
                    
                    getURLParameter(buffer, "currentPass", currentPass, sizeof(currentPass));
                    getURLParameter(buffer, "newPass", newPass, sizeof(newPass));
                    
                    strcpy(currPassEnc, currentPass);
                    encryptDecrypt(currPassEnc, strlen(currPassEnc));
                    
                    loadConfig(&config);
                    
                    if (strcmp(currPassEnc, config.adminPasswordEncrypted) != 0) {
                        const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"Incorrect current password.\"}";
                        send(client_fd, resp, strlen(resp), 0);
                    } else if (strlen(newPass) < 6) {
                        const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":false,\"message\":\"New password must be at least 6 characters.\"}";
                        send(client_fd, resp, strlen(resp), 0);
                    } else {
                        strcpy(config.adminPasswordEncrypted, newPass);
                        encryptDecrypt(config.adminPasswordEncrypted, strlen(config.adminPasswordEncrypted));
                        saveConfig(&config);
                        
                        writeAuditLog("admin", "Changed administrative credentials via Web Dashboard.");
                        send(client_fd, resp_success, strlen(resp_success), 0);
                    }
                }
                else if (strncmp(buffer, "POST /api/reset", 15) == 0) {
                    /* Wipe databases, re-seed configuration */
                    FILE *fpStu;
                    SystemConfig config;
                    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":true}";
                    
                    fpStu = fopen(FILE_STUDENTS, "wb");
                    if (fpStu) fclose(fpStu);
                    
                    strcpy(config.adminUsername, "admin");
                    strcpy(config.adminPasswordEncrypted, "password123");
                    encryptDecrypt(config.adminPasswordEncrypted, strlen(config.adminPasswordEncrypted));
                    config.roomsPerHostel = 10;
                    config.feeSingle = 50000.0f;
                    config.feeDouble = 35000.0f;
                    config.feeTriple = 25000.0f;
                    saveConfig(&config);
                    
                    /* Seed rooms */
                    seedMockData(config.roomsPerHostel);
                    
                    writeAuditLog("SYSTEM", "Global administrative wipe triggered via Web Dashboard.");
                    send(client_fd, resp, strlen(resp), 0);
                }
                else if (strncmp(buffer, "GET /api/audit", 14) == 0) {
                    FILE *fp = fopen(FILE_AUDIT, "r");
                    const char *resp_header = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n[";
                    send(client_fd, resp_header, strlen(resp_header), 0);
                    
                    if (fp != NULL) {
                        char line[256];
                        int first = 1;
                        char escaped[512];
                        int i_e, i_l;
                        
                        while (fgets(line, sizeof(line), fp) != NULL) {
                            /* Trim trailing newline */
                            int len = strlen(line);
                            if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
                            if (len > 1 && line[len-2] == '\r') line[len-2] = '\0';
                            
                            if (!first) {
                                send(client_fd, ",", 1, 0);
                            }
                            
                            /* Escape backslashes and double quotes in log line */
                            i_e = 0;
                            i_l = 0;
                            escaped[i_e++] = '\"';
                            while (line[i_l] != '\0' && i_e < 510) {
                                if (line[i_l] == '\"' || line[i_l] == '\\') {
                                    escaped[i_e++] = '\\';
                                }
                                escaped[i_e++] = line[i_l++];
                            }
                            escaped[i_e++] = '\"';
                            escaped[i_e] = '\0';
                            
                            send(client_fd, escaped, strlen(escaped), 0);
                            first = 0;
                        }
                        fclose(fp);
                    }
                    send(client_fd, "]", 1, 0);
                }
                else if (strncmp(buffer, "POST /api/shutdown", 18) == 0) {
                    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"success\":true}";
                    send(client_fd, resp, strlen(resp), 0);
                    keep_running = 0;
                }
                else {
                    const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n404 Route Not Found";
                    send(client_fd, not_found, strlen(not_found), 0);
                }
            }
            close_socket(client_fd);
        }
    }
    
    close_socket(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    printStatusMessage("Web Console service stopped successfully.", 1);
}
