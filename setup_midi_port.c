#include "setup_midi_port.h"
//#include <alsa/asoundlib.h>
//#include <unistd.h>
//#include "ps3_connect.h"

// create_alsamidi_port
int setup_midi_port(snd_seq_t** midi_ptr, int* port_ptr) {

    // Open a MIDI device
	snd_seq_t* midi = NULL;
	snd_seq_open(&midi, "default", SND_SEQ_OPEN_OUTPUT, 0);
	snd_seq_set_client_name(midi, "Gamepad MIDI");

	// Create a MIDI port

	int port = snd_seq_create_simple_port(midi, "Gamepad", SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    // Update the pointer with the obtained values

	*midi_ptr = midi;
	*port_ptr = port;

    printf("port: open\n");
}
