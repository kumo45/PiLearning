#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>  // required for I2C device configuration
#include <sys/ioctl.h> // required for I2C device usage
#include <linux/i2c-dev.h> // required for constant definitions

#define QUEUE_SIZE 4

char* queue[QUEUE_SIZE];
int currentPosition = 0;
int lastPosition = 0;

void enqueue(char *command)
{
    if(lastPosition > QUEUE_SIZE-1)
    {
        lastPosition = 0;
        printf("Resetting last counter\n");
    }

    if(queue[lastPosition] != NULL)
    {
        printf("Queue is full!\n");
        return;
    }

    queue[lastPosition] = command;
    printf("Enquequed %s\n", command);

    lastPosition++;
}

char* dequeue()
{
    if(currentPosition > QUEUE_SIZE-1)
    {
        currentPosition = 0;
        printf("Resetting current counter\n");
    }

    if(queue[currentPosition] == NULL)
    {
        printf("Queue is empty!\n");
        return NULL;
    }

    char *currentCommand = queue[currentPosition];
    printf("Dequequed %s\n", currentCommand);
    queue[currentPosition] = NULL;
    currentPosition++;

    return currentCommand;
}

bool verify_command(char* command)
{
    int commandLenght = 5;
    char engineDesignation[22] = {'0','1','2','3','4','5','6','7','8','9',
                                  'a','b','c','d','e','f','A','B','C','D','E','F'};
    
    if(strlen(command) != commandLenght)
    {
        printf("Wrong length of command: %s\n", command);
        return false;
    }

    bool isDesignationOk = false;
    for(int i = 0; i < 22; i++)
    {
        isDesignationOk |= (command[0] == engineDesignation[i]); 
    }
    if(!isDesignationOk)
    {
        printf("Engine designation %c is incorrect\n", command[0]);
        return false;
    }

    int amount = 100 * (command[1]-48) + 10 * (command[2]-48) + (command[3]-48);

    if(amount > 999 || amount < 0)
    {
        printf("Amount parsed is illegal: %d, from %s\n", amount, command);
        return false;
    }

    return true;
}

int main()
{
    char *filename = (char*)"/dev/i2c-1"; // Define the filename
    int file_i2c = open(filename, O_RDWR); // open file for R/W

    if (file_i2c < 0)
    {
    printf("Failed to open file!");
    return -1;
    }

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

    int baseAdress = 0x08;
    int addressOffset = 0x04;
    double baseAngle = 292;
    double angleInc = 10.65;

    while(1)
    {
        char command[5];
        printf("Write the command:");
        scanf("%s",command);
        //printf("%s\n", command);
        int amount = 100 * (command[1]-48) + 10 * (command[2]-48) + (command[3]-48);
        //printf("%d\n", amount); 
        double x = baseAngle + angleInc*amount; 
        double y = floor(x);
        //printf("%f\n",y);
        int calculatedAmount = (int)y;
        //printf("%d\n", calculatedAmount);
        int selectedEngine = baseAdress+(addressOffset*(command[0]-48));
        //printf("%d\n",selectedEngine);
        buffer[0] = selectedEngine;   // "stop time" reg for channel 0
        buffer[1] = calculatedAmount & 0xff; // The "low" byte comes first...
        buffer[2] = (calculatedAmount>>8) & 0xff; // followed by the high byte.
        write(file_i2c, buffer, length); // Initiate the write.
    }
    return 0;
}