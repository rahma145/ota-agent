#include <stdlib.h>
#include <string.h>
#include "hawkbit.h"

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t data_len = nmemb * size ;
    struct buf* buffer = userdata; 
    char *tmp = realloc(buffer->data, buffer->len + data_len +1); 
    if (!tmp) return 0; 
    memcpy(buffer->data + buffer->len, ptr, data_len); 
    buffer->len += data_len; 

    buffer->data[buffer->len] = '\0'; 
    
    return data_len ; 
}

RetStatus hawkbit_poll(const char *base_url, const char *tenant, const char *controller_id, const char *token, struct buf *out)
{
    int RetStatus = STATUS_NOK; 
    char url[512]; 
    char auth_header[300]; 
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *h = curl_easy_init(); 

    snprintf(url, sizeof(url), "%s/%s/controller/v1/%s", base_url, tenant, controller_id);
    snprintf(auth_header, sizeof(auth_header), "Authorization: TargetToken %s", token); 

    if (h)
    {
        printf("Creating the handle OK \n"); 
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, auth_header);

        printf("URL = %s\n", url);

        curl_easy_setopt(h, CURLOPT_URL, url);
        printf("here is the error \n ", url);
        curl_easy_setopt(h, CURLOPT_HTTPHEADER, headers);
        printf("here is the error \n ", url);
        curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write_cb);
        printf("here is the error \n ", url);
        curl_easy_setopt(h, CURLOPT_WRITEDATA, out);
        printf("here is the error \n ", url);

        CURLcode result; 
        result = curl_easy_perform(h); 
        sleep(3); 
        printf("here is the error \n ", url);

        if (result == CURLE_OK)
        {
            printf("Executing http request OK \n"); 
            long code = 0; 
            curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &code); 
            if (code == CODE_RESPONSE_OK)
            {
                printf("Getting info OK \n"); 
                RetStatus = STATUS_OK; 
            }
            else 
            {
                printf("Getting info NOK \n");
                RetStatus = STATUS_NOK; 
            }
        }
        else 
        {
            printf("Executing http request NOK and curl code = %d and url = %s \n", result,url); 
            RetStatus = STATUS_NOK; 
        }
        curl_slist_free_all(headers); 
    }
    else
    {
        printf("Creating the handle NOK \n"); 
        RetStatus = STATUS_NOK; 
    }

    curl_easy_cleanup(h); 
    curl_global_cleanup(); 
        
    return RetStatus; 
}