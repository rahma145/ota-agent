#ifndef HAWKBIT_H
#define HAWKBIT_H

#include <curl/curl.h>
#include <cjson/cJSON.h>

#define MAX_FILENAME_SIZE 256
#define MAX_SHA_SIZE 65
#define MAX_URL_SIZE 512

#define CODE_RESPONSE_OK 200
typedef enum {
    STATUS_NOK = -1, 
    STATUS_OK = 0,
} RetStatus; 

typedef struct artifact_info {
    char filename[256];
    char sha256[65];      /* 64 caractères hexa + '\0' */
    char download_url[512];
    long size;
} artifact_info;


struct buf { char *data; size_t len; };

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata); 

RetStatus hawkbit_poll(const char *base_url, const char *tenant, const char *controller_id, const char *token, struct buf *out); 
RetStatus hawkbit_has_update(const char *json_text, char *href_out, size_t href_len); 
RetStatus hawkbit_get(const char *url, const char *token, struct buf *out);
RetStatus hawkbit_parse_deployment(char* response, struct artifact_info* info); 
RetStatus hawkbit_download(const char *url, const char *token, const char *dest_path);
RetStatus hawkbit_send_feedback(const char *url, const char *token, cJSON* json_body); 
RetStatus hawkbit_build_feedback_json(const char *execution, const char *finished, cJSON **object); 





#endif // HAWKBIT_H