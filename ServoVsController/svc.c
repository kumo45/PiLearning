#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <fcntl.h>  // required for I2C device configuration
#include <sys/ioctl.h> // required for I2C device usage
#include <linux/i2c-dev.h> // required for constant definitions
#include <linux/joystick.h>

#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */

int currentPosition = 0;
int lastPosition = 0;
int file_i2c = -1;
int length = -1;

struct rotateArgs 
{
    int engine;
    bool clockwise;
    bool rotate;
    bool exit;
};

void *rotate_servo(void *arguments)
{
    struct rotateArgs *args = arguments;
    int baseAdress = 0x08;
    int addressOffset = 0x04;
    double baseAngle = 292;
    double angleInc = 10.65;
    double currentAngle[] = {0,0};
    int anglePressInc = 10;
    int minAngle = 0;
    int maxAngle = 180;
    bool run = true;
    bool axisPressed[] = {false,false};
    char buffer[5];
    int count = 0;
    while(!args->exit)
    {
        if(args->rotate)
        {
            while(args->rotate)
            {
                if(args->clockwise)
                {
                    currentAngle[args->engine] += 1;
                    if(currentAngle[args->engine] > 180)
                    {
                        currentAngle[args->engine] = 180;
                    }
                }
                else
                {
                    currentAngle[args->engine] -= 1;
                    if(currentAngle[args->engine] < 0)
                    {
                        currentAngle[args->engine] = 0;
                    }
                }
                double x = baseAngle + angleInc*currentAngle[args->engine]; 
                double y = floor(x);
                int calculatedAmount = (int)y;
                int selectedEngine = baseAdress+(addressOffset*args->engine);
                buffer[0] = selectedEngine;   // "stop time" reg for channel 0
                buffer[1] = calculatedAmount & 0xff; // The "low" byte comes first...
                buffer[2] = (calculatedAmount>>8) & 0xff; // followed by the high byte.
                write(file_i2c, buffer, 3); // Initiate the write.
                usleep(10000);
            }
        }
    }

                

}

int main()
{
    char *filename = (char*)"/dev/i2c-1"; // Define the filename
    file_i2c = open(filename, O_RDWR); // open file for R/W

    if (file_i2c < 0)
    {
    printf("Failed to open file!");
    return -1;
    }

    int joyfile = open ("/dev/input/js0", O_RDONLY);
    if(joyfile == -1)
    {
        printf("Could not open joystick\n");
        return -1;
    }

    printf("1\n");
    struct js_event event;
    int addr = 0x40;    // PCA9685 address
    ioctl(file_i2c, I2C_SLAVE, addr); // Set the I2C address for upcoming
                                    //  transactions

    char buffer[5];   // Create a buffer for transferring data to the I2C device

    // First we need to enable the chip. We do this by writing 0x20 to register
    //  0. buffer[0] is always the register address, and subsequent bytes are
    //  written out in order.
    buffer[0] = 0;    // target register
    buffer[1] = 0x20; // desired value
    int length = 2;       // number of bytes, including address
    write(file_i2c, buffer, length); // initiate write

    // Enable multi-byte writing.

    buffer[0] = 0xfe;  
    buffer[1] = 0x1e;
    write(file_i2c, buffer, length);

    // Write the start time out to the chip. This is the time when the chip will
    //  generate a high output.

    buffer[0] = 0x06;  // "start time" reg for channel 0
    buffer[1] = 0;     // We want the pulse to start at time t=0
    buffer[2] = 0;
    length = 3;        // 3 bytes total written
    write(file_i2c, buffer, length); // initiate the write

    buffer[0] = 0x0A;  // "start time" reg for channel 0
    buffer[1] = 0;     // We want the pulse to start at time t=0
    buffer[2] = 0;
    length = 3;        // 3 bytes total written
    write(file_i2c, buffer, length); // initiate the write
    // Write the stop time out to the chip. This is the time when the chip will
    //  generate a low output. The value is in units of 1.2us. 1.5ms corresponds
    //  to "neutral" position. This is where the value 1250 below comes
    //  from.

    printf("2\n");
    int baseAdress = 0x08;
    int addressOffset = 0x04;
    double baseAngle = 292;
    double angleInc = 10.65;
    double currentAngle[] = {0,0};
    int anglePressInc = 10;
    int minAngle = 0;
    int maxAngle = 180;
    bool run = true;
    bool axisPressed[] = {false,false};
    printf("3\n");
    struct rotateArgs *rotate_args_s1, *rotate_args_s2;
    rotate_args_s1 = malloc(sizeof(struct rotateArgs) * 1);
    rotate_args_s1->engine = 0;
    rotate_args_s1->clockwise = true;
    rotate_args_s1->rotate = false;
    rotate_args_s1->exit = false;
    rotate_args_s2 = malloc(sizeof(struct rotateArgs) * 1);
    rotate_args_s2->engine = 1;
    rotate_args_s2->clockwise = true;
    rotate_args_s2->rotate = false;
    rotate_args_s2->exit = false;
    printf("pre-thread\n");
    pthread_t rotateThread1,rotateThread2;
    int result = pthread_create(&rotateThread1, NULL, rotate_servo, rotate_args_s1);
    if(result !=0)
    {
        perror("Could not create thread.");
    }
    result = pthread_create(&rotateThread2, NULL, rotate_servo, rotate_args_s2);
    if(result !=0)
    {
        perror("Could not create thread.");
    }

    printf("threads started\n");

    while(read(joyfile, &event, sizeof(event)) && run)
    {
        switch(event.type)
        {
            case JS_EVENT_BUTTON:
                if(event.number == 11)
                {
                    run = false;
                }
                break;
            case JS_EVENT_AXIS:
                if(event.number == 0)
                {
                    rotate_args_s1->engine = event.number;
                    rotate_args_s1->clockwise = event.value > 0;
                    rotate_args_s1->rotate = event.value != 0;
                }
                else
                {
                    rotate_args_s2->engine = event.number;
                    rotate_args_s2->clockwise = event.value > 0;
                    rotate_args_s2->rotate = event.value != 0;
                }
                break;
            default:
                break;
        }

        
    }

    close(joyfile);
    rotate_args_s1->exit = true;
    pthread_join(rotateThread1, NULL);
    rotate_args_s2->exit = true;
    pthread_join(rotateThread2, NULL);

    return 0;
}