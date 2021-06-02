#include "version.h"
#include <stdio.h>
#include <unistd.h>

void print_version() {
    fprintf(stdout, "digiham version %s\n", VERSION);
}