	// Create MIDI number note array
	const int NOTE_MAP[] = {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76};
	int channel = 0;
	int invert_axis = 1;

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

