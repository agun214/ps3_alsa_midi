#include "gamepad_alsa_midi_functions.h"

/* compile instructions: 
gcc -c gamepad_alsa_midi_functions.c -o gamepad_alsa_midi_functions.o
gcc -o gamepad_alsa_midi main.c gamepad_alsa_midi_functions.o -I/usr/include/libevdev-1.0 -levdev -lasound 
*/

int main(int argc, char **argv) {
    int fd, rc, port;
	snd_seq_t* midi;
	struct libevdev* dev = ps3_connect(0x045e, 0x028e, &fd, &rc);
	setup_midi_port(&midi, &port);
	dev_midi_event_loop(dev, midi, port);

    // Clean up resources
	libevdev_free(dev);
	close(fd);
	return 0;
}
