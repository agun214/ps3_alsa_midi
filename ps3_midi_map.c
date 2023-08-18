#include <stdio.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <alsa/asoundlib.h>

const int NOTE_MAP[] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76};

void ps3_midi_map(struct libevdev *dev, int port, snd_seq_t *midi) {
    // ...
    int channel = 0;
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

			// Joystick to pitchbend						
			if (ev.type == EV_ABS && ev.code == ABS_RY) { 			// RY AXIS
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
}
