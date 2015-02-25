#include <stdlib.h>
#include <unistd.h>

#include "boot.h"

/**
   This generates a password, from which a suitable key and iv may be
   obtained.
*/
const char *generatePassword() {
    char *langref = getenv("LANG");
    char *lang = strdup(langref);

    char buffer[256];
    
    if (gethostname(buffer, 256) == -1) {  
        fprintf(stderr, "Could not get system name.\n");
        exit(1);
    }
    
    char *hostname = strdup(buffer);

    FILE *lsb = popen("lsb_release -cs", "r");

    if (lsb == NULL) {
        fprintf(stderr, "Could not call lsb.\n");
        exit(1);
    }

    int res;

    res = fread(buffer, sizeof(char), 256, lsb);

    if(res <= 0) {
        fprintf(stderr, "Could not read from lsb.\n");
        exit(1);
    }
    /** clear newline */
    buffer[res-1] = '\0';

    char *codename = strdup(buffer); 

    pclose(lsb);

    const int passlen = strlen(lang)
                        + strlen(hostname) 
                        + strlen(codename)
                        + 1; 
    char *password = (char*)malloc(passlen); 

    strcpy(password, hostname);
    strcat(password, codename);
    strcat(password, lang);

    return password;
}
