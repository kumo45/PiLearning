#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>

#include <pigpio.h>

int main(int argc, char *argv[])
{
    if (gpioInitialise() < 0)
    {
        fprintf(stderr, "pigpio initialisation failed\n");
        return 1;
    }

    RunDiodeAndButtonTest();
    RunPulsingDiode();
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

void RunPulsingDiode()
{
    gpioSetMode(4, PI_OUTPUT);
    gpioSetMode(16, PI_INPUT);
    int antiEpilepsyQuotient = 1000;
    int counter = 0;
    int currentState = 0;
    while(1)
    {          
        if(gpioRead(16) == currentState)
        {
            counter++;
        }
        if(counter == antiEpilepsyQuotient)
        {
            counter = 0;
            currentState = ++currentState%2;
            gpioWrite(4, currentState);
        }
        usleep(110);
    }
}

//TODO add async read from stdin that will gracefully shutdown app