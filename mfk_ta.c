#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include "mbedtls/platform.h"
#include "mbedtls/sha256.h"
#include <aes_ta.h>
#include <string.h>


#ifndef SHA256_DIGEST_LENGTH
#define SHA256_DIGEST_LENGTH 32
#endif

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
    return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
                                    TEE_Param __unused params[4],
                                    void __unused **sess_ctx)
{
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE,
                                               TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __unused *sess_ctx)
{
}

// Function to calculate the hash (MFK)
static TEE_Result concatenate_and_hash(uint32_t param_types, TEE_Param params[4]) {
    uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_INPUT,
                                               TEE_PARAM_TYPE_MEMREF_OUTPUT,
                                               TEE_PARAM_TYPE_NONE);

    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    const size_t total_size = params[0].memref.size + params[1].memref.size;
    unsigned char *concatenated = TEE_Malloc(total_size, TEE_MALLOC_FILL_ZERO); 
    if (!concatenated) 
        return TEE_ERROR_OUT_OF_MEMORY;
        
	//concatenates Two inputs
    memcpy(concatenated, params[0].memref.buffer, params[0].memref.size);
    memcpy(concatenated + params[0].memref.size, params[1].memref.buffer, params[1].memref.size);
	
    // Hash calculation
    unsigned char hash[SHA256_DIGEST_LENGTH];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0); 
    mbedtls_sha256_update(&ctx, concatenated, total_size);
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);

    TEE_Free(concatenated);
    
    //saves the hash in buffer to access it from the normal world
    memcpy(params[2].memref.buffer, hash, SHA256_DIGEST_LENGTH);
    params[2].memref.size = SHA256_DIGEST_LENGTH;

    return TEE_SUCCESS;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __unused *sess_ctx,
                                      uint32_t cmd_id,
                                      uint32_t param_types,
                                      TEE_Param params[4])
{
    (void)&sess_ctx;

    switch (cmd_id) {
        case TA_AES_CMD_HASH:
            return concatenate_and_hash(param_types, params);
        default:
            return TEE_ERROR_BAD_PARAMETERS;
    }
}

