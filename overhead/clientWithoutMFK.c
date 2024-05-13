#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h> 
#include <pigpio.h>
#include <pthread.h>
#include <sys/time.h>

double total = 0.0;
double elapsed_time = 0.0;
clock_t start_time, end_time;
int y = 1;

#define BUFFER_SIZE 1024
char message[BUFFER_SIZE];

// Motor control pins
const int IN1 = 24;
const int IN2 = 23;
const int ENA = 25;

void startMotor() {
    gpioWrite(IN1, 1);
    gpioWrite(IN2, 0);
}

void stopMotor() {
    gpioWrite(IN1, 0);
    gpioWrite(IN2, 0);
    gpioWrite(ENA, 0);
    printf("You're in the stop function\n");
}

void lowSpeed() {
    gpioPWM(ENA, 50);
}

void highSpeed() {
    gpioPWM(ENA, 70);
}

int main() {
    // GPIO library initialization and motor control pin setup 
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialization failed\n");
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        start_time = clock(); 
        gpioSetMode(IN1, PI_OUTPUT);
        gpioSetMode(IN2, PI_OUTPUT);
        gpioSetMode(ENA, PI_OUTPUT);

        // while (1) {
        strcpy(message, "run");

        if (strcmp(message, "quit") == 0) {
            stopMotor();
            printf("Quitting\n");
        } else if (strcmp(message, "run") == 0) {
            startMotor();
            printf("Motor started\n");
            if (y == 1) {
                strcpy(message, "high");
            } else {
                strcpy(message, "low");
            }
            if (strcmp(message, "high") == 0) {
                y = 0;
                highSpeed();
                printf("Motor set to high speed\n");
            } else if (strcmp(message, "low") == 0) {
                y = 1;
                lowSpeed();
                printf("Motor set to low speed\n");
            } else {
                printf("Invalid command\n");
            }
        } else if (strcmp(message, "stop") == 0) {
            stopMotor();
            printf("Motor stopped\n");
        } else {
            return 0;
            printf("Invalid command\n");
        }
        end_time = clock();
        elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000000;
        total += elapsed_time;
        sleep(2);
    }
    printf("-------------- else if stop: %.8f microseconds\n", total);
    return 0;
}
