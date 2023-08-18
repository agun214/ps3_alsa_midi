#include "ps3_connect.h"

struct libevdev* ps3_connect(int* fd_ptr, int* rc_ptr) {
    
    int vendor_id = 0x045e;
    int product_id = 0x028e;

    struct libevdev* dev = NULL;
    int fd = -1;
    int rc = -1;

    for (int i = 0; i < 300; i++) {
        char path[128];
        snprintf(path, sizeof(path), "/dev/input/event%d", i);

        fd = open(path, O_RDONLY|O_NONBLOCK);
        if (fd < 0) {continue;}

        rc = libevdev_new_from_fd(fd, &dev);
        if (rc < 0) {close(fd); continue;}

        if (libevdev_get_id_vendor(dev) == vendor_id &&
            libevdev_get_id_product(dev) == product_id) 
            {break;}
    }
    printf("Device found: %s\n", libevdev_get_name(dev));

    // Update the pointers with the obtained values
    *fd_ptr = fd;
    *rc_ptr = rc;

    return dev; // Return the dev variable to main
}
