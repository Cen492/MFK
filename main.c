#include <err.h>
#include <stdio.h>
#include <string.h>
#include <pigpio.h>
/* OP-TEE TEE client API  */
#include <tee_client_api.h>
#include <aes_ta.h>

#define MAX_INPUT_LENGTH 256
#define HASH_SIZE 32 // Size of SHA-256 hash in bytes

// TEE resources
struct test_ctx {
    TEEC_Context ctx;
    TEEC_Session sess;
};
struct test_ctx ctx;

unsigned char hash[HASH_SIZE]; 
char Mcode[256]; //to store the machine code

// Motor control pins
const int IN1 = 24;
const int IN2 = 23;
const int ENA = 25;

// Motor function to start the motor forward
void startMotor() {
  gpioWrite(IN1, 1); 
  gpioWrite(IN2, 0);
  memcpy(Mcode,(void *)&startMotor,144) ; //store startMotor Machine code in Mcode
  concatenate_and_hash(&ctx, hash, Mcode); //concatenate Mcode and hash, then hash them and store the value in hash
}



// Motor function to set the motor speed to High
void High() {
  gpioPWM(ENA, 100);
  memcpy(Mcode,(void *)&High,132) ;
  concatenate_and_hash(&ctx, hash, Mcode);
}

// Motor function to set the motor speed to Low
void Low() {
  gpioPWM(ENA, 50);
  memcpy(Mcode,(void *)&Low,132) ;
  concatenate_and_hash(&ctx, hash, Mcode);
}

// Motor function to set the motor speed
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
    /* Initialize a context connecting us to the TEE */
    res = TEEC_InitializeContext(NULL, &ctx->ctx);
    if (res != TEEC_SUCCESS) {
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
    }
	/* Open a session with the TA */
    res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid,
         TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    if (res != TEEC_SUCCESS) {
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x", res, origin);
    }
}

void terminate_tee_session(struct test_ctx *ctx) {
    TEEC_CloseSession(&ctx->sess);
    TEEC_FinalizeContext(&ctx->ctx);
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
}

int main(void) {
    printf("Preparing TEE session...\n");
    prepare_tee_session(&ctx);

//GPIO library initialization and motor control pin setup
if (gpioInitialise() < 0) {
    fprintf(stderr, "pigpio initialisation failed\n");
    return 1;
  }

  gpioSetMode(IN1, PI_OUTPUT);
  gpioSetMode(IN2, PI_OUTPUT);
  gpioSetMode(ENA, PI_OUTPUT);
	
  char choice;

  while (1) {
    printf("Press r for run, e for stop and exit: ");
    scanf(" %c", &choice);
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
    }  else if (choice == 'e') {
           stopMotor();
           printf("Motor stopped\n");
           gpioTerminate();
           return 0;  // Exit the program
    } else {
          printf("Invalid choice\n");
    }
  }
    
}


