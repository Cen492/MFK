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
#include <sys/time.h>

double total = 0.0;
double elapsed_time = 0.0;
clock_t start_time, end_time;
int y = 1;

char Mcode[256];
#define BUFFER_SIZE 1024
char message[BUFFER_SIZE];
char secret[SHA256_DIGEST_LENGTH * 2 + 1];
char lastHash[SHA256_DIGEST_LENGTH * 2 + 1];

// Motor control pins
const int IN1 = 24;
const int IN2 = 23;
const int ENA = 25;

void startMotor() {
    memcpy(Mcode, (void *)&startMotor, 144);
    strcpy(secret, check(secret, Mcode));

    gpioWrite(IN1, 1);
    gpioWrite(IN2, 0);
}

void stopMotor() {
    memcpy(Mcode, (void *)&stopMotor, 156);
    strcpy(secret, check(secret, Mcode));
    strcpy(lastHash, secret);

    gpioWrite(IN1, 0);
    gpioWrite(IN2, 0);
    gpioWrite(ENA, 0);
    printf("You're in the stop function\n");
}

void lowSpeed() {
    memcpy(Mcode, (void *)&lowSpeed, 132);
    strcpy(secret, check(secret, Mcode));
    strcpy(lastHash, secret);

    gpioPWM(ENA, 50);
}

void highSpeed() {
    memcpy(Mcode, (void *)&highSpeed, 132);
    strcpy(secret, check(secret, Mcode));
    strcpy(lastHash, secret);

    gpioPWM(ENA, 70);
}

int main() {
    // GPIO library initialization and motor control pin setup
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialization failed\n");
        return 1;
    }

    for (int i = 0; i < 100; i++) {
        strcpy(secret, check(Mcode, "1"));
        start_time = clock();
        gpioSetMode(IN1, PI_OUTPUT);
        gpioSetMode(IN2, PI_OUTPUT);
        gpioSetMode(ENA, PI_OUTPUT);

        strcpy(secret, "");
        char *a;
        strcpy(Mcode, "");
        memcpy(Mcode, (void *)&main, 256);

        strcpy(message, "run");

        if (strcmp(message, "quit") == 0) {
            a = "1";
            strcpy(secret, check(Mcode, a));
            stopMotor();
            printf("Quitting\n");
        } else if (strcmp(message, "run") == 0) {
            a = "2";
            strcpy(secret, check(Mcode, a));
            startMotor();
            printf("Motor started\n");
            if (y == 1) {
                strcpy(message, "high");
            } else {
                strcpy(message, "low");
            }

            if (strcmp(message, "high") == 0) {
                a = "3";
                y = 0;
                strcpy(secret, check(secret, a));
                highSpeed();
                printf("Motor set to high speed\n");
            } else if (strcmp(message, "low") == 0) {
                a = "4";
                y = 1;
                strcpy(secret, check(secret, a));
                lowSpeed();
                printf("Motor set to low speed\n");
            } else {
                printf("Invalid command\n");
            }
        } else if (strcmp(message, "stop") == 0) {
            a = "5";
            strcpy(secret, check(Mcode, a));
            stopMotor();
        } else {
            printf("Invalid command\n");
            return 0;
        }
        end_time = clock();
        elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000000;
        total += elapsed_time;
        sleep(2);
    }

    printf("-------------- else if stop: %.8f microseconds\n", total);
    return 0;
}
