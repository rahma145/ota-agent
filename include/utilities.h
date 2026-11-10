#ifndef UTILITIES_H
#define UTILITIES_H

typedef enum {
    STATUS_NOK_UTIL = -1, 
    STATUS_OK_UTIL = 0,
} RetStatusUtilities; 


#define SHA256_SIZE 32
#define SHA256_HEX_SIZE 65
#define BUFFER_SIZE 4096

RetStatusUtilities calculate_sha256(const char *file_path, char *sha256_hex); 




#endif //UTILITIES_H