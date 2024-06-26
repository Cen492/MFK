#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <poll.h>
#include "mfk.h"
#include <pigpio.h>
#include <pthread.h>

char Mcode[256]; //to store the machine code
#define BUFFER_SIZE 1024
char message[BUFFER_SIZE];
char secret[SHA256_DIGEST_LENGTH * 2 + 1];
char lastHash[SHA256_DIGEST_LENGTH * 2 + 1];
int found;

// Motor control pins
const int IN1 = 24;
const int IN2 = 23;
const int ENA = 25;

// Initializes the OpenSSL library
void init_openssl() {
    SSL_library_init();
    SSL_load_error_strings();
}

// Creates an SSL context for the client
SSL_CTX* create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_client_method();
    ctx = SSL_CTX_new(method);

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

// Attack-------------------------------------
struct VulnStruct {
    char buf[64];
    void (*func_ptr)(char* arg);
};

void new(char *name) {
    printf("state of the motor is: %s\n", name);
}

void foo(char *arg) {
    struct VulnStruct local_struct;
    local_struct.func_ptr = new;
    strcpy(local_struct.buf, arg);
    (local_struct.func_ptr)(local_struct.buf);
}
// Attack--------------------------------------

// Motor functions
void startMotor() {
    memcpy(Mcode, (void *)&startMotor, 144); // store startMotor Machine code in Mcode
    strcpy(secret, check(secret, Mcode)); // concatenate Mcode and secret (previous hash), then hash them and store the value in secret

    gpioWrite(IN1, 1);
    gpioWrite(IN2, 0);
}

void stopMotor() {
    memcpy(Mcode, (void *)&stopMotor, 156); // store stopMotor Machine code in Mcode
    strcpy(secret, check(secret, Mcode));
    strcpy(lastHash, secret);

    gpioWrite(IN1, 0);
    gpioWrite(IN2, 0);
    gpioWrite(ENA, 0);
}

void lowSpeed() {
    memcpy(Mcode, (void *)&lowSpeed, 132);  // store lowSpeed Machine code in Mcode
    strcpy(secret, check(secret, Mcode));// concatenate Mcode and secret (previous hash), then hash them and store the value in secret
    strcpy(lastHash, secret); // store secret in lastHash since it represent the last node on the call graph , this will be sent to the server

    gpioPWM(ENA, 50);
}

void highSpeed() {
    memcpy(Mcode, (void *)&stopMotor, 132);  // store highSpeed Machine code in Mcode
    strcpy(secret, check(secret, Mcode)); 
    strcpy(lastHash, secret); 

    gpioPWM(ENA, 70);
}

void check_hash() { // to check if the hash exist in the file or not 
    if (CONTROL == 1) {// 1 = offline mode , if so the hashes will be stored in the file 
        FILE* fp = fopen("hash.txt", "r");
        char line[BUFFER_SIZE];
        while (fgets(line, sizeof(line), fp) != NULL) {
            // Remove trailing newline character
            strtok(line, "\n");
            if (strcmp(line, lastHash) == 0) {
                fclose(fp);
                found = 1; // Hash found in the file
            } else {
                found = 0;
            }
        }
        if (!found) {
            fclose(fp);
            FILE* fp = fopen("hash.txt", "a"); // Open in append mode
            if (fp != NULL) {
                fprintf(fp, "%s\n", lastHash);
                fclose(fp);
            } else {
                perror(" fopen failed");
            }
        }
    }
}

int main() {
    // GPIO library initialization and motor control pin setup
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialisation failed\n");
        return 1;
    }
    strcpy(lastHash, "");
    gpioSetMode(IN1, PI_OUTPUT);
    gpioSetMode(IN2, PI_OUTPUT);
    gpioSetMode(ENA, PI_OUTPUT);

    // Server connection setup
    const int num_servers = 2;
    int client_fd[num_servers];
    struct sockaddr_in server_addr[num_servers];
    SSL *ssl[num_servers];

    // OpenSSL initialization and context creation
    init_openssl();
    SSL_CTX *ctx = create_context();

    // Define server addresses and ports
    char *server_ips[num_servers];
    server_ips[1] = "172.20.10.8";
    server_ips[0] = "172.20.10.8";

    int server_ports[num_servers];
    server_ports[1] = 12346;
    server_ports[0] = 12345;

    // Connect to each server
    for (int i = 0; i < num_servers; i++) {
        if ((client_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        server_addr[i].sin_family = AF_INET;
        server_addr[i].sin_port = htons(server_ports[i]);

        if (inet_pton(AF_INET, server_ips[i], &server_addr[i].sin_addr) <= 0) {
            perror("Invalid address/Address not supported");
            exit(EXIT_FAILURE);
        }

        if (connect(client_fd[i], (struct sockaddr *)&server_addr[i], sizeof(server_addr[i])) < 0) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }

        ssl[i] = SSL_new(ctx);
        SSL_set_fd(ssl[i], client_fd[i]);

        if (SSL_connect(ssl[i]) <= 0) {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        } else {
            printf("SSL connection established with server %d\n", i + 1);
        }
    }

    // Poll setup for monitoring multiple file descriptors
    struct pollfd fds[num_servers];
    for (int i = 0; i < num_servers; i++) {
        fds[i].fd = client_fd[i];
        fds[i].events = POLLIN;
    }

    while (1) {
        strcpy(secret, "");
        char* a;
        strcpy(Mcode, "");
        for (int i = 0; i < num_servers; i++) {
            fds[i].fd = client_fd[i];
            fds[i].events = POLLIN;
        }
        memcpy(Mcode, (void *)&main, 255); // store main Machine code in Mcode

        int ret = poll(fds, num_servers, -1);

        if (ret < 0) {
            perror("poll() failed");
            break;
        }

        for (int i = 0; i < num_servers; i++) {
            if (fds[i].revents & POLLIN) {
                int bytes_received = SSL_read(ssl[i], message, sizeof(message) - 1);
                if (bytes_received <= 0) {
                    // Connection closed or error occurred
                    break;
                }
                char nbuf[1024];

                message[bytes_received] = '\0';

                if (i == 1 && strcmp(message, "challenge") == 0) { // check it it is the the server who sent the challenge
                    check_hash(); //to see if it is offline mode , if so store the hash in the file 
                    SSL_write(ssl[i], lastHash, strlen(lastHash)); // send the current hash to the server as response to the challenge
                    continue; // Proceed to next iteration in the loop
                }

                else if (strcmp(message, "quit") == 0) {
                    a = "1";
                    strcpy(secret, check(Mcode, a)); // concatenate Mcode and a, then hash them and store the value in secret
                    stopMotor();
                    printf("Quitting\n");
                    break;
                } else if (strcmp(message, "run") == 0) {
                    a = "2";
                    strcpy(secret, check(Mcode, a));
                    startMotor();
                    printf("Motor started \n");
                    bytes_received = SSL_read(ssl[i], message, sizeof(message) - 1);
                    message[bytes_received] = '\0';
                    strcpy(nbuf, message);
                    foo(nbuf); // to print the state of the motor , also used for the attack
                    printf("Received from server: %s\n", message);
                    if (strcmp(message, "high") == 0) {
                        a = "3";
                        strcpy(secret, check(secret, a));  
                        highSpeed();
                        printf("Motor set to high speed\n");
                    } else if (strcmp(message, "low") == 0) {
                        a = "4";
                        strcpy(secret, check(secret, a));  
                        lowSpeed();
                        printf("Motor set to low speed\n");
                    } else {
                        printf("invalid command");
                    }
                } else if (strcmp(message, "stop") == 0) {
                    a = "5";
                    strcpy(secret, check(Mcode, a)); 
                    stopMotor();
                    printf("Motor stopped\n");
                } else {
                    printf("Invalid command\n");
                }
            }
        }
    }
    // Cleanup and resource deallocation
    for (int i = 0; i < num_servers; i++) {
        SSL_shutdown(ssl[i]);
        SSL_free(ssl[i]);
        close(client_fd[i]);
    }
    SSL_CTX_free(ctx);
    EVP_cleanup();
    gpioTerminate();
    return 0;
}
