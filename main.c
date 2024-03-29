#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <alsa/asoundlib.h>
#include "ps3_connect.h"
#include "setup_midi_port.h"
#include "ps3_midi_map.h"

/*
gcc -c ps3_connect.c -o ps3_connect.o
gcc -c setup_midi_port.c -o setup_midi_port.o
gcc -c ps3_midi_map.c -o ps3_midi_map.o
gcc -o ps3_alsa_midi main.c ps3_connect.o setup_midi_port.o ps3_midi_map.o -levdev -lasound
*/

int main(int argc, char** argv) {
    // connect controller and open MIDI port
	int fd, rc, port;
	struct libevdev* dev;
	snd_seq_t* midi;
	dev = ps3_connect(&fd, &rc);
	setup_midi_port(&midi, &port);

	ps3_midi_map(dev, port, midi);

	// clean up and close
    libevdev_free(dev);
    close(fd);
	snd_seq_close(midi);
    return 0;
}
