#pragma once
#include "defines.h"

/*
* Gets the length of a 0 terminated string.
* 
* @param str - The string that will be queried for length
* @return u64 - The lenght of the string
*/
VAPI u64 string_length(const char* str);

/**
* Duplicate a 0 terminated string.
* 
* @param str - The string that will be duplicated
* @return char * - A pointer to the duplicated string
*/
VAPI char* string_duplicate(char* str);


/**
* Case-sensitive check to see if 2 strings are equal
* 
* @param str1 - The string the base string
* @param str2 - The string to check against
* @return b8 - TRUE is the same, FALSE otherwise
*/
VAPI b8 strings_equal(const char* str1, const char* str2);