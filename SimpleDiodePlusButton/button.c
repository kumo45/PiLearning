#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <pigpio.h>

void RunDiodeAndButtonTest();
void RunPulsingDiode();

int main(int argc, char *argv[])
{
    if (gpioInitialise() < 0)
    {
        fprintf(stderr, "pigpio initialisation failed\n");
        return 1;
    }

    RunDiodeAndButtonTest();
    gpioTerminate();
    return 0;
}

void RunDiodeAndButtonTest()
{
    gpioSetMode(4, PI_OUTPUT);
    gpioSetPullUpDown(17,PI_PUD_UP);
    while(1)
    {  
        int buttonState = gpioRead(17);
        gpioWrite(4,buttonState);
        
        printf("Button state is %d\n", buttonState);  
        usleep(10000);
    }
}

//TODO add async read from stdin that will gracefully shutdown app
