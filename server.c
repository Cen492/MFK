#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 12345
#define BUFFER_SIZE 1024

void init_openssl() {
    SSL_library_init();
    SSL_load_error_strings();
}

SSL_CTX* create_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();
    ctx = SSL_CTX_new(method);

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    SSL_CTX_set_ecdh_auto(ctx, 1);

    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int bytes_received;
    char buffer[BUFFER_SIZE];

    init_openssl();
    SSL_CTX *ctx = create_context();
    configure_context(ctx);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_fd);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        } else {
            printf("SSL connection established!\n");

            char command[BUFFER_SIZE];
            while (1) {
                printf("Enter command ('r' , 's', or 'q'): ");
                fgets(command, BUFFER_SIZE, stdin);
                command[strcspn(command, "\n")] = '\0'; // Remove newline character

                // Send command to client
                SSL_write(ssl, command, strlen(command));

                // Check if it's time to quit
                if (strcmp(command, "quit") == 0) {
                    break;

                }

            bytes_received =   SSL_read(ssl,buffer,sizeof(buffer)-1);
            buffer[bytes_received]= '\0';
            printf("recived from client: %s\n",buffer);

            if(strcmp(command, "s") == 0){

                if(strcmp(buffer, "1c63363333363333333333363333333333333333333333363333333333333333") == 0){
                    printf("Safe\n");

                }
                else{
                    printf("Hacked\n");
                    break;
                                    }
            }

            else if(strcmp(command, "r") == 0){

                if(strcmp(buffer, "1737333733333337333333333333333733333333333333333333333333333337") == 0){
                    printf("Safe\n");

                     

                     printf("Enter speed ('h', 'l') ");
                     fgets(command, BUFFER_SIZE, stdin);
                     command[strcspn(command, "\n")] = '\0'; // Remove newline character
                     SSL_write(ssl, command, strlen(command));
                     bytes_received =   SSL_read(ssl,buffer,sizeof(buffer)-1);
                     buffer[bytes_received]= '\0';
                     printf("recived from client: %s\n",buffer);

                    if(strcmp(command, "h") == 0){
                        if(strcmp(buffer, "8030333033333330333333333333333033333333333333333333333333333330") == 0){
                        printf("Safe\n"); }
                    else{
                        printf("Hacked\n");
                        printf(buffer);
                        break;
                                    }
            }
                    else if(strcmp(command, "l") == 0){

                        if(strcmp(buffer, "5737333733333337333333333333333733333333333333333333333333333337") == 0){
                        printf("Safe\n");

                }
                        else{
                             printf("Hacked\n");
                             printf(buffer);
                              break;
                                    }

                }}
                else{
                    printf("Hacked\n");
                    break;
                                    }
            }

            
            
            else if(strcmp(command, "q") == 0){

                if(strcmp(buffer, "1c63363333363333333333363333333333333333333333363333333333333333") == 0){
                    printf("Safe\n");
                    printf("Quitting\n");
                    return 0;

                }
                else{
                    printf("Hacked\n");
                    break;
                                    }
            }

            
            }

            SSL_shutdown(ssl);
            SSL_free(ssl);
        }

        close(client_fd);
    }

    close(server_fd);
    SSL_CTX_free(ctx);
    EVP_cleanup();
    return 0;
}
