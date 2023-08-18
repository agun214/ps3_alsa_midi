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
    // ...    	
	int fd, rc, port;
	struct libevdev* dev;
	snd_seq_t* midi;

	dev = ps3_connect(&fd, &rc);
	setup_midi_port(&midi, &port);

	//ps3_midi_map(dev, midi_port, &midi);
	// Create MIDI number note array
	const int NOTE_MAP[] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76};
	int channel = 0;
	int invert_axis = 1;

    // Process events
    while (1) {
        struct input_event ev;
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0) {
            // Translate input event into MIDI message
            snd_seq_event_t midi_event;
            snd_seq_ev_clear(&midi_event);
            snd_seq_ev_set_source(&midi_event, port);
            snd_seq_ev_set_subs(&midi_event);
            snd_seq_ev_set_direct(&midi_event);

			// Joystick to pitchbend						
			if (ev.type == EV_ABS && ev.code == ABS_X) { 	
				// assign PB channel based on axis
				channel = 0; 
				//invert_axis = 1;

				// Calculate the pitch bend value based on the joystick position
				int joystick_min = libevdev_get_abs_minimum(dev, ev.code);
				int joystick_max = libevdev_get_abs_maximum(dev, ev.code);
				int joystick_mid = (joystick_max + joystick_min);
				int pitchbend_val = (int)(((float)(ev.value - joystick_mid) / (float)(joystick_max - joystick_min)) * 16383.5f);

				// Set the MIDI event to a pitchbend event
				midi_event.type = SND_SEQ_EVENT_PITCHBEND;
				midi_event.data.control.channel = channel;
				midi_event.data.control.value = pitchbend_val;
				
			} else if (ev.type == EV_ABS && ev.code == ABS_Y) { // Y AXIS		MOD WHEEL
				// assign PB channel based on axis
				channel = 0; 
				invert_axis = -1;
				
				// Calculate the pitch bend value based on the joystick position
				int joystick_min = libevdev_get_abs_minimum(dev, ev.code);
				int joystick_max = libevdev_get_abs_maximum(dev, ev.code);
				int joystick_mid = (joystick_max + joystick_min) / 2;
				int modwheel_val = (int)(((float)(ev.value - joystick_mid) / (float)(joystick_max - joystick_min)) * 255.5f * invert_axis);
				modwheel_val = abs(modwheel_val);

				// Set the MIDI event to a pitchbend event
				midi_event.type = SND_SEQ_EVENT_CONTROLLER;
				midi_event.data.control.channel = channel;  // set MIDI channel to 1
				midi_event.data.control.param = 1;  // set controller to modulation wheel
				midi_event.data.control.value = modwheel_val;  // use joystick position as controller value	
					
			} else if (ev.type == EV_ABS && ev.code == ABS_RX) { 			// RX AXIS	
				// assign PB channel based on axis
				channel = 0; 
				invert_axis = 1;

				// Calculate the pitch bend value based on the joystick position
				int joystick_min = libevdev_get_abs_minimum(dev, ev.code);
				int joystick_max = libevdev_get_abs_maximum(dev, ev.code);
				int joystick_mid = (joystick_max + joystick_min);
				int pitchbend_val = (int)(((float)(ev.value - joystick_mid) / (float)(joystick_max - joystick_min)) * 4778.67f * invert_axis);

				// Set the MIDI event to a pitchbend event
				midi_event.type = SND_SEQ_EVENT_PITCHBEND;
				midi_event.data.control.channel = channel;
				midi_event.data.control.value = pitchbend_val;
						
			} else if (ev.type == EV_ABS && ev.code == ABS_RY) { 			// RY AXIS
				// assign PB channel based on axis
				channel = 0; 
				invert_axis = -1;
				
				// Calculate the pitch bend value based on the joystick position
				int joystick_min = libevdev_get_abs_minimum(dev, ev.code);
				int joystick_max = libevdev_get_abs_maximum(dev, ev.code);
				int joystick_mid = (joystick_max + joystick_min);
				int pitchbend_val = (int)(((float)(ev.value - joystick_mid) / (float)(joystick_max - joystick_min)) * 1365.33f * invert_axis);

				// Set the MIDI event to a pitchbend event
				midi_event.type = SND_SEQ_EVENT_PITCHBEND;
				midi_event.data.control.channel = channel;
				midi_event.data.control.value = pitchbend_val;

			} else if (ev.type == EV_ABS && (ev.code == ABS_HAT0X || ev.code == ABS_HAT0Y || ev.code == ABS_Z || ev.code == ABS_RZ)) {
				int note_index = -1;				
				// assign PB channel based on axis
				switch (ev.code) {
					case ABS_HAT0X: note_index = 12; break;
					case ABS_HAT0Y:	note_index = 13; break;
					case ABS_Z: note_index = 14; break;
					case ABS_RZ: note_index = 15; break;
				}
				if (note_index != -1) {
					if (ev.value != 0) {
						midi_event.type = SND_SEQ_EVENT_NOTEON;
						midi_event.data.note.channel = 0;
						midi_event.data.note.note = NOTE_MAP[note_index];
						midi_event.data.note.velocity = 127;
					} else {
						midi_event.type = SND_SEQ_EVENT_NOTEOFF;
						midi_event.data.note.channel = 0;
						midi_event.data.note.note = NOTE_MAP[note_index];
						midi_event.data.note.velocity = 0;
					}
				}

			// Buttons to notes
			} else if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 1) {
				int note_index = -1;
				switch (ev.code) {
					case 304: case 305: case 307 ... 318:
						note_index = ev.code - 304;
						break;
					case 2:
						note_index = 10;
						break;
					case 5:
						note_index = 11;
						break;
				}
				if (note_index != -1) {
					if (ev.value != 0) {
						midi_event.type = SND_SEQ_EVENT_NOTEON;
						midi_event.data.note.channel = 0;
						midi_event.data.note.note = NOTE_MAP[note_index];
						midi_event.data.note.velocity = 127;
					} else {
						midi_event.type = SND_SEQ_EVENT_NOTEOFF;
						midi_event.data.note.channel = 0;
						midi_event.data.note.note = NOTE_MAP[note_index];
						midi_event.data.note.velocity = 0;

					}
				}
            }        

            snd_seq_event_output(midi, &midi_event);
            snd_seq_drain_output(midi);

        } else if (rc == -EAGAIN) {
            // No events available, try again
            continue;
        } else {
            // Error reading event, exit loop
            break;
        }
    }


/*
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
*/
    libevdev_free(dev);
    close(fd);
	snd_seq_close(midi);
    return 0;
}
