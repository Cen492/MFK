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

char Mcode[4096];
#define BUFFER_SIZE 1024
char message[BUFFER_SIZE];
char secret[SHA256_DIGEST_LENGTH *2 +1];

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
void startMotor(SSL *ssl, char secret[], int server_number) {
   if (server_number == 1) { // Send hash only if it's server 2
    memcpy(Mcode, (void *)&startMotor, 144);
    strcpy(secret, check(secret, Mcode));  
    SSL_write(ssl, secret, strlen(secret)); 
  } 
  else{
  gpioWrite(IN1, 1);
  gpioWrite(IN2, 0);}}

  

void stopMotor(SSL *ssl, char secret[], int server_number) {
   if (server_number == 1) { // Send hash only if it's server 2
    memcpy(Mcode,(void *)&stopMotor,156) ;
    strcpy(secret, check(secret, Mcode));  
    SSL_write(ssl, secret, strlen(secret)); 
  } 
  else{
  gpioWrite(IN1, 0);
  gpioWrite(IN2, 0);
  gpioWrite(ENA, 0);}
  
}

void lowSpeed(SSL *ssl, char secret[], int server_number) {
    if (server_number == 1) { // Send hash only if it's server 2
    memcpy(Mcode,(void *)&stopMotor,132) ;
    strcpy(secret, check(secret, Mcode));  
    SSL_write(ssl, secret, strlen(secret)); 
  } 
  else{
  gpioPWM(ENA, 50);}
   
}
void highSpeed(SSL *ssl, char secret[], int server_number) {
    if (server_number == 1) { // Send hash only if it's server 2
    memcpy(Mcode,(void *)&stopMotor,132) ;
    strcpy(secret, check(secret, Mcode));  
    SSL_write(ssl, secret, strlen(secret)); 
  } 
  else{
  gpioPWM(ENA, 70);}
  
}

int main() {
  char buffer[64];

   // GPIO library initialization and motor control pin setup
   if (gpioInitialise() < 0) {
    fprintf(stderr, "pigpio initialisation failed\n");
    return 1;
  }
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
    strcpy(secret, "9");
    char* a;
    strcpy(Mcode,"");
    for (int i = 0; i < num_servers; i++) {
    fds[i].fd = client_fd[i];
    fds[i].events = POLLIN;
  }
    memcpy(Mcode,(void *)&main,3060);

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
        message[bytes_received] = '\0';
        strcpy(buffer,message); //for attack 
        printf("Received from server %d: %s\n", i + 1, message);
         
      if (strcmp(message, "q") == 0) {
        a="1";
        strcpy(secret, check(Mcode, a));
        stopMotor(ssl[i],secret,i + 1);
        printf("Quitting\n");

      break;
    } else if (strcmp(message, "r") == 0) {
       a="2";
      strcpy(secret, check(Mcode, a));
      startMotor(ssl[i],secret,i + 1);
      printf("Motor started \n");

      bytes_received = SSL_read(ssl[i], message, sizeof(message) - 1);
      message[bytes_received] = '\0';
      strcpy(buffer,message);//for attack 
      printf("Received from server: %s\n", message);
      //printf("Received from server buffer : %s\n", buffer);
         if (strcmp(buffer, "h") == 0) {
           a="3"; 
          strcpy(secret, check(secret, a));
          highSpeed(ssl[i],secret,i + 1);
          printf("Motor set to high speed\n");}

        else if (strcmp(buffer, "l") == 0) {
           a="4";
          strcpy(secret, check(secret, a));
          lowSpeed(ssl[i],secret,i + 1);
          printf("Motor set to low speed\n");
    }
        else {
          printf("invalid commaned");
          }
    }
     else if (strcmp(message, "s") == 0) {
       a="5";
      strcpy(secret, check(Mcode, a));
      stopMotor(ssl[i],secret,i + 1);
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
