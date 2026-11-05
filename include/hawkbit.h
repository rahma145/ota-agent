#ifndef HAWKBIT_H
#define HAWKBIT_H

#include <curl/curl.h>

#define CODE_RESPONSE_OK 200
typedef enum {
    STATUS_NOK = -1, 
    STATUS_OK = 0,
} RetStatus; 


struct buf { char *data; size_t len; };

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata); 

RetStatus hawkbit_poll(const char *base_url, const char *tenant, const char *controller_id, const char *token, struct buf *out); 





#endif // HAWKBIT_H