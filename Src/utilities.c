/*
 * utilities.c
 *
 *  Created on: Apr 21, 2026
 *      Author: julius
 */

#include "utilities.h"


/*
 * @brief Converts an uppercase letter to lowercase
 * Leaves all non-uppercase characters unchanged
 *
 * @param[c] Character to convert
 *
 * @return Lowercase version of the character if uppercase, otherwise original character
 */
int to_lower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return (int)(c + ('a' - 'A'));
    }
    return c;
}
