// mfk.c
#include "mfk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

char* sha256(const char* data) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  EVP_MD_CTX *mdctx;
  const EVP_MD *md = EVP_sha256();

  mdctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(mdctx, md, NULL);
  EVP_DigestUpdate(mdctx, data, strlen(data));
  EVP_DigestFinal_ex(mdctx, hash, NULL);
  EVP_MD_CTX_free(mdctx);

  // Allocate memory for the hash in hexadecimal format
  char* hash_str = malloc(SHA256_DIGEST_LENGTH * 2 + 1);
  if (hash_str == NULL) {
    return NULL; // Handle memory allocation failure
  }

  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    sprintf(&hash_str[i * 2], "%02x", (unsigned int)hash[i]);
  }

  return hash_str;
}

char* check(const char* arg1, const char* arg2) {
  // Concatenate the strings
  size_t arg1_len = strlen(arg1);
  size_t arg2_len = strlen(arg2);
  size_t combined_len = arg1_len + arg2_len;
  char* combined = malloc(combined_len + 1);
  if (combined == NULL) {
    return NULL; // Handle memory allocation failure
  }
  memcpy(combined, arg1, arg1_len);
  memcpy(combined + arg1_len, arg2, arg2_len);
  combined[combined_len] = '\0';

  // Calculate hash of the combined string
  char* hashResult = sha256(combined);

  // Free the memory allocated for combined string
  free(combined);

  return hashResult;
}
