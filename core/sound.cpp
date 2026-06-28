#include "engine/sound.h"

Sound::Sound(const std::string &path) : path(path), loaded(true) {}

Sound::~Sound() { dispose(); }

void Sound::play() {
        frame_position = start_position;
        playing = loaded;
}

void Sound::pause() { playing = false; }

void Sound::resume() {
        if (loaded)
                playing = true;
}

void Sound::restart() { frame_position = 0; }

void Sound::set_frame_pos(int position) { frame_position = position; }

void Sound::set_volume(float volume) { this->volume = volume; }

float Sound::get_volume() const { return volume; }

void Sound::dispose() {
        playing = false;
        loaded = false;
}
