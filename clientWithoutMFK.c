#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <poll.h> 
#include <pigpio.h>
#include <pthread.h>
#include <time.h>


#define BUFFER_SIZE 1024
char message[BUFFER_SIZE];
char secret[SHA256_DIGEST_LENGTH *2 +1];
char lastHash[SHA256_DIGEST_LENGTH *2 +1];

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

//attack------------------
void vuln(){
 printf("attack is happen --- \n");  
 } 
 
struct VulnStruct{
 char buf[64];
 void (*func_ptr)(char* arg);
 };
 void new(char *name){
 printf("state of the motor is: %s\n", name);  
 } 
  
 void foo(char *arg){
  struct VulnStruct local_struct;
  local_struct.func_ptr=new;
  strcpy(local_struct.buf,arg);
  (local_struct.func_ptr)(local_struct.buf);
  }

//attack------------------


void startMotor() {

 gpioWrite(IN1, 1);
 gpioWrite(IN2, 0);}
 
void stopMotor() {

 gpioWrite(IN1, 0);
 gpioWrite(IN2, 0);
 gpioWrite(ENA, 0);
  
 printf("you in stop function \n");
  
}
void lowSpeed() {

  gpioPWM(ENA, 50);}
  
void highSpeed() {
 
 gpioPWM(ENA, 70);}
  

int main() {
	clock_t t;
	
 // GPIO library initialization and motor control pin setup
 if (gpioInitialise() < 0) {
 fprintf(stderr, "pigpio initialisation failed\n");
 return 1;
 }
 strcpy(lastHash, "123");
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
 server_ips[0] = "172.20.10.8";
 server_ips[1] = "127.0.0.1";

 int server_ports[num_servers];
 server_ports[0] = 12346;
 server_ports[1] = 12345;

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
 for (int i = 0; i < num_servers; i++) {
 fds[i].fd = client_fd[i];
 fds[i].events = POLLIN;
 }


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
  strcpy(nbuf,message);
  foo(nbuf);
  message[bytes_received] = '\0';
   
  if (strcmp(message, "q") == 0) {
 
  stopMotor();
  printf("Quitting\n");

  break;
 } else if (strcmp(message, "r") == 0) {
  startMotor();
  printf("Motor started \n");

  bytes_received = SSL_read(ssl[i], message, sizeof(message) - 1);
  message[bytes_received] = '\0';
  printf("Received from server: %s\n", message);
   if (strcmp(message, "h") == 0) {

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
	 t=clock();
  stopMotor();
  t=clock()-t;
  double time_taken=((double) t)/CLOCKS_PER_SEC;
  printf("stop take %f seconed to execute\n", time_taken);
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
