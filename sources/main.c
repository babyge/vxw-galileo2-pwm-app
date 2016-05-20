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
 * A sample application to use a PWM channel to make a LED appear to glow.
 *
 * A LED attached to D6 of the Grove adapter on a Galileo Gen2 board, PWM 5
 * is used to modulate the LED to make it appear to 'glow'
 *
 * The program will set up the required GPIO pins and run the "glow" loop
 * forever
 */


#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "gpioutils.h"

#define INCREMENT   0.01
#define MAX_PERCENT 1.0
#define MIN_PERCENT 0.0


#define INVERSE_POLARITY 0     /* Set to 1 to invert the polarity */

extern void usleep (long);


static void pwmGlow (void);
static void setup(void);
static void error_exit(char *);

/*******************************************************************************
* main - User application entry function
*
* This the the entry point for a VxWorks User Mode application
*/
int main ( int argc, char *argv[] )
    {

    setup();     /* Setup the GPIO signals as needed */

    pwmGlow();   /* The PWM Demo */

    }


/*******************************************************************************
* error_exit - Clean up and exit with an error
*
* Print the message and value of errno on the console, deallocate the GPIO
* pins, and exit with -1 status code
*
* Arguments - msg - Message to display as the program terminates
*
* Returns - calls exit()
*
*/

static void error_exit(char *msg)
{
    printf(">>> %s (%d)\n", msg, errno);

    gpio_dealloc(20);
    gpio_dealloc(21);
    gpio_dealloc(68);

    exit(-1);
}

/*******************************************************************************
* setup - Allocate and program the GPIO pins for this program0
*
* Setup the GPIO pins as follows:
*   GPIO20 - Output - Low
*   GPIO21 - Input
*   GPIO68 - Output 0 High
*
* Arguments - void
*
* Returns - runs sucessfully or calls exit()
*
*/
void setup()
{


    /* Set GPIO pin 20 as output, value LOW */

    if (gpio_alloc(20))
        error_exit("Allocating GPIO20");
    if (gpio_set_direction(20, "out"))
        error_exit("Setting GPIO20 direction");
    if (gpio_write_pin(20,"0"))
        error_exit("Setting GPIO20 to 0");

    /* Set GPIO pin 2 as input */
    if (gpio_alloc(21))
        error_exit("Allocating GPIO21");
    if (gpio_set_direction(21, "in"))
        error_exit("Setting GPIO21 direction");

    /* Set GPIO pin 68 as output HIGH (sets up MUX for PWM) */
    if (gpio_alloc(68))
        error_exit("Allocating GPIO68");
    if (gpio_set_direction(68,"out"))
        error_exit("Setting GPIO68 direction");
    if (gpio_write_pin(68,"1"))
        error_exit("Setting GPIO68 value");
}

/*******************************************************************************
* pwmGlow - Demonstrate some PWM utility calls and they slowly cycle the
*  LED connected to Grove D6.  Every 5th cycle, disable the PWM and re-enable
*  it on the next cycle
*
* Arguments - void
*
* Returns - runs forever or calls exit() on failure
*
*/
void pwmGlow()
    {
    char buf[64];
    int fd;

    int enableFd;
    int enabled;
    uint32_t cycle;

    int dutyFd;
    int length;
    int bytes;
    uint32_t period;


   /* Display how many PWM channels are available */
   fd = open("/sys/class/pwm/pwmchip0/npwm", O_RDWR, 0664);
   if (fd < 0)
       error_exit("Error opening number of PWMs");
   memset(buf, 0, sizeof(buf));
   bytes = read(fd, buf, sizeof(buf));
   printf("Found %s PWM channels\n", buf);
   close(fd);


    /* Set PWM5 period */
    fd = open("/sys/class/pwm/pwmchip0/pwm5/period", O_RDWR, 0664);
    if (fd < 0)
        error_exit("Error opening PWM5 period");

    period = 200*1000; /* 200 uS */

    length = snprintf(buf, sizeof(buf), "%d", period);
    bytes = write(fd, buf, length*sizeof(char));
    if (bytes != length)
        error_exit("Error setting PWM period");

    (void)close(fd);


    /* Set the the polarity */
    fd = open("/sys/class/pwm/pwmchip0/pwm5/polarity", O_RDWR, 0664);
    if (fd < 0)
        error_exit("Error opening PWM5 polarity");

    length = snprintf(buf, sizeof(buf), (INVERSE_POLARITY == 1 ? "inversed" : "normal") );


    bytes = write(fd, buf, length*sizeof(char));
    if (bytes != length)
        error_exit("Error setting PWM polarity");

    (void)read(fd, buf, sizeof(buf));
    printf ("Polarity is %s\n", buf);

    (void)close(fd);

    /* Enable the PWM signal */
    enableFd = open("/sys/class/pwm/pwmchip0/pwm5/enable", O_RDWR, 0664);
    if (enableFd < 0)
        error_exit("Error opening PWM enable");

    enabled = 1; /* enable */
    length = snprintf(buf, sizeof(buf), "%d", enabled);
    bytes = write(enableFd, buf, length*sizeof(char));
    if (bytes != length)
        error_exit("Error enabling PWN output signal");

    /* Open the PWM duty cycle */
    dutyFd = open("/sys/class/pwm/pwmchip0/pwm5/duty_cycle", O_RDWR, 0664);
    if (dutyFd < 0)
        error_exit("Error opening PWM duty cycle");

    /* Setup intial values */
    float percentage = 0.0f;
    float increment = INCREMENT;
    uint32_t duty = (uint32_t)(percentage*period);



    cycle = 0;

    while (1)
        {

        percentage = percentage + increment;
        duty = (uint32_t)(percentage*period);

        /* printf("Duty: %d, Percent; %f\n", duty, percentage); */

        length = sprintf(buf, "%d", duty);

        bytes = write(dutyFd, buf, length * sizeof(char));
        if (bytes != length)
            error_exit("Errro writing PWM duty cycle");

        usleep(20000);


        if (percentage >= MAX_PERCENT)
            {
            percentage =  MAX_PERCENT;
            increment  = -INCREMENT;
            }
        else if (percentage <= MIN_PERCENT)
            {
            percentage = MIN_PERCENT;
            increment  = INCREMENT;

             /* Every 4th cycle, disable the PWM */
            if (cycle >= 4)
                {
                enabled = 0; /* disable every 4th cycle */
                length = snprintf(buf, sizeof(buf), "%d", enabled);
                bytes = write(enableFd, buf, length*sizeof(char));
                if (bytes != length)
                    error_exit("Error writing PWN output enable");

                cycle = 0;
                }
            else
                {
                if (enabled != 1)
                    {
                    enabled = 1; /* enable */
                    length = snprintf(buf, sizeof(buf), "%d", enabled);
                    bytes = write(enableFd, buf, length*sizeof(char));
                    if (bytes != length)
                        error_exit("Error writing PWM output enable");
                    }

                cycle++;
                }

            }
        }

    exit(0);
    }
