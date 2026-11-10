#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "hawkbit.h"

#define MAX_STR_LEN 256

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

static size_t write_file_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    FILE *f = userdata; 
    return fwrite(ptr, size, nmemb, f);
}

RetStatus hawkbit_poll(const char *base_url, const char *tenant, const char *controller_id, const char *token, struct buf *out)
{
    char url[512]; 
    RetStatus Status = STATUS_OK; 

    snprintf(url, sizeof(url), "%s/%s/controller/v1/%s", base_url, tenant, controller_id);
    Status = hawkbit_get(url, token, out); 
    
    return Status; 
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
                
                if (cJSON_IsString(href) && (href->valuestring))
                {
                    if((href_out != NULL) && (href_len != 0))
                    {
                        snprintf(href_out, href_len, "%s", href->valuestring);
                        Status = STATUS_OK; 
                    }
                    else
                    {
                        printf("wrong arguments \n"); 
                        Status = STATUS_NOK; 
                    }
                }
                else
                {
                    printf("href is not string \n"); 
                    Status = STATUS_NOK; 
                }
            }
            else
            {
                printf("object deploymentBase not found \n"); 
                Status = STATUS_NOK; 
            }
        }
        else
        {
            printf("object links not found \n"); 
            Status = STATUS_NOK; 
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

RetStatus hawkbit_get(const char *url, const char *token, struct buf *out)
{

    RetStatus RetStatus = STATUS_NOK; 
    char auth_header[300]; 
    curl_global_init(CURL_GLOBAL_ALL);
    CURL *h = curl_easy_init(); 

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
RetStatus hawkbit_parse_deployment(char* response, struct artifact_info* info)
{
    RetStatus Status = STATUS_OK; 
    cJSON * root = cJSON_Parse(response);
    if(root)
    {
        cJSON *deployment = cJSON_GetObjectItemCaseSensitive(root, "deployment"); 
        if (deployment)
        {
            printf("deployment object found \n"); 
            cJSON *chunks = cJSON_GetObjectItemCaseSensitive(deployment, "chunks"); 
            if (cJSON_IsArray(chunks))
            {
                printf("chunks found \n"); 
                cJSON *chunk = cJSON_GetArrayItem(chunks, 0);
                cJSON *artifacts_array = cJSON_GetObjectItemCaseSensitive(chunk, "artifacts");
                if (cJSON_IsArray(artifacts_array))
                {
                    printf("artifacts is an array \n"); 
                    cJSON *artifact = cJSON_GetArrayItem(artifacts_array, 0); 
                    if (cJSON_IsObject(artifact))
                    {
                        printf("artifact is an object \n"); 
                        // Filename
                        cJSON *filename = cJSON_GetObjectItemCaseSensitive(artifact, "filename");
                        if(cJSON_IsString(filename))
                        {
                            printf("filename is a string \n"); 
                            if (info->filename == NULL)
                            {
                                printf("filename is NULL \n"); 
                            }
                            strncpy(info->filename, filename->valuestring, MAX_FILENAME_SIZE - 1); 
                        }

                        // sha256
                        cJSON *hashes = cJSON_GetObjectItemCaseSensitive(artifact, "hashes");
                        if (cJSON_IsObject(hashes))
                        {
                            printf("hashes is a string \n");
                            cJSON *sha256 = cJSON_GetObjectItemCaseSensitive(hashes, "sha256");
                            if (cJSON_IsString(sha256))
                            {
                                printf("sha256 is a string \n"); 
                                strncpy(info->sha256, sha256->valuestring, MAX_SHA_SIZE - 1); 
                            }
                        }

                        // Size
                        cJSON *size = cJSON_GetObjectItemCaseSensitive(artifact, "size");
                        if (cJSON_IsNumber(size))
                        {
                            printf("size is a number \n"); 
                            info->size = size->valuedouble; 
                        }

                        // URL
                        cJSON *links = cJSON_GetObjectItemCaseSensitive(artifact, "_links");
                        if (cJSON_IsObject(links))
                        {
                            printf("links is an object \n"); 
                            cJSON *download_http = cJSON_GetObjectItemCaseSensitive(links, "download-http");
                            if (cJSON_IsObject(download_http))
                            {
                                printf("download_http is an object \n"); 
                                cJSON *href = cJSON_GetObjectItemCaseSensitive(download_http, "href");
                                if (cJSON_IsString(href))
                                {
                                    printf("href is a string \n"); 
                                    strncpy(info->download_url, href->valuestring, MAX_URL_SIZE - 1);
                                }
                                 
                            }
                        }

                        Status = STATUS_OK; 
                    }
                    else
                    {
                        printf("artifact is not an object \n"); 
                        Status = STATUS_NOK; 
                    }
                }
                else
                {
                    printf("artifacts is not an array \n"); 
                    Status = STATUS_NOK; 
                }
            }
            else
            {
                printf("chunks not found \n"); 
                Status = STATUS_NOK; 
            }
        }
        else
        {
            Status = STATUS_NOK; 
        }
    }
    else
    {
        Status = STATUS_NOK; 
    }
    cJSON_Delete(root); 

    return Status; 

}

RetStatus hawkbit_download(const char *url, const char *token, const char *dest_path)
{
    RetStatus Status = STATUS_OK; 

    FILE *f = fopen(dest_path, "wb");
    if (f) 
    {
        char auth_header[300]; 
        curl_global_init(CURL_GLOBAL_ALL);
        CURL *h = curl_easy_init(); 

        snprintf(auth_header, sizeof(auth_header), "Authorization: TargetToken %s", token); 

        if (h)
        {
            printf("Creating the handle OK \n"); 
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, auth_header);

            curl_easy_setopt(h, CURLOPT_URL, url);
            curl_easy_setopt(h, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write_file_cb);
            curl_easy_setopt(h, CURLOPT_WRITEDATA, f);
            curl_easy_setopt(h, CURLOPT_VERBOSE, 1L);

            CURLcode result; 
            result = curl_easy_perform(h); 

            if (result == CURLE_OK)
            {
                printf("Executing http request OK \n"); 
                long code = 0; 
                CURLcode info_res = curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &code);

                if (info_res != CURLE_OK)
                {
                    fprintf(stderr, "curl_easy_getinfo failed: %s\n",
                            curl_easy_strerror(info_res));
                    Status = STATUS_NOK;
                }
                else if (code == CODE_RESPONSE_OK)
                {
                    printf("HTTP request successful\n");
                    Status = STATUS_OK;
                }
                else
                {
                    printf("HTTP request failed with status code: %ld\n", code);
                    Status = STATUS_NOK;
                }
            }
            else 
            {
                printf("Executing http request NOK and curl code = %d and url = %s \n", result,url); 
                Status = STATUS_NOK; 
            }
            curl_slist_free_all(headers); 
        }
        else
        {
            printf("Creating the handle NOK \n"); 
            Status = STATUS_NOK; 
        }

        curl_easy_cleanup(h); 
        curl_global_cleanup();

    }
    else
    {
        Status = STATUS_NOK; 
    }
    fclose(f); 

    return Status;
}
