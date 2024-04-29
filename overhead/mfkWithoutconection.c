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

  double time=0;
   struct timeval start , end;
   long seconds;
 long microseconds;
 
 char Mcode[4096];
#define BUFFER_SIZE 1024
char message[BUFFER_SIZE];
char secret[SHA256_DIGEST_LENGTH *2 +1];
char lastHash[SHA256_DIGEST_LENGTH *2 +1];


// Motor control pins
const int IN1 = 24;
const int IN2 = 23;
const int ENA = 25;

void startMotor() {
	gettimeofday(&start,0);
 memcpy(Mcode, (void *)&startMotor, 144);
 strcpy(secret, check(secret, Mcode)); 
    gpioWrite(IN1, 1);
 gpioWrite(IN2, 0);
   gettimeofday(&end,0);
  seconds=end.tv_sec-start.tv_sec;
  microseconds=end.tv_usec-start.tv_usec;
 time=time+seconds*1e6 +microseconds; 
}
 
void stopMotor() {
 memcpy(Mcode,(void *)&stopMotor,156) ;
 strcpy(secret, check(secret, Mcode));  
 strcpy(lastHash, secret);
 
 gpioWrite(IN1, 0);
 gpioWrite(IN2, 0);
 gpioWrite(ENA, 0);
  
 printf("you in stop function \n");
  
}
void lowSpeed() {
  memcpy(Mcode,(void *)&stopMotor,132) ;
  strcpy(secret, check(secret, Mcode));  
  strcpy(lastHash, secret);
  gpioPWM(ENA, 50);}
  
void highSpeed() {
gettimeofday(&start,0);
 memcpy(Mcode,(void *)&stopMotor,132) ;
 strcpy(secret, check(secret, Mcode)); 
 strcpy(lastHash, secret);
   gettimeofday(&end,0);
    gpioPWM(ENA, 70);
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
 
 //while (1){

 strcpy(secret, "");
 char* a;
 strcpy(Mcode,"");
 memcpy(Mcode,(void *)&main,3060);
 gettimeofday(&end,0);
  seconds=end.tv_sec-start.tv_sec;
  microseconds=end.tv_usec-start.tv_usec;
 time=time+seconds*1e6 +microseconds;
   
   strcpy(message,"run");
  if (strcmp(message, "quit") == 0) {
  a="1";
  strcpy(secret, check(Mcode, a));
  stopMotor();
  printf("Quitting\n");

  
 } else if (strcmp(message, "run") == 0) {
	 gettimeofday(&start,0);
  a="2";
  strcpy(secret, check(Mcode, a));
  gettimeofday(&end,0);
  seconds=end.tv_sec-start.tv_sec;
  microseconds=end.tv_usec-start.tv_usec;
 time=time+seconds*1e6 +microseconds;
  startMotor();
   
  printf("Motor started \n");
    strcpy(message,"high");

   if (strcmp(message, "high") == 0) {
	   gettimeofday(&start,0);
   a="3"; 
   strcpy(secret, check(secret, a));
   gettimeofday(&end,0);
  seconds=end.tv_sec-start.tv_sec;
  microseconds=end.tv_usec-start.tv_usec;
 time=time+seconds*1e6 +microseconds;
  highSpeed();
  printf("Motor set to high speed\n");
   }

  else if (strcmp(message, "low") == 0) {
   a="4";
   strcpy(secret, check(secret, a));
   lowSpeed();
   printf("Motor set to low speed\n");
 }
  else {
   printf("invalid commaned");
   }
 }
  else if (strcmp(message, "stop") == 0) {
  
  a="5";
  strcpy(secret, check(Mcode, a));
  stopMotor();
  
  printf("Motor stopped\n");
 } else {
   return 0;
  printf("Invalid command\n");
 }
  
 
 
 printf("-------------- take: %.8f seconds\n", time);

 return 0;
}
