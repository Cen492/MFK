#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#define PORT 12346
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
                usleep(5000000);
                SSL_write(ssl, "challenge", strlen("challenge"));

                // Check if it's time to quit

             bytes_received =   SSL_read(ssl,buffer,sizeof(buffer)-1);
             buffer[bytes_received]= '\0';
             printf("recived from client: %s\n",buffer);

                if(strcmp(buffer, "8a61363133363331333333363333333133333333333333363333333333333331") == 0){
                    printf("Safe\n");

                }

                else if(strcmp(buffer, "d333333333333333333333333333333333333333333333333333333333333333") == 0){
                    printf("Safe\n");}

               
                else if(strcmp(buffer, "3a61363133363331333333363333333133333333333333363333333333333331") == 0){
                        printf("Safe\n"); }
                    

                else if(strcmp(buffer, "a434333433333334333333333333333433333333333333333333333333333334") == 0){
                        printf("Safe\n");}


                else if(strcmp(buffer, "0535333533333335333333333333333533333333333333333333333333333335") == 0){
                    printf("Safe\n"); }
                else{
                    printf("Hacked\n");
                    
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
