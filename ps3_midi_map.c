#include <stdio.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <alsa/asoundlib.h>

int NOTE_MAP[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
int NOTE_OCTAVE = 60;
int NOTE_VELOCITY = 127;

int MIDI_CHANNEL = 1;
int MIDI_CC = 1;
int MIDI_CC_VALUE = 0;

int pitchbend_val = 0;

void ps3_midi_map(struct libevdev *dev, int port, snd_seq_t *midi) {
    // ...
    //int channel = 0;
    int invert_axis = 1;
	int rc;

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

			int joystick_min = libevdev_get_abs_minimum(dev, ev.code);
			int joystick_max = libevdev_get_abs_maximum(dev, ev.code);
			int joystick_mid = (joystick_max + joystick_min);

			int note_index = -1;				
			// assign PB channel based on axis
			switch (ev.code) {
				// joystick for PBend and CC and PGMCHG
				case ABS_X: 
					invert_axis = 1;
					pitchbend_val = (int)(((float)(ev.value - joystick_mid) / (float)(joystick_max - joystick_min)) * 16383.5f);
					midi_event.type = SND_SEQ_EVENT_PITCHBEND;
					midi_event.data.control.channel = MIDI_CHANNEL;
					midi_event.data.control.value = pitchbend_val;					
					break;
				case ABS_Y: 
					invert_axis = -1;
					joystick_mid = joystick_mid / 2;
					MIDI_CC_VALUE = (int)(((float)(ev.value - joystick_mid) / (float)(joystick_max - joystick_min)) * 255.5f * invert_axis);
					MIDI_CC_VALUE = abs(MIDI_CC_VALUE);
					midi_event.type = SND_SEQ_EVENT_CONTROLLER;
					midi_event.data.control.channel = MIDI_CHANNEL;  // set MIDI channel to 1
					midi_event.data.control.param = MIDI_CC;  // set controller to modulation wheel
					midi_event.data.control.value = MIDI_CC_VALUE;  // use joystick position as controller value
					break;
				case ABS_RX: 
					invert_axis = 1;
					pitchbend_val = (int)(((float)(ev.value - joystick_mid) / (float)(joystick_max - joystick_min)) * 16383.5f);
					midi_event.type = SND_SEQ_EVENT_PITCHBEND;
					midi_event.data.control.channel = MIDI_CHANNEL;
					midi_event.data.control.value = pitchbend_val;					
					break;
				case ABS_RY: 
					invert_axis = -1;
					joystick_mid = joystick_mid / 2;
					MIDI_CC_VALUE = (int)(((float)(ev.value - joystick_mid) / (float)(joystick_max - joystick_min)) * 255.5f * invert_axis);
					MIDI_CC_VALUE = abs(MIDI_CC_VALUE);
					midi_event.type = SND_SEQ_EVENT_CONTROLLER;
					midi_event.data.control.channel = MIDI_CHANNEL;  // set MIDI channel to 1
					midi_event.data.control.param = MIDI_CC;  // set controller to modulation wheel
					midi_event.data.control.value = MIDI_CC_VALUE;  // use joystick position as controller value
					break;
				case ABS_HAT0X:
					if (ev.value == 1) {
						note_index = 12;
						break;
					} else {
						note_index = 13;
						break;
					}

				case ABS_HAT0Y:
					if (ev.value == 1) {
						note_index = 14;
						break;
					} else {
						note_index = 15;
						break;
					}

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
					midi_event.data.note.channel = MIDI_CHANNEL;
					midi_event.data.note.note = (int)(NOTE_MAP[note_index] + NOTE_OCTAVE);
					midi_event.data.note.velocity = NOTE_VELOCITY;
				} else {
					midi_event.type = SND_SEQ_EVENT_NOTEOFF;
					midi_event.data.note.channel = MIDI_CHANNEL;
					midi_event.data.note.note = (int)(NOTE_MAP[note_index] + NOTE_OCTAVE);
					midi_event.data.note.velocity = 0;
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
}
