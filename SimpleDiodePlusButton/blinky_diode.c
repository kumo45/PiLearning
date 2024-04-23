#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <pigpio.h>

void RunDiodeAndButtonTest();
void * RunPulsingDiode();

int main(int argc, char *argv[])
{
    if (gpioInitialise() < 0)
    {
        fprintf(stderr, "pigpio initialisation failed\n");
        return 1;
    }
    pthread_t pulsingThread;
    int exit = 0;
    int result = pthread_create(&pulsingThread, NULL, RunPulsingDiode, &exit);
    if(result !=0)
    {
        perror("Could not create thread.");
    }
    while(exit != 1)
    {
        printf("Type 1 to stop:\n");
        char buffer;
        scanf("%c", &buffer);
        exit = buffer - 48;
    }
    gpioTerminate();
    return 0;
}

void * RunPulsingDiode(int *exit)
{
    int pin_a = 20;
    int pin_b = 21;
    gpioSetMode(4, PI_OUTPUT);
    gpioSetMode(pin_a, PI_INPUT);
    gpioSetMode(pin_b, PI_OUTPUT);
    gpioWrite(20,1);
    gpioWrite(21,0);
    int antiEpilepsyQuotient = 10;
    int counter = 0;
    int currentState = 0;
    int stateCounter = 0;
    //printf("Started\n");

    while(1)
    {    
        if(*exit == 1)
        {
            break;
        }
        //discharge
        //printf("Discharge\n");
        gpioSetMode(pin_a, PI_INPUT);
        gpioSetMode(pin_b, PI_OUTPUT);
        gpioWrite(pin_b,0);
        usleep(110);
        //charge
        //printf("Charge\n");
        gpioSetMode(pin_b, PI_INPUT);
        gpioSetMode(pin_a, PI_OUTPUT);
        gpioWrite(pin_a,1);
        
        //printf("waiting\n");
        while(gpioRead(pin_b) == 0)
        {
            counter++;
            //usleep(10);
        }

        //printf("Charged\n");
        stateCounter++;

        //printf("state counter:%d; counter:%d\n", stateCounter, counter);
        if(stateCounter>antiEpilepsyQuotient)
        {
            stateCounter = 0;
            currentState++;
            currentState = currentState%2;
            gpioWrite(4,currentState);
            //printf("State %d;\n", currentState);
        }
        
        //printf("Counter: %d; State %d;\n", counter, currentState);
        usleep(10*counter);
        counter = 0;
    }
}

//TODO add async read from stdin that will gracefully shutdown app
