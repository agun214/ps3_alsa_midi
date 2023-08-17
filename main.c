#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <alsa/asoundlib.h>
#include "ps3_connect.h" // Include your custom header (?)
#include "setup_midi_port.h"  // Include the MIDI setup header

/*

gcc -c ps3_connect.c -o ps3_connect.o

gcc -c setup_midi_port.c -o setup_midi_port.o

gcc -o ps3_alsa_midi main.c ps3_connect.o setup_midi_port.o -levdev -lasound

*/

int main(int argc, char** argv) {
    // ...    	int ps3_connect(int* fd_ptr, int* rc_ptr)
    int fd, rc, port;

	snd_seq_t* midi;

    ps3_connect(&fd, &rc);

	setup_midi_port(&midi, &port);
    // ...

    time_t start_time = time(NULL);
    int duration = 2; // 10 seconds

    while (1) {
        // Your loop logic here

        // Check if 10 seconds have elapsed
        time_t current_time = time(NULL);
        if (current_time - start_time >= duration) {
            break; // Exit the loop after 10 seconds
        }
    }	


	snd_seq_close(midi);
    return 0;
}
