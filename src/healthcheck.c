#include "rauc_dbus.h"
#include "hawkbit.h"

int main()
{
    sd_bus *bus = NULL; 
    int ret = 0; 
    char *RetValue = NULL;
    sd_bus_message *message = NULL; 
    //Connexion to the system dbus
    ret = sd_bus_open_system(&bus); 
    if (ret < 0)
    {
        //envoyer mark-bad
        printf("error while connecting to the dbus system \n"); 
        return ret; 
    }
    else
    {
        printf("Connected to the system dbus successfully ! \n"); 
        /* Read BootSlot Property */
        ret = print_properry(bus, "BootSlot", &RetValue);
        if (ret < 0)
        {   //envoyer mark-bad
            printf("error while getting BootSlot property \n"); 
            return ret; 
        }
        else
        {
            printf("BootSlot = %s \n", RetValue);
            //envoyer mark-good
            ret = sd_bus_call_method(bus, "de.pengutronix.rauc", "/", "de.pengutronix.rauc.Installer", "Mark", NULL, &message, "ss", "good", "booted");
            if (ret < 0)
            {
                printf("mark-good failed ! \n"); 
            }

        }
    }

    return 0; 
}