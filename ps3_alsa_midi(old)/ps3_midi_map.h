#ifndef PS3_MIDI_MAP_H
#define PS3_MIDI_MAP_H

#include <libevdev-1.0/libevdev/libevdev.h>
#include <alsa/asoundlib.h>

void ps3_midi_map(struct libevdev *dev, int port, snd_seq_t *midi);

#endif // PS3_MIDI_MAP_H

