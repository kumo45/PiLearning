#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <linux/joystick.h>

#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */

int main()
{
    int fd = open ("/dev/input/js0", O_RDONLY);
    if(fd == -1)
    {
        printf("Could not open joystick\n");
        return -1;
    }

    struct js_event event;
    
printf("Reading input:\n");

    while(read(fd, &event, sizeof(event)))
{
    
    printf("time:%d\tvalue:%d\ttype:%d\tnumber:%d\n", event.time, event.value, event.type, event.number);
}

    return 0;
}