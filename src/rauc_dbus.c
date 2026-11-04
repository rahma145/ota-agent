#include "rauc_dbus.h"

int print_properry(sd_bus *bus, const char *name, char **RetValue) {

    return sd_bus_get_property_string(bus, "de.pengutronix.rauc", "/", "de.pengutronix.rauc.Installer", name, NULL, RetValue);

}