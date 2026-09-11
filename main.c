// HEDER FILES------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// NETWORKING HEADERS
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

// MECRO DEFINE------------------------------------------------
#define BRANCHES 6
#define DAYS 6
#define LECTURES 5
#define SUBJECTS_PER_BRANCH 7
#define TOTAL_TEACHERS 35
#define PORT 8080
#define BUFFER_SIZE 32768

// LINKED LIST ------------------------------------------------
struct student {
    int roll;
    char name[20];
    char branch[10];
    struct student *next;
};
struct student *head = NULL;

// STACK (UNDO DELETE) ----------------------------------------
struct stack {
    int roll;
    char name[20];
    char branch[10];
    struct stack *next;
};
struct stack *top = NULL;

// QUEUE (FEE) ------------------------------------------------
struct fee {
    int roll;
    struct fee *next;
};
struct fee *f_front = NULL, *f_rear = NULL;

// QUEUE (COMPLAINT) -----------------------------------------
struct complaint {
    int roll;
    char issue[50];
    struct complaint *next;
};
struct complaint *c_front = NULL, *c_rear = NULL;

// FUNCTION DECLARATIONS -------------------------------------
void showSchedule(int b, char *response);
void addStudent(int roll, char *name, char *branch, char *response);
void deleteStudent(int roll, char *response);
void displayStudents(char *response);
void undoDelete(char *response);
void feeSubmission(int roll, char *response);
void processFee(char *response);
void addComplaint(int roll, char *issue, char *response);
void processComplaint(char *response);
void showSubjectsByBranch(int b, char *response);
void showBookSuggestion(int b, int sub, char *response);
void getStats(char *response);
int selectBranch(char *branch_str);
void handle_request(char *request, char *response);
void send_response(SOCKET client, char *response);
void url_decode(char *src, char *dest);

//   BRANCHES -----------------------------------------------
char branches[BRANCHES][10] = {"CS","EX","EC","AIDS","CYBER","ME"};

// SUBJECTS PER BRANCH ---------------------------------------
char branchSubjects[BRANCHES][SUBJECTS_PER_BRANCH][30] = {
    {"DSA","OS","DBMS","CN","AI","SE","Compiler"},
    {"Maths","Circuits","Signals","Control","PowerSys","EMFT","Measurements"},
    {"Networks","Circuits","OS","Microprocessor","VLSI","Signals","Comm"},
    {"AI","ML","Python","Maths","DL","Statistics","DSA"},
    {"Security","EthicalHack","Networks","OS","Crypto","Forensics","WebSec"},
    {"Mechanics","Thermodynamics","FluidMech","MachineDesign","Production","Kinematics","Materials"}
};

// TEACHERS NAMES ---------------------------------------------
char teachers[TOTAL_TEACHERS][30] = {
    "Dr. A","Dr. B","Dr. C","Dr. D","Dr. E",
    "Prof. Sharma","Prof. Verma","Prof. Singh","Prof. Gupta","Prof. Mishra",
    "Dr. Raj","Dr. Aman","Dr. Neha","Dr. Pooja","Dr. Rohit",
    "Prof. Mehta","Prof. Jain","Prof. Khanna","Prof. Saxena","Prof. Tiwari",
    "Dr. Kumar","Dr. Yadav","Dr. Arjun","Dr. Suresh","Dr. Mahesh",
    "Prof. Anil","Prof. Sunita","Prof. Kavita","Prof. Rakesh","Prof. Deepak",
    "Dr. Nitin","Dr. Alok","Dr. Pankaj","Dr. Shalini","Prof. Sarma"
};

// BOOK SUGGESTIONS PER SUBJECT AND BRANCH --------------------
char books[BRANCHES][SUBJECTS_PER_BRANCH][2][50] = {
    {
        {"Data Structures in C - Reema Thareja", "DSA Made Easy - Narasimha Karumanchi"},
        {"Operating System Concepts - Silberschatz", "Modern OS - Andrew Tanenbaum"},
        {"Database System Concepts - Korth", "SQL Complete Reference - Groff"},
        {"Computer Networks - Forouzan", "Computer Networking - Tanenbaum"},
        {"Artificial Intelligence - Stuart Russell", "AI Basics - Elaine Rich"},
        {"Software Engineering - Pressman", "Clean Code - Robert C. Martin"},
        {"Compilers - Aho", "Compiler Design - Ullman"}
    },
    {
        {"Advanced Engineering Maths - Erwin Kreyszig", "Higher Engineering Maths - B.S. Grewal"},
        {"Electrical Circuits - Nilsson", "Circuits Theory - Boylestad"},
        {"Signals & Systems - Oppenheim", "Signal Processing - Proakis"},
        {"Control Systems - Nagrath", "Modern Control - Ogata"},
        {"Power System - C.L. Wadhwa", "Power System Analysis - Hadi Saadat"},
        {"EMFT - Sadiku", "Electromagnetics - Kraus"},
        {"Measurements - Raghuwanshi", "Electrical Measurements - Golding"}
    },
    {
        {"Computer Networks - Forouzan", "Data Communications - Stallings"},
        {"Electronic Circuits - Sedra Smith", "Microelectronics - Boylestad"},
        {"Operating System - Galvin", "OS Concepts - Silberschatz"},
        {"Microprocessor - Gaonkar", "8086 Programming - Liu"},
        {"VLSI Design - Weste", "CMOS VLSI - Kang"},
        {"Signals & Systems - Oppenheim", "DSP - Proakis"},
        {"Communication Systems - Simon Haykin", "Digital Comm - Sklar"}
    },
    {
        {"Artificial Intelligence - Russell", "AI Basics - Rich"},
        {"Machine Learning - Tom Mitchell", "Hands-On ML - Aurélien Géron"},
        {"Python Crash Course - Eric Matthes", "Learning Python - Mark Lutz"},
        {"Engineering Maths - Kreyszig", "Advanced Maths - Grewal"},
        {"Deep Learning - Ian Goodfellow", "DL with Python - Chollet"},
        {"Statistics - Sheldon Ross", "Probability - Papoulis"},
        {"Data Structures - Karumanchi", "DSA - Reema Thareja"}
    },
    {
        {"Network Security - William Stallings", "Cyber Security - Nina Godbole"},
        {"Ethical Hacking - Ankit Fadia", "CEH Guide - Matt Walker"},
        {"Computer Networks - Tanenbaum", "Data Comm - Forouzan"},
        {"Operating System - Silberschatz", "Modern OS - Tanenbaum"},
        {"Cryptography - William Stallings", "Crypto - Katz"},
        {"Digital Forensics - Nelson", "Forensics Guide - Casey"},
        {"Web Security - Dafydd Stuttard", "OWASP Guide - Andrew Hoffman"}
    },
    {
        {"Engineering Mechanics - Bansal", "Mechanics - Hibbeler"},
        {"Thermodynamics - P.K. Nag", "Thermo - Cengel"},
        {"Fluid Mechanics - Fox", "FM - Raghuwanshi"},
        {"Machine Design - Bhandari", "Design - Shigley"},
        {"Production Engineering - Raghuwanshi", "Manufacturing - Kalpakjian"},
        {"Kinematics - Raghuwanshi", "Theory of Machines - Thomas"},
        {"Material Science - Callister", "Engineering Materials - Smith"}
    }
};

// GLOBAL------------------------------------------------------
int schedule[BRANCHES][DAYS][LECTURES];       
int teacherAssigned[BRANCHES][DAYS][LECTURES]; 
int teacherBusy[TOTAL_TEACHERS][DAYS][LECTURES];

// generateTimetable ------------------------------------------
void generateTimetable() 
{
    srand(time(NULL));
    memset(teacherBusy, 0, sizeof(teacherBusy));

    for (int b = 0; b < BRANCHES; b++) 
    {
        for (int d = 0; d < DAYS; d++) 
        {
            int used[SUBJECTS_PER_BRANCH] = {0};

            for (int l = 0; l < LECTURES; l++) 
            {
                int tries = 0;

                while (tries < 100) 
                {
                    int sub = rand() % SUBJECTS_PER_BRANCH;
                    int t = rand() % TOTAL_TEACHERS;

                    if (!used[sub] && !teacherBusy[t][d][l]) 
                    {
                        schedule[b][d][l] = sub;
                        teacherAssigned[b][d][l] = t;
                        teacherBusy[t][d][l] = 1;
                        used[sub] = 1;
                        break;
                    }
                    tries++;
                }
            }
        }
    }
}

// URL DECODE -------------------------------------------------
void url_decode(char *src, char *dest) {
    char *p = src;
    char *q = dest;
    while (*p) {
        if (*p == '%') {
            int code;
            if (p[1] && p[2]) {
                char hex[3] = {p[1], p[2], 0};
                code = strtol(hex, NULL, 16);
                *q++ = code;
                p += 3;
            }
        } else if (*p == '+') {
            *q++ = ' ';
            p++;
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
}

// BRANCH SELECT BY NAME----------------------------------------
int selectBranch(char *branch_str) {
    for (int i = 0; i < BRANCHES; i++) {
        if (strcmp(branches[i], branch_str) == 0) {
            return i;
        }
    }
    return -1;
}

// GET STATS ---------------------------------------------------
void getStats(char *response) {
    int student_count = 0, undo_count = 0, fee_count = 0, complaint_count = 0;
    struct student *s = head;
    struct stack *st = top;
    struct fee *f = f_front;
    struct complaint *c = c_front;
    
    while (s) { student_count++; s = s->next; }
    while (st) { undo_count++; st = st->next; }
    while (f) { fee_count++; f = f->next; }
    while (c) { complaint_count++; c = c->next; }
    
    sprintf(response, 
            "{\"students\":%d,\"undo\":%d,\"fee\":%d,\"complaints\":%d}",
            student_count, undo_count, fee_count, complaint_count);
}

// STUDENT ADD FUNCTIONS --------------------------------------
void addStudent(int roll, char *name, char *branch, char *response) {
    struct student *n = (struct student*)malloc(sizeof(struct student));
    n->roll = roll;
    strcpy(n->name, name);
    strcpy(n->branch, branch);
    n->next = head;
    head = n;
    sprintf(response, "{\"status\":\"success\",\"message\":\"Student Added Successfully\"}");
}

// STUDENT DELETE FUNCTIONS -----------------------------------
void deleteStudent(int roll, char *response) {
    struct student *t = head, *p = NULL;

    while (t && t->roll != roll) {
        p = t;
        t = t->next;
    }

    if (!t) {
        sprintf(response, "{\"status\":\"error\",\"message\":\"Student not found\"}");
        return;
    }

    struct stack *s = (struct stack*)malloc(sizeof(struct stack));
    s->roll = t->roll;
    strcpy(s->name, t->name);
    strcpy(s->branch, t->branch);
    s->next = top;
    top = s;

    if (p)
        p->next = t->next;
    else
        head = t->next;

    free(t);
    sprintf(response, "{\"status\":\"success\",\"message\":\"Student Deleted (Undo Available)\"}");
}

// DELETE UNDO FUNCTIONS --------------------------------------
void undoDelete(char *response) {
    if (!top) {
        sprintf(response, "{\"status\":\"error\",\"message\":\"Nothing to Undo\"}");
        return;
    }

    struct student *n = (struct student*)malloc(sizeof(struct student));
    n->roll = top->roll;
    strcpy(n->name, top->name);
    strcpy(n->branch, top->branch);
    n->next = head;
    head = n;

    struct stack *temp = top;
    top = top->next;
    free(temp);

    sprintf(response, "{\"status\":\"success\",\"message\":\"Undo Successful\"}");
}

// STUDENT DISPLAY FUNCTIONS ----------------------------------
void displayStudents(char *response) {
    struct student *t = head;
    char temp[BUFFER_SIZE] = "";
    char row[200];
    
    strcpy(response, "[");
    
    while (t) {
        sprintf(row, "{\"roll\":%d,\"name\":\"%s\",\"branch\":\"%s\"}", 
                t->roll, t->name, t->branch);
        strcat(response, row);
        t = t->next;
        if (t) strcat(response, ",");
    }
    
    strcat(response, "]");
}

// FEE QUEUE -------------------------------------------------
void feeSubmission(int roll, char *response) {
    struct fee *n = (struct fee*)malloc(sizeof(struct fee));
    n->roll = roll;
    n->next = NULL;

    if (!f_rear)
        f_front = f_rear = n;
    else {
        f_rear->next = n;
        f_rear = n;
    }

    sprintf(response, "{\"status\":\"success\",\"message\":\"Fee Submitted (FIFO)\"}");
}

// FEE QUEUE PROCESS------------------------------------------
void processFee(char *response) {
    if (!f_front) {
        sprintf(response, "{\"status\":\"error\",\"message\":\"No pending fees\"}");
        return;
    }

    struct fee *t = f_front;
    int roll = t->roll;
    
    f_front = f_front->next;
    if (!f_front) f_rear = NULL;
    free(t);
    
    sprintf(response, "{\"status\":\"success\",\"message\":\"Fee Processed for Roll No: %d\",\"roll\":%d}", roll, roll);
}

// COMPLAINT QUEUE ------------------------------------------
void addComplaint(int roll, char *issue, char *response) {
    struct complaint *n = (struct complaint*)malloc(sizeof(struct complaint));
    n->roll = roll;
    strcpy(n->issue, issue);
    n->next = NULL;

    if (!c_rear)
        c_front = c_rear = n;
    else {
        c_rear->next = n;
        c_rear = n;
    }

    sprintf(response, "{\"status\":\"success\",\"message\":\"Complaint Added Successfully\"}");
}

// COMPLAINT QUEUE PROCESS---------------------------------
void processComplaint(char *response) {
    if (!c_front) {
        sprintf(response, "{\"status\":\"error\",\"message\":\"No pending complaints\"}");
        return;
    }

    struct complaint *t = c_front;
    int roll = t->roll;
    char issue[50];
    strcpy(issue, t->issue);

    c_front = c_front->next;
    if (!c_front) c_rear = NULL;
    free(t);
    
    sprintf(response, "{\"status\":\"success\",\"message\":\"Complaint Resolved (Roll %d): %s\",\"roll\":%d,\"issue\":\"%s\"}", 
            roll, issue, roll, issue);
}

// BookSuggestion ------------------------------------------
void showBookSuggestion(int b, int sub, char *response) {
    if (b < 0 || b >= BRANCHES) {
        sprintf(response, "{\"status\":\"error\",\"message\":\"Invalid Branch\"}");
        return;
    }
    
    if (sub < 0 || sub >= SUBJECTS_PER_BRANCH) {
        sprintf(response, "{\"status\":\"error\",\"message\":\"Invalid Subject\"}");
        return;
    }

    sprintf(response, 
            "{\"status\":\"success\",\"branch\":\"%s\",\"subject\":\"%s\","
            "\"book1\":\"%s\",\"book2\":\"%s\"}",
            branches[b], branchSubjects[b][sub],
            books[b][sub][0], books[b][sub][1]);
}

// SubjectsByBranch ------------------------------------------
void showSubjectsByBranch(int b, char *response) {
    if (b < 0 || b >= BRANCHES) {
        sprintf(response, "{\"status\":\"error\",\"message\":\"Invalid Branch\"}");
        return;
    }

    char temp[BUFFER_SIZE] = "";
    char subject[100];
    
    sprintf(response, "{\"branch\":\"%s\",\"subjects\":[", branches[b]);
    
    for (int i = 0; i < SUBJECTS_PER_BRANCH; i++) {
        sprintf(subject, "\"%s\"", branchSubjects[b][i]);
        strcat(response, subject);
        if (i < SUBJECTS_PER_BRANCH - 1) strcat(response, ",");
    }
    
    strcat(response, "]}");
}

// Schedule ---------------------------------------------------
void showSchedule(int b, char *response) {
    static int generated = 0;
    if (!generated) {
        generateTimetable();
        generated = 1;
    }

    if (b < 0 || b >= BRANCHES) {
        sprintf(response, "{\"status\":\"error\",\"message\":\"Invalid Branch\"}");
        return;
    }

    char days[DAYS][10] = {
        "Monday","Tuesday","Wednesday",
        "Thursday","Friday","Saturday"
    };

    char temp[BUFFER_SIZE] = "";
    char entry[500];
    
    sprintf(response, "{\"branch\":\"%s\",\"schedule\":[", branches[b]);

    for (int d = 0; d < DAYS; d++) {
        sprintf(entry, "{\"day\":\"%s\",\"lectures\":[", days[d]);
        strcat(response, entry);
        
        for (int l = 0; l < LECTURES; l++) {
            int s = schedule[b][d][l];
            int t = teacherAssigned[b][d][l];
            sprintf(entry, "{\"lecture\":%d,\"subject\":\"%s\",\"teacher\":\"%s\"}", 
                    l+1, branchSubjects[b][s], teachers[t]);
            strcat(response, entry);
            if (l < LECTURES - 1) strcat(response, ",");
        }
        
        strcat(response, "]}");
        if (d < DAYS - 1) strcat(response, ",");
    }
    
    strcat(response, "]}");
}

// HANDLE HTTP REQUEST ----------------------------------------
void handle_request(char *request, char *response) {
    char method[10], path[100], protocol[10];
    sscanf(request, "%s %s %s", method, path, protocol);
    
    // Serve static files
    if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
        FILE *f = fopen("index.html", "r");
        if (f) {
            char *resp = response;
            resp += sprintf(resp, "HTTP/1.1 200 OK\r\n");
            resp += sprintf(resp, "Content-Type: text/html\r\n");
            resp += sprintf(resp, "\r\n");
            
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), f)) {
                resp += sprintf(resp, "%s", buffer);
            }
            fclose(f);
            return;
        } else {
            sprintf(response, "HTTP/1.1 404 Not Found\r\n\r\nFile not found");
            return;
        }
    }
    
    // API endpoints
    if (strncmp(path, "/api/", 5) == 0) {
        char *json_response = response + 200; // Reserve space for HTTP header
        char temp[BUFFER_SIZE] = "";
        
        if (strcmp(path, "/api/stats") == 0) {
            getStats(temp);
        }
        else if (strncmp(path, "/api/students", 13) == 0) {
            if (strcmp(method, "GET") == 0) {
                displayStudents(temp);
            }
            else if (strcmp(method, "POST") == 0) {
                char *body = strstr(request, "\r\n\r\n");
                if (body) {
                    body += 4;
                    int roll; char name[20], branch[10];
                    if (sscanf(body, "roll=%d&name=%[^&]&branch=%s", &roll, name, branch) == 3) {
                        url_decode(name, name);
                        addStudent(roll, name, branch, temp);
                    }
                }
            }
            else if (strncmp(method, "DELETE", 6) == 0) {
                int roll;
                if (sscanf(path, "/api/students/%d", &roll) == 1) {
                    deleteStudent(roll, temp);
                }
            }
        }
        else if (strcmp(path, "/api/undo") == 0) {
            undoDelete(temp);
        }
        else if (strcmp(path, "/api/fee") == 0) {
            if (strcmp(method, "POST") == 0) {
                char *body = strstr(request, "\r\n\r\n");
                if (body) {
                    body += 4;
                    int roll;
                    if (sscanf(body, "roll=%d", &roll) == 1) {
                        feeSubmission(roll, temp);
                    }
                }
            } else if (strcmp(method, "DELETE") == 0) {
                processFee(temp);
            }
        }
        else if (strcmp(path, "/api/complaint") == 0) {
            if (strcmp(method, "POST") == 0) {
                char *body = strstr(request, "\r\n\r\n");
                if (body) {
                    body += 4;
                    int roll; char issue[50];
                    if (sscanf(body, "roll=%d&issue=%[^\n]", &roll, issue) == 2) {
                        url_decode(issue, issue);
                        addComplaint(roll, issue, temp);
                    }
                }
            } else if (strcmp(method, "DELETE") == 0) {
                processComplaint(temp);
            }
        }
        else if (strncmp(path, "/api/subjects/", 14) == 0) {
            char branch[10];
            if (sscanf(path, "/api/subjects/%s", branch) == 1) {
                int b = selectBranch(branch);
                showSubjectsByBranch(b, temp);
            }
        }
        else if (strncmp(path, "/api/books/", 11) == 0) {
            char branch[10]; int sub;
            if (sscanf(path, "/api/books/%[^/]/%d", branch, &sub) == 2) {
                int b = selectBranch(branch);
                showBookSuggestion(b, sub-1, temp);
            }
        }
        else if (strncmp(path, "/api/schedule/", 14) == 0) {
            char branch[10];
            if (sscanf(path, "/api/schedule/%s", branch) == 1) {
                int b = selectBranch(branch);
                showSchedule(b, temp);
            }
        }
        
        sprintf(response, "HTTP/1.1 200 OK\r\n");
        strcat(response, "Content-Type: application/json\r\n");
        strcat(response, "Access-Control-Allow-Origin: *\r\n");
        strcat(response, "\r\n");
        strcat(response, temp);
        return;
    }
    
    // 404 for other paths
    sprintf(response, "HTTP/1.1 404 Not Found\r\n\r\n");
}

// SEND RESPONSE ---------------------------------------------
void send_response(SOCKET client, char *response) {
    send(client, response, strlen(response), 0);
}

// MAIN ------------------------------------------------------
int main() {
    #ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    #endif
    
    SOCKET server_fd, client_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    
    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return 1;
    }
    
    // Set socket options
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("Bind failed\n");
        return 1;
    }
    
    // Listen
    if (listen(server_fd, 3) < 0) {
        printf("Listen failed\n");
        return 1;
    }
    
    printf("\n========================================\n");
    printf("SMART COLLEGE MANAGEMENT SYSTEM - SERVER\n");
    printf("========================================\n");
    printf("Server running on http://localhost:%d\n", PORT);
    printf("Press Ctrl+C to stop\n");
    printf("========================================\n\n");
    
    // Generate timetable once at start
    generateTimetable();
    
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_fd == INVALID_SOCKET) {
            printf("Accept failed\n");
            continue;
        }
        
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_fd, buffer, BUFFER_SIZE, 0);
        
        printf("Received request\n");
        
        memset(response, 0, BUFFER_SIZE);
        handle_request(buffer, response);
        send_response(client_fd, response);
        
        closesocket(client_fd);
    }
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    return 0;
}