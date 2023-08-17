#ifndef PS3_CONNECT_H
#define PS3_CONNECT_H

#include <stdio.h>
#include <stdlib.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>

int ps3_connect(int* fd_ptr, int* rc_ptr);

#endif // PS3_CONNECT_H

