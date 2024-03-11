#include <pigpio.h>
#include <stdio.h>

// Define GPIO pins for motor control (adjust as needed)
const int IN1 = 24;
const int IN2 = 23;
const int ENA = 25;

// Function to start the motor forward
void startMotor() {
  gpioWrite(IN1, 1);
  gpioWrite(IN2, 0);
}

// Function to set the motor speed
void setSpeed(int speed) {
  gpioPWM(ENA, speed);
}

// Function to stop the motor
void stopMotor() {
  gpioWrite(IN1, 0);
  gpioWrite(IN2, 0);
}

int main() {
  if (gpioInitialise() < 0) {
    fprintf(stderr, "pigpio initialisation failed\n");
    return 1;
  }

  gpioSetMode(IN1, PI_OUTPUT);
  gpioSetMode(IN2, PI_OUTPUT);
  gpioSetMode(ENA, PI_OUTPUT);

  char choice;

  while (1) {
    printf("Press r for forward, h for high speed, l for low speed, e for stop and exit: ");
    scanf(" %c", &choice);

    if (choice == 'r') {
      startMotor();
      printf("Motor started forward\n");
    } else if (choice == 'h') {
      setSpeed(100);  // Adjust high speed value as needed
      printf("Motor set to high speed\n");
    } else if (choice == 'l') {
      setSpeed(50);   // Adjust low speed value as needed
      printf("Motor set to low speed\n");
    } else if (choice == 'e') {
      stopMotor();
      printf("Motor stopped\n");
      gpioTerminate();
      return 0;  // Exit the program
    } else {
      printf("Invalid choice\n");
    }
  }

  return 0;
}
