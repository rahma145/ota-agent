#include <systemd/sd-bus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include "log.h"
#include "hawkbit.h"
#include "utilities.h"

#define HREF_LEN 512

struct ctx { int done; int result; };

static int print_properry(sd_bus *bus, const char *name, char **RetValue) {

    return sd_bus_get_property_string(bus, "de.pengutronix.rauc", "/", "de.pengutronix.rauc.Installer", name, NULL, RetValue);

}

static int on_Completed(sd_bus_message *m, void *userdata, sd_bus_error *err)
{
    struct ctx *c = userdata;
    int r = sd_bus_message_read(m, "i", &c->result);
    if (r < 0) return r;
    printf("Completed: %d\n", c->result);
    c->done = 1;
    return 0;
}

static int on_props_changed(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {


    int ret = 0; 

    const char *interface = NULL; 
    const char *property_name = NULL;
    int progress_value = 0; 
    const char *progress_str = NULL;
    int depth = 0; 
    sd_bus_message_read(m, "s", &interface);
    if (strcmp(interface, "de.pengutronix.rauc.Installer") == 0)
    {
        printf("Properties changed for interface: %s\n", interface);
        /* Handle the property change event here */

        // enter the array of changed properties
        ret = sd_bus_message_enter_container(m, 'a', "{sv}");
        if (ret < 0) {
            printf("Failed to enter container: %s\n", strerror(-ret));
            return ret;
        }
        while((ret = sd_bus_message_enter_container(m, 'e', "sv")) > 0){

            // read the property name
            ret = sd_bus_message_read(m,"s", &property_name);
            if (ret < 0) {
                printf("Failed to read property name: %s\n", strerror(-ret));
                return ret;
            }
            if (strcmp(property_name, "Progress") == 0)
            {
                // enter the variant container
                ret = sd_bus_message_enter_container(m, 'v', "(isi)"); 
                if (ret < 0) {
                    printf("Failed to enter variant container: %s\n", strerror(-ret));
                    return ret;
                }
                // read the struct values
                ret = sd_bus_message_read(m, "(isi)", &progress_value, &progress_str, &depth);
                if (ret < 0) {
                    printf("Failed to read struct value: %s\n", strerror(-ret));
                    return ret;
                }
                printf("Progress = %d%% (%s)\n", progress_value, progress_str);

                // exit the variant container
                ret = sd_bus_message_exit_container(m);
                if (ret < 0) {
                    printf("Failed to exit variant container: %s\n", strerror(-ret));
                    return ret;
                }

            }
            else
            {
                //skip the variant container for other properties
                ret = sd_bus_message_skip(m, "v");  
                if (ret < 0) {
                    printf("Failed to skip variant container: %s\n", strerror(-ret));
                    return ret;
                }
            }
            //exit the pair container
            ret = sd_bus_message_exit_container(m);
            if (ret < 0) {
                printf("Failed to exit the pair container: %s\n", strerror(-ret));
                return ret;
            }
        }
        // exit the array container
        ret = sd_bus_message_exit_container(m);
        if (ret < 0) {
            printf("Failed to exit array container: %s\n", strerror(-ret));
            return ret;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    struct ctx ctx = {0}; // Initialize the context
    int ret = 0; 

    sd_bus *bus = NULL; 
    sd_bus_error *bus_error = NULL; 
    const char *Slot = NULL; 
    sd_bus_message *message = NULL; 
    char *RetValue = NULL;

    struct buf response = {0}; 
    RetStatus status; 
    char *href = NULL; 

    status = hawkbit_poll(argv[1], argv[2], argv[3], argv[4], &response); 
    sleep(5); 
    if (status == STATUS_OK)
    {
        printf ("hawkbit poll OK and response = %s \n", response.data);
        href = malloc(HREF_LEN * sizeof(char)); 
        status = hawkbit_has_update(response.data, href, HREF_LEN);
        if (status == STATUS_OK)
        {
            printf("update availabe and href = %s \n", href); 
            struct buf response2 = {0};
            char *url; 
            status = hawkbit_get(href, argv[4], &response2);
            if (status == STATUS_OK)
            {
                printf("deployement info = %s \n", response2.data); 
                struct artifact_info info = {0}; 
                status = hawkbit_parse_deployment(response2.data, &info); 
                if (status == STATUS_OK)
                {
                    printf("filename = %s \n", info.filename);
                    printf("sha256 = %s \n", info.sha256); 
                    printf("size = %ld \n", info.size); 
                    printf("download url = %s \n", info.download_url); 

                    //downloading the bundle
                    status = hawkbit_download(info.download_url, argv[4], "/data/update.raucb");
                    if (status == STATUS_OK)
                    {
                        printf("downloaded successfully \n"); 
                        // Verifying the size 
                        struct stat st;
                        if (stat("/data/update.raucb", &st) == 0 && st.st_size == info.size) {
                            printf("Taille OK : %ld octets\n", st.st_size);
                            char sha256[65]; 
                            status = calculate_sha256("/data/update.raucb", sha256); 
                            if (status == STATUS_OK)
                            {
                                printf ("sha256 = %s \n", sha256); 
                                if ((strcmp(sha256, info.sha256)) == 0)
                                {
                                    printf("sha256 OK \n"); 
                                    cJSON* FeedBackObject = NULL; 
                                    status = hawkbit_build_feedback_json("proceeding", "none", &FeedBackObject); 
                                    if (status == STATUS_OK)
                                    {
                                        status = hawkbit_send_feedback(argv[1], argv[4], FeedBackObject); 
                                        if (status == STATUS_OK)
                                        {
                                            printf("sending feedback OK \n"); 
                                        }
                                        else
                                        {
                                            printf("sending feedback NOK \n"); 
                                        }
                                        if(FeedBackObject)
                                        {
                                            free(FeedBackObject); 
                                            FeedBackObject = NULL; 
                                        }
                                    }
                                    else
                                    {
                                        printf("error building the json object \n"); 
                                    }
                                }
                            }
                            else
                            {
                                printf("error while calculating sha256 \n"); 
                            }

                        } 
                        else {
                            printf("Taille incorrecte !\n");
                        }
                    } 
                    else
                    {
                        printf("error while downloading \n"); 
                    }
                }
                else
                {
                    printf("error parsing the deployment info \n"); 
                }
            }
            else
            {
                printf("No deployment info ! \n"); 
            }
            
        } 
        else 
        {
            printf("update not available \n"); 
        }
    }
    else
    {
        printf ("hawkbit poll NOK ! \n"); 
    }

    //Connexion to the system dbus
    ret = sd_bus_open_system(&bus); 
    if (ret < 0)
    {
        printf("error while connecting to the dbus system \n"); 
        return ret; 
    }
    else
    {
        printf("Connected to the system dbus successfully ! \n"); 
    }

    /* Read BootSlot Property */
    ret = print_properry(bus, "BootSlot", &RetValue);
    if (ret < 0)
    {
        printf("error while getting BootSlot property \n"); 
        return log_error(ret, "BootSlot property failed");
    }
    else
    {
        printf("BootSlot = %s \n", RetValue);
    }

    /* Read Operation Property */
    ret = print_properry(bus, "Operation", &RetValue);
    if (ret < 0)
    {
        printf("error while getting Operation property \n"); 
        return log_error(ret, "Operation property failed");
    }
    else
    {
        printf("Operation = %s \n", RetValue);
    }

    /* Read Compatible Property */
    ret = print_properry(bus, "Compatible", &RetValue);
    if (ret < 0)
    {
        printf("error while getting Compatible property \n"); 
        return log_error(ret, "Compatible property failed");
    }
    else
    {
        printf("Compatible = %s \n", RetValue);
    }

    /* Match to the signal Completed */
    ret = sd_bus_match_signal(bus, NULL, "de.pengutronix.rauc", "/", "de.pengutronix.rauc.Installer", "Completed", (void*)on_Completed, &ctx);
    if (ret < 0)
    {
        printf("error while matching the signal Completed \n");
        return log_error(ret, "Completed signal match failed");
    }   
    else
    {
        printf("Completed signal matched successfully ! \n");
    }

    /* Match to the signal PropertiesChanged */
    ret = sd_bus_match_signal(bus, NULL, "de.pengutronix.rauc", "/", "org.freedesktop.DBus.Properties", "PropertiesChanged", (void*)on_props_changed, NULL);
    if (ret < 0)
    {
        printf("error while matching the signal PropertiesChanged \n");
        return log_error(ret, "PropertiesChanged signal match failed");
    }
    else
    {
        printf("PropertiesChanged signal matched successfully ! \n");
    }

    /* Install the bundle */
    if (argv[1] != NULL)
    {
        ret = sd_bus_call_method(bus, "de.pengutronix.rauc", "/", "de.pengutronix.rauc.Installer", "InstallBundle", NULL, &message, "sa{sv}", argv[1], 0);
        if (ret < 0)
        {
            printf("error while calling InstallBundle method \n");
            return log_error(ret, "InstallBundle method call failed");
        }
        else
        {
            printf("InstallBundle method called successfully ! \n");
        }
    }
    else
    {
        printf("Please provide the bundle path as an argument \n");
        return -1; 
    }

    while(!ctx.done)
    {
        ret = sd_bus_process(bus, NULL);
        if (ret == 0)
        {
            sd_bus_wait(bus, UINT64_MAX);
        }
        if (ret < 0) break;
        if (ret > 0) continue; 

    }

    return ctx.result == 0 ? 0 : 1;
}
