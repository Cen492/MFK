#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pigpio.h>
#include "mfk.h"
#include <stdint.h>


#define PORT 12345
#define BUFFER_SIZE 1024

const int IN1 = 24;
const int IN2 = 23;
const int ENA = 25;
char secret[SHA256_DIGEST_LENGTH * 2 + 1];
SSL *ssl; // Global SSL object
uintptr_t G;
void init_openssl() {
    SSL_library_init();
    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();
}

SSL_CTX* create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_client_method(); // Use TLS_client_method() as SSLv23_client_method() is deprecated
    ctx = SSL_CTX_new(method);

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void cleanup_openssl() {
    EVP_cleanup();
}

void handleErrorWithSSL(const char* message) {
    perror(message);
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
}

void startMotor() {
    gpioWrite(IN1, 1);
    gpioWrite(IN2, 0);
    G = (uintptr_t) startMotor;
    strcpy(secret, check(secret, G)); // Assuming 'check' modifies 'secret' safely
    if (SSL_write(ssl, secret, strlen(secret)) <= 0) {
        handleErrorWithSSL("SSL_write failed in startMotor");
    }
}

void lowSpeed() {
    gpioPWM(ENA, 50);
    G = (uintptr_t) lowSpeed;
    strcpy(secret, check(secret, G));
    if (SSL_write(ssl, secret, strlen(secret)) <= 0) {
        handleErrorWithSSL("SSL_write failed in setSpeed");
    }
}

void highSpeed() {
    gpioPWM(ENA, 70);
    G = (uintptr_t) highSpeed;
    strcpy(secret, check(secret, G));
    if (SSL_write(ssl, secret, strlen(secret)) <= 0) {
        handleErrorWithSSL("SSL_write failed in setSpeed");
    }
}

void stopMotor() {
    gpioWrite(IN1, 0);
    gpioWrite(IN2, 0);
    gpioWrite(ENA, 0);
    G = (uintptr_t) stopMotor;
    strcpy(secret, check(secret, G));
    if (SSL_write(ssl, secret, strlen(secret)) <= 0) {
        handleErrorWithSSL("SSL_write failed in stopMotor");
    }
}

int main() {
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    init_openssl();
    SSL_CTX *ctx = create_context();

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        handleErrorWithSSL("Socket creation failed");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "172.20.10.8", &server_addr.sin_addr) <= 0) {
        handleErrorWithSSL("Invalid address/ Address not supported");
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        handleErrorWithSSL("Connection failed");
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    if (SSL_connect(ssl) != 1) {
        handleErrorWithSSL("SSL_connect failed");
    }

    if (gpioInitialise() < 0) {
        handleErrorWithSSL("pigpio initialisation failed");
    }
    gpioSetMode(IN1, PI_OUTPUT);
    gpioSetMode(IN2, PI_OUTPUT);
    gpioSetMode(ENA, PI_OUTPUT);

    while (1) {
       G = (uintptr_t) main;
        strcpy(secret,"9" );
         //char buffer[BUFFER_SIZE];
       // printf("Enter command ('r', 'h', 'l', 's', or 'q'): ");
        //fgets(buffer, BUFFER_SIZE, stdin);
       // buffer[strcspn(buffer, "\n")] = '\0';
        bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        buffer[bytes_received] = '\0';
            printf("Received from server: %s\n", buffer);

        if (strcmp(buffer, "q") == 0) {

                strcpy(secret, check(secret, G));
            stopMotor();
            printf("Quitting\n");

            break;
        } else if (strcmp(buffer, "r") == 0) {

                strcpy(secret, check(secret, G));
            startMotor();
            printf("Motor started \n");

            bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);
			buffer[bytes_received] = '\0';
            printf("Received from server: %s\n", buffer);

				 if (strcmp(buffer, "h") == 0) {

					strcpy(secret, check(secret, G));
					highSpeed();
					printf("Motor set to high speed\n");}

				else if (strcmp(buffer, "l") == 0) {

					strcpy(secret, check(secret, G));
					lowSpeed();
					printf("Motor set to low speed\n");
        }
				else {
					printf("invalid commaned");
					}




        }
         else if (strcmp(buffer, "s") == 0) {

                strcpy(secret, check(secret, G));
            stopMotor();
            printf("Motor stopped\n");
        } else {
            printf("Invalid command\n");
        }
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    gpioTerminate();

    return 0;
}
