#pragma once

#include "stivale2.h"

// Check if a mod exists
int check_mod(char* mod);

// Initialize header variable to have mods list
void set_hdr(struct stivale2_struct_tag_modules* hdr);

// Call a mod off the list
int execute_mod(char* mod);
