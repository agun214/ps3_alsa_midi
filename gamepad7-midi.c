#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <alsa/asoundlib.h>

/* gcc -o gamepad5-midi gamepad5-midi.c -I/usr/include/libevdev-1.0 -levdev -lasound */
  
#define NUM_CONTROLS 19

// Structure to store the state of each button
struct ControlState {
    int evcode;  // PS3 button code
    int state;    // 0 for released, 1-255 for pressed
    int MIDIevcode;   // midi event associated with the control
};

struct libevdev* ps3_connect(int* fd_ptr, int* rc_ptr);
int setup_midi_port(snd_seq_t** midi_ptr, int* port_ptr);
void MIDI_note(snd_seq_t *midi, int note, int velocity, int channel, int port, int noteoffset);
void MIDI_controller(struct libevdev *dev, struct input_event ev, snd_seq_t *midi, int channel, int port, int invert_axis);
void MIDI_pitchbend(struct libevdev *dev, struct input_event ev, snd_seq_t *midi, int port, int channel, int invert_axis);
void dev_midi_event_loop(struct libevdev *dev, snd_seq_t *midi, int port);

int main(int argc, char **argv) {
	int fd, rc, port;
	snd_seq_t* midi;
	struct libevdev* dev = ps3_connect(&fd, &rc);
	setup_midi_port(&midi, &port);
	dev_midi_event_loop(dev, midi, port);

	// Clean up resources
	libevdev_free(dev);
	close(fd);
	return 0;
}

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

//   MIDI_note(midi, controlStates[controlIndex].MIDIevcode, ev.value, channel, port, noteoffset);
void MIDI_note(snd_seq_t *midi, int note, int velocity, int channel, int port, int noteoffset) {
	snd_seq_event_t midi_event;
	snd_seq_ev_clear(&midi_event);
	snd_seq_ev_set_source(&midi_event, port);
	snd_seq_ev_set_subs(&midi_event);
	snd_seq_ev_set_direct(&midi_event);

	if (velocity != 0) {
		midi_event.type = SND_SEQ_EVENT_NOTEON;
		midi_event.data.note.channel = 0;
		midi_event.data.note.note = note + noteoffset; //controlStates[controlIndex].MIDIevcode 
		midi_event.data.note.velocity = 127;
	} else {
		midi_event.type = SND_SEQ_EVENT_NOTEOFF;
		midi_event.data.note.channel = channel;
		midi_event.data.note.note = note + noteoffset;
		midi_event.data.note.velocity = 0;
	}
    snd_seq_event_output(midi, &midi_event);
    snd_seq_drain_output(midi);
}

void MIDI_controller(struct libevdev *dev, struct input_event ev, snd_seq_t *midi, int channel, int port, int invert_axis) {
	snd_seq_event_t midi_event;
	snd_seq_ev_clear(&midi_event);
	snd_seq_ev_set_source(&midi_event, port);
	snd_seq_ev_set_subs(&midi_event);
	snd_seq_ev_set_direct(&midi_event);
	
	// Calculate the pitch bend value based on the joystick position
	int joystick_min = libevdev_get_abs_minimum(dev, ev.code);
	int joystick_max = libevdev_get_abs_maximum(dev, ev.code);
	int joystick_mid = (joystick_max + joystick_min) / 2;
	int modwheel_val = (int)(((float)(ev.value - joystick_mid) / (float)(joystick_max - joystick_min)) * 255.0f);

	switch(invert_axis){
		case 1:	modwheel_val = modwheel_val - invert_axis; break;
		case -1: modwheel_val = modwheel_val * invert_axis; break;
	}
  	modwheel_val = abs(modwheel_val);
	
	// Set the MIDI event to a pitchbend event
	midi_event.type = SND_SEQ_EVENT_CONTROLLER;
	midi_event.data.control.channel = channel;  // set MIDI channel to 1
	midi_event.data.control.param = 1;  // set controller to modulation wheel
	midi_event.data.control.value = modwheel_val;  // use joystick position as controller value	
	
    // Send the MIDI event
    snd_seq_event_output(midi, &midi_event);
    snd_seq_drain_output(midi);
}

// Function to handle pitch bend events
//   MIDI_pitchbend(dev, ev, &midi, port, channel, 1);
void MIDI_pitchbend(struct libevdev *dev, struct input_event ev, snd_seq_t *midi, int port, int channel, int invert_axis) {
    snd_seq_event_t midi_event;
    snd_seq_ev_clear(&midi_event);
    snd_seq_ev_set_source(&midi_event, port);
    snd_seq_ev_set_subs(&midi_event);
    snd_seq_ev_set_direct(&midi_event);
	
	// Calculate the pitch bend value based on the joystick position
	int joystick_min = libevdev_get_abs_minimum(dev, ev.code);
	int joystick_max = libevdev_get_abs_maximum(dev, ev.code);
	int joystick_mid = (joystick_max + joystick_min);
	int pitchbend_val = (int)(((float)(ev.value - joystick_mid) / (float)(joystick_max - joystick_min)) * 16384.0f);

	switch(invert_axis){
		case 1:	pitchbend_val = pitchbend_val - invert_axis; break;
		case -1: pitchbend_val = pitchbend_val * invert_axis; break;
	}
    // Set the MIDI event to a pitchbend event
    midi_event.type = SND_SEQ_EVENT_PITCHBEND;
    midi_event.data.control.channel = channel;
    midi_event.data.control.value = pitchbend_val;
	
    // Send the MIDI event
    snd_seq_event_output(midi, &midi_event);
    snd_seq_drain_output(midi);
}

void dev_midi_event_loop(struct libevdev *dev, snd_seq_t *midi, int port) {
	int rc;
	int controlIndex;
	int noteoffset = 60;
	int channel = 0;

	// Array to store the state of each button : ev.code, ev.value, MIDI 
	struct ControlState controlStates[NUM_CONTROLS] = {
		{304, 0, 0},      	// A
		{305, 0, 4},      	// B
		{307, 0, 2},      	// X
		{308, 0, 5},      	// Y
		{310, 0, 9}, 		// L1
		{311, 0, 7}, 		// R1
		{314, 0, 1},    	// SELECT
		{315, 0, 3},       	// START
		{316, 0, 6},        // HOME		
		{317, 0, 8}, 		// L3
		{318, 0, 10},   	// R3
		{2, 0, 12},     	// L2
		{5, 0, 11},       	// R2
		{0, 0, 120},       	// Left X-axis
		{1, 0, 121},		// Left Y-axis
		{3, 0, 122},		// Right X-axis
		{4, 0, 123},		// Right Y-axis
		{16, 0, 124},		// Right Y-axis
		{17, 0, 125},		// Right Y-axis
		// Add more buttons as needed
	};

	// Process events
	while (1) {
		struct input_event ev;
		rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
		if (rc == 0) {
		    // Find the corresponding button state
		    controlIndex = -1;

		    for (int i = 0; i < NUM_CONTROLS; i++) {
		        if (ev.code == controlStates[i].evcode) {
		            controlIndex = i;
		            break;
		        }
		    }

		    if (controlIndex != -1 && (ev.type == EV_ABS || ev.type == EV_KEY)) {
				switch (ev.code) {
					// Buttons
					case 2:
					case 5:
					case 304 ... 318:  // Button pressed
						printf("BTN ev: %d, val: %d, MIDI: %d\n", ev.code, ev.value, controlStates[controlIndex].MIDIevcode);
						MIDI_note(midi, controlStates[controlIndex].MIDIevcode, ev.value, channel, port, noteoffset);
                        break;

					case 16:
					case 17:
						printf("DPAD ev: %d, val: %d, MIDI: %d\n", ev.code, ev.value, controlStates[controlIndex].MIDIevcode);
						break;

					// Joystick Axes
					case 0: 						
						// struct libevdev *dev, struct input_event ev, snd_seq_t *midi, int port, int channel, int invert_axis
						MIDI_pitchbend(dev, ev, midi, port, channel, 1); // invert_axis = 1;
                        break;
					case 1:
						// struct libevdev *dev, struct input_event ev, snd_seq_t *midi, int channel, int port, int invert_axis
						MIDI_controller(dev, ev, midi, port, channel, -1);
                        break;
					case 3:
						printf("JOY ev: %d, val: %d, MIDI: %d\n", ev.code, ev.value, controlStates[controlIndex].MIDIevcode);
                        break;
					case 4:
						//printf("JOY Event: time %ld.%06ld, code %d, value %d\n", ev.time.tv_sec, ev.time.tv_usec, ev.code, ev.value);
						printf("JOY ev: %d, val: %d, MIDI: %d\n", ev.code, ev.value, controlStates[controlIndex].MIDIevcode);
						break;
				}
		    }
		} else if (rc == -EAGAIN) {
		    // No events available, try again
		    continue;
		} else {
		    // Error reading event, exit loop
		    break;
		}
	}
}


/*
snd_seq_event_t MIDI_event(struct libevdev *dev, struct input_event ev, snd_seq_t *midi, int port, int channel, int invert_axis) {
    snd_seq_event_t midi_event;
    snd_seq_ev_clear(&midi_event);
    snd_seq_ev_set_source(&midi_event, port);
    snd_seq_ev_set_subs(&midi_event);
    snd_seq_ev_set_direct(&midi_event);



    // Send the MIDI event
    snd_seq_event_output(midi, &midi_event);
    snd_seq_drain_output(midi);
}
*/
