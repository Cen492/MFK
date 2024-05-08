#include <err.h>
#include <stdio.h>
#include <string.h>
#include <pigpio.h>
#include <tee_client_api.h>
#include <aes_ta.h>
#include <time.h>
#include<unistd.h>
#include <sys/time.h>          
#define MAX_INPUT_LENGTH 256
#define HASH_SIZE 32 // Size of SHA-256 hash in bytes
#define MAX_HASHES 32

// TEE resources
struct test_ctx {
    TEEC_Context ctx;
    TEEC_Session sess;
};

struct test_ctx ctx;
unsigned char hash[HASH_SIZE];
char Mcode[256];

unsigned char hash[HASH_SIZE];
unsigned char hash_array[MAX_HASHES][HASH_SIZE];
size_t num_hashes = 0;
int Flag = 1;
int found=0;

 double wqt=0;
   struct timeval start , end;
   long seconds;
 long microseconds;


const int IN1 = 24;
const int IN2 = 23;
const int ENA = 25;

// Function to start the motor forward
void startMotor() {
  gpioWrite(IN1, 1);
  gpioWrite(IN2, 0);
  memcpy(Mcode,(void *)&startMotor,144) ;
  concatenate_and_hash(&ctx, hash, Mcode);

}

// Function to set the motor speed
void High() {
  gpioPWM(ENA, 100);
  memcpy(Mcode,(void *)&High,132) ;
  concatenate_and_hash(&ctx, hash, Mcode);
}

void Low() {
  gpioPWM(ENA, 50);
  
  memcpy(Mcode,(void *)&Low,132) ;
  concatenate_and_hash(&ctx, hash, Mcode);
  
}

// Function to stop the motor
void stopMotor() {
  gpioWrite(IN1, 0);
  gpioWrite(IN2, 0);
  gpioWrite(ENA, 0);
   
  memcpy(Mcode,(void *)&stopMotor,156) ;
  concatenate_and_hash(&ctx, hash, Mcode);
   
  
}

void prepare_tee_session(struct test_ctx *ctx) {
    TEEC_UUID uuid = TA_AES_UUID;
    uint32_t origin;
    TEEC_Result res;
  printf("enterd TS1\n");
    res = TEEC_InitializeContext(NULL, &ctx->ctx);
    if (res != TEEC_SUCCESS) {
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
    }

    res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid,
                           TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    if (res != TEEC_SUCCESS) {
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x", res, origin);
    }
}

void terminate_tee_session(struct test_ctx *ctx) {
    TEEC_CloseSession(&ctx->sess);
    TEEC_FinalizeContext(&ctx->ctx);
      printf("enterd TS2\n");
}

void concatenate_and_hash(struct test_ctx *ctx, const char *input1, const char *input2) {
    TEEC_Operation op;
    uint32_t origin;
    TEEC_Result res;

    // Prepare the operation for the Trusted Application (TA)
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_MEMREF_TEMP_INPUT,
                                     TEEC_MEMREF_TEMP_OUTPUT,
                                     TEEC_NONE);

    // Set the input parameters
    op.params[0].tmpref.buffer = (void*)input1;
    op.params[0].tmpref.size = strlen(input1);
    op.params[1].tmpref.buffer = (void*)input2;
    op.params[1].tmpref.size = strlen(input2);

    // Initialize output buffer for hash
   op.params[2].tmpref.buffer = hash;
    op.params[2].tmpref.size = HASH_SIZE;
    
    
    // Invoke the command to concatenate and hash the inputs
    res = TEEC_InvokeCommand(&ctx->sess, TA_AES_CMD_HASH, &op, &origin);
    if (res != TEEC_SUCCESS) {
        errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, origin);
    }

    // Print the hashed result from the TA in binary format
    printf("Concatenated and hashed result in binary:\n");
    for (size_t i = 0; i < HASH_SIZE; i++) {
        printf("%02X ", hash[i]); // Print each byte as a hexadecimal value
    }
    printf("\n");


    if (Flag == 1) {
        // Check if the hash is already in the array
            memcpy(hash_array[num_hashes], hash, HASH_SIZE);
            num_hashes++;
	printf("saved hash.\n");
         
        }
        if (Flag == 0) {
        // Check if the hash is already in the array
        for (size_t i = 0; i < num_hashes; i++) {
            if (memcmp(hash, hash_array[i], HASH_SIZE) == 0) {
                 found=1;
            }
        }
        if (found == 1) {
        	printf("safe.\n");
}
	if(found == 0) {
	printf("hacked.\n");

    }
        

    
}
}


int main(void) {
 for(int i=0;i<11;i++) {
 wqt=0;
  gettimeofday(&start,0);
    

    //printf("Preparing TEE session...\n");
    //prepare_tee_session(&ctx);


if (gpioInitialise() < 0) {
    fprintf(stderr, "pigpio initialisation failed\n");
    return 1;
  }

  gpioSetMode(IN1, PI_OUTPUT);
  gpioSetMode(IN2, PI_OUTPUT);
  gpioSetMode(ENA, PI_OUTPUT);





  char choice="s";

  //while (1) {
    printf("Press r for run, s for stop, e for exit: ");
    
    strcpy(hash, "G");
    memcpy(Mcode,(void *)&main,256) ;
    concatenate_and_hash(&ctx, hash, Mcode);
    if (choice == 'r') {
      startMotor();
      
      printf("Motor started forward\n");
      printf("Press h for high speed, l for low speed: ");
      scanf(" %c", &choice);
    	 if (choice == 'h') {
      High();  // Adjust high speed value as needed
      printf("Motor set to high speed\n");
    } 	else if (choice == 'l') {
      Low();   // Adjust low speed value as needed
      printf("Motor set to low speed\n");
      }
    } else if (choice == 's') {
      stopMotor();
      printf("Motor stopped\n");
      Flag=0; // Exit the program
    } 
    else if (choice == 'e') {
       printf("Terminating TEE session...\n");
    terminate_tee_session(&ctx);
    return 0;
    }
    
    else {
      printf("Invalid choice\n");
    }
    gettimeofday(&end,0);
  seconds=end.tv_sec-start.tv_sec;
  microseconds=end.tv_usec-start.tv_usec;
 wqt=wqt+seconds +microseconds*1e-6;
  printf("-------------- take: %.8f seconds\n", wqt);
  //}
  }
   
}

