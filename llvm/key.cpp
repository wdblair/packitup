#include <stdlib.h>

#include "boot.h"

/**
   This generates a password, from which a suitable key and iv may be
   obtained.
*/
const char *generatePassword() {
    char *lang = getenv("LANG");
    return strdup(lang);
}
