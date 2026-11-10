#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include "utilities.h"

RetStatusUtilities calculate_sha256(const char *file_path, char *sha256_hex)
{
    RetStatusUtilities Status = STATUS_OK_UTIL; 
    EVP_MD_CTX *ctx = NULL;
    char buffer[BUFFER_SIZE]; 
    unsigned char hash[SHA256_SIZE];
    unsigned int hash_len = 0;

    size_t bytes_read = 0;

    FILE *file = fopen(file_path, "rb");
    if(file)
    {
        // Create SHA256 context
        ctx = EVP_MD_CTX_new();
        if (ctx)
        {
            //Initialize SHA256
            if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) == 1)
            {
                // Read file by bloc and update hash 
                while ((bytes_read = fread(buffer, 1, sizeof(buffer), file) > 0))
                {
                    EVP_DigestUpdate(ctx, buffer, bytes_read);
                }

                // Finalize hash calculation
                if (EVP_DigestFinal_ex(ctx, hash, &hash_len))
                {
                    /* 6. Convert binary hash to hexadecimal */
                    for (int i = 0; i < SHA256_SIZE; i++) {
                        snprintf(&sha256_hex[i * 2], 3, "%02x", hash[i]);
                    }

                    sha256_hex[SHA256_HEX_SIZE - 1] = '\0';

                    Status = STATUS_OK_UTIL; 
                }
                else
                {
                    Status = STATUS_NOK_UTIL; 
                }


            }
            else
            {
                Status = STATUS_NOK_UTIL; 
            }

        }
        else
        {
            Status = STATUS_NOK_UTIL; 
        }
    }
    else
    {
        Status = STATUS_NOK_UTIL; 
    }

    return Status; 
    
}