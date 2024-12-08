#include "audio.h"
#include "helpers.h"


static MusicData *current_music = NULL;
static bool looped = false;
static bool stopped = true;
static uint32_t last_start = 0;
static uint32_t next_note = 0;

static uint32_t current_note = 0;
static float current_bpm = 0;
static int current_separation = 0;
static NotificationApp *notification_app;

void setup_audio(NotificationApp *notificationApp) {
    notification_app = notificationApp;
}

static const float reference_freq = 16.35f; //C0

static NotificationMessage note = {
    .data.sound.volume = 1,
    .data.sound.frequency = reference_freq,
    .type = NotificationMessageTypeSoundOn,
};

static NotificationMessage delay = {
    .type=NotificationMessageTypeDelay,
    .data.delay.length=250
};

static NotificationMessage off = {
    .type = NotificationMessageTypeSoundOff,
};

static NotificationSequence music_sequence = {
    &note,
    &delay,
    &off,
    NULL
};

// Function to reverse scale the 16-bit integer to float note_length
// return be between 0.0 and 4.0
float reverse_scale_note_length(uint16_t scaled_note_length) {
    return ((float) scaled_note_length / 65535.0f) * 4.0f;
}

Beat decode_note(uint32_t beat) {
    return (Beat) {
        .note=(beat >> 26) & 0xF,
        .octave=(beat >> 16) & 0x3F,
        .beat_length=reverse_scale_note_length(beat & 0xFFFF),
    };
}

void set_volume(float volume) {
    note.data.sound.volume = volume > 1.0f ? 1.0f : (volume < 0.0f ? 0.0f : volume);
}

float get_delay(uint8_t bpm) {
    return 60000.0f / bpm;
}

float get_frequency(Beat *b) {
    float n = b->octave * 12.f + b->note;
    return reference_freq * powf(2, (n / 12));
}

void set_audio(MusicData *data) {
    current_music = data;
    current_bpm = get_delay(data->bpm);
    current_separation = (int) floorf(current_bpm * current_music->separation);
}


void update_audio() {
    if (!notification_app || current_music == NULL || stopped ||
        (looped == true && current_music->loop == false))
        return;

    size_t t = furi_get_tick();
    if ((t - last_start) > next_note) {
        Beat curr = decode_note(current_music->music_notes[current_note]);
        if (curr.note == NOTE_END) {
            current_note = 0;
            curr = decode_note(current_music->music_notes[current_note]);
        }

        delay.data.delay.length = (int) floorf(current_bpm * curr.beat_length);
        next_note = delay.data.delay.length;
        delay.data.delay.length -= current_separation;

        if (curr.note < 12) {
            note.data.sound.frequency = get_frequency(&curr);
            note.type = NotificationMessageTypeSoundOn;
            off.type = NotificationMessageTypeSoundOff;
        } else if (curr.note == NOTE_NONE) {
            note.type = NotificationMessageTypeSoundOff;
        } else if (curr.note == NOTE_BUZZ) {
            note.type = NotificationMessageTypeVibro;
            note.data.vibro.on = true;
            off.type = NotificationMessageTypeVibro;
            off.data.vibro.on = false;
        }

        notification_message(notification_app, (const NotificationSequence *) &music_sequence);

        last_start = t;
        current_note++;
    }
}

void play_audio() {
    stopped = false;
    looped = false;
    last_start = furi_get_tick();
    next_note = (int) floorf(current_bpm);
    current_note = 0;
}

void stop_audio() {
    stopped = true;
}