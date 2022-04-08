#pragma once

#include "stivale2.h"

// Initialize header variable to have mods list
void set_hdr(struct stivale2_struct_tag_modules* hdr);

// Call a mod off the list
void execute_mod(char* mod);
