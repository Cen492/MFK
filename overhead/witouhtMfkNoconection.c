#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h> 
#include <pigpio.h>
#include <pthread.h>
#include <sys/time.h>

double time=0;
struct timeval start , end;
   long seconds;
 long microseconds;

#define BUFFER_SIZE 1024
char message[BUFFER_SIZE];
// Motor control pins
const int IN1 = 24;
const int IN2 = 23;
const int ENA = 25;


void startMotor() {
   gettimeofday(&start,0);

 gpioWrite(IN1, 1);
 gpioWrite(IN2, 0);
 gettimeofday(&end,0);
  seconds=end.tv_sec-start.tv_sec;
  microseconds=end.tv_usec-start.tv_usec;
 time=time+seconds*1e6 +microseconds;
 }
 
void stopMotor() {

 gpioWrite(IN1, 0);
 gpioWrite(IN2, 0);
 gpioWrite(ENA, 0);
  
 printf("you in stop function \n");
  
}
void lowSpeed() {

  gpioPWM(ENA, 50);}
  
void highSpeed() {
    gettimeofday(&start,0);

 gpioPWM(ENA, 70);
  gettimeofday(&end,0);
   seconds=end.tv_sec-start.tv_sec;
  microseconds=end.tv_usec-start.tv_usec;
 time=time+seconds*1e6 +microseconds;
 
 }
  

int main() {
	 

   gettimeofday(&start,0);
	
 // GPIO library initialization and motor control pin setup 
  if (gpioInitialise() < 0) {
 fprintf(stderr, "pigpio initialisation failed\n");
 return 1;
 }

 gpioSetMode(IN1, PI_OUTPUT);
 gpioSetMode(IN2, PI_OUTPUT);
 gpioSetMode(ENA, PI_OUTPUT);
  gettimeofday(&end,0);
  seconds=end.tv_sec-start.tv_sec;
  microseconds=end.tv_usec-start.tv_usec;
 time=time+seconds*1e6 +microseconds;
  
 //while (1) {
strcpy(message, "run");

  if (strcmp(message, "quit") == 0) {
 
  stopMotor();
  printf("Quitting\n");

  
 } else if (strcmp(message, "run") == 0) {

  startMotor();
  printf("Motor started \n");
   strcpy(message, "high");
  
   if (strcmp(message, "high") == 0) {
  
   highSpeed();
   printf("Motor set to high speed\n");}

  else if (strcmp(message, "l") == 0) {
 
   lowSpeed();
   printf("Motor set to low speed\n");
 }
  else {
   printf("invalid commaned");
   }
 }
  else if (strcmp(message, "s") == 0) {
	
  stopMotor();
  printf("Motor stopped\n");
 } else {
   return 0;
  printf("Invalid command\n");
 }
  
 
 printf("-------------- take: %.8f useconds\n", time);
 return 0;
}
