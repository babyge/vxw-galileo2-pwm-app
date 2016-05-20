/*
* Copyright (c) 2016, Wind River Systems, Inc.
*
* Redistribution and use in source and binary forms, with or without modification, are
* permitted provided that the following conditions are met:
*
* 1) Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2) Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* 3) Neither the name of Wind River Systems nor the names of its contributors may be
* used to endorse or promote products derived from this software without specific
* prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/*
 * DESCRIPTION
 * Utilities to manipulate GPIO pins on a Intel Galileo Gen 2
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "gpioutils.h"

#define BUF_SZ  128

/*
 * gpio_alloc() - Allocate/reserve a GPIO pin
 *
 * Arguments: gpionum - pin number to allocate
 *
 * Returns: 0 - Success
 *          1 - Failed
 */
int gpio_alloc(int gpionum)
{
    int fd;
    char buffer[BUF_SZ];
    int  bytes;

    /* Create the required GPIO pins and reserve them */
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1)
        {
        fprintf(stderr, "Error opening /sys/class/gpio/export, errno = %d\n", errno);
        return (1);
        }

    sprintf(buffer, "%d", gpionum);

    bytes = write(fd, buffer, strlen(buffer));
    if (bytes != strlen(buffer))
        {
        fprintf(stderr, "Error allocating GPIO %d errno=%d\n", gpionum, errno);
        return(1);
        }

    (void)close(fd);

    return(0);
}

/*
 * gpio_dealloc() - DeAllocate/unreserve a GPIO pin
 *
 * Arguments: gpionum - pin number to allocate
 *
 * Returns: 0 - Success
 *          1 - Failed
 */
int gpio_dealloc(int gpionum)
{
    int fd;
    char buffer[BUF_SZ];
    int  bytes;

    /* Create the required GPIO pins and reserve them */
    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1)
        {
        fprintf(stderr, "Error opening /sys/class/gpio/unexport, errno = %d\n", errno);
        return (1);
        }

    sprintf(buffer, "%d", gpionum);

    bytes = write(fd, buffer, strlen(buffer));
    if (bytes != strlen(buffer))
        {
        fprintf(stderr, "Error deallocating GPIO %d errno=%d\n", gpionum, errno);
        return(1);
        }

    (void)close(fd);

    return(0);
}

/*
 * gpio_set_direction() - Set a GPIO pin as an input or output
 *
 * Arguments: gpionum - pin number to allocate
 *            direction - "in" or "out"
 *
 * Returns: 0 - Success
 *          1 - Failed
 */

int gpio_set_direction(int gpionum, char *direction)
{
    int fd;
    char buffer[BUF_SZ];
    int  bytes;

    /* Check the direction argument */
    if ( (strcasecmp(direction, "in") != 0) &&
         (strcasecmp(direction, "out") != 0) )
        return(1);

    /* Open the pin */
    sprintf(buffer, "/sys/class/gpio/gpio%d/direction", gpionum);

    fd = open(buffer, O_WRONLY);
    if (fd == -1)
        {
        fprintf(stderr, "Error opening %s, errno = %d\n", buffer, errno);
        return (1);
        }

    /* Set the direction */
    bytes = write(fd, direction, strlen(direction));
    if (bytes != strlen(direction))
        {
        fprintf(stderr, "Error setting GPIO %d as %s errno=%d\n", gpionum, direction, errno);
        return (1);
        }

    /* Close the pin */
    (void) close(fd);

    return(0);
}

/*
 * gpio_write_pin() - Set the output state of a GPIO pin
 *
 * Arguments: gpionum - pin number to allocate
 *            value - "0" or "1"
 *
 * Returns: 0 - Success
 *          1 - Failed
 */

int gpio_write_pin(int gpionum, char *value)
{
    int fd;
    char buffer[BUF_SZ];
    int  bytes;

    /* Check the value argument */
    if ( (strcasecmp(value, "0") != 0) &&
         (strcasecmp(value, "1") != 0) )
        return(1);

    /* Open the pin */
    sprintf(buffer, "/sys/class/gpio/gpio%d/value", gpionum);
    fd = open(buffer, O_WRONLY);
    if (fd == -1)
        {
        fprintf(stderr, "Error opening %s, errno = %d\n", buffer, errno);
        return (1);
        }

    /* Write the value */
    bytes = write(fd, value, strlen(value));
    if (bytes != strlen(value))
        {
        fprintf(stderr, "Error setting GPIO %d to %s errno=%d\n", gpionum, value, errno);
        return (1);
        }

    /* Close the pin */
    (void) close(fd);

    return (0);

}
