#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "hawkbit.h"

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t data_len = nmemb * size ;
    struct buf* buffer = userdata; 
    char *tmp = realloc(buffer->data, buffer->len + data_len +1); 
    if (!tmp) return 0; 
    buffer->data = tmp; 
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

        curl_easy_setopt(h, CURLOPT_URL, url);
        curl_easy_setopt(h, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(h, CURLOPT_WRITEDATA, out);

        CURLcode result; 
        result = curl_easy_perform(h); 

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

RetStatus hawkbit_has_update(const char *json_text, char *href_out, size_t href_len)
{
    printf("executing hawkbit_has_update \n"); 
    RetStatus Status = STATUS_OK; 
    cJSON * root = cJSON_Parse(json_text);
    if(root)
    {
        printf("root found \n"); 
        cJSON *links = cJSON_GetObjectItemCaseSensitive(root, "_links"); 
        if(cJSON_IsObject(links))
        {
            printf("links found \n"); 
            cJSON *deploymentBase = cJSON_GetObjectItemCaseSensitive(links, "deploymentBase");
            if(cJSON_IsObject(deploymentBase))
            {
                printf ("deploymentBase found \n"); 
                cJSON *href = cJSON_GetObjectItemCaseSensitive(deploymentBase, "href");
                printf("href value : %s \n", href->valuestring); 
                if (href_out == NULL)
                {
                    printf("href_out is NULL \n"); 
                }
                else 
                {
                    printf("href is not null \n"); 
                }
                snprintf(href_out, href_len, "%s", href->valuestring); 
                // memcpy(href_out, href_len, href->valuestring); 
                printf("href_out = %s \n", href_out); 
                // if (cJSON_IsString(href) && (href->valuestring))
                // {
                //     printf("href is string and its value is %s", href->valuestring); 
                //     Status = STATUS_OK; 
                //     memcpy(href_out, href_len, href->valuestring); 
                // }
                // else
                // {
                //     printf("href is not string \n"); 
                //     cJSON_Delete(href); 
                //     Status = STATUS_NOK; 
                // }
            }
            else
            {
                printf("object deploymentBase not found \n"); 
                Status = STATUS_NOK; 
                cJSON_Delete(deploymentBase); 
            }
        }
        else
        {
            printf("object links not found \n"); 
            Status = STATUS_NOK; 
            cJSON_Delete(links); 
        }
    }
    else
    {
        printf("root is NULL \n"); 
        Status = STATUS_NOK; 
    }
    cJSON_Delete(root);
    return Status; 
    
}