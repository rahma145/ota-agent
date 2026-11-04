#ifndef RAUC_DBUS_H
#define RAUC_DBUS_H

#include <systemd/sd-bus.h>

int print_properry(sd_bus *bus, const char *name, char **RetValue); 


#endif // RAUC_DBUS_H