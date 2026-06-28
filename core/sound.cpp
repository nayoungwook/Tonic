#include "engine/sound.h"

#include "engine/error.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

namespace {
MIX_Mixer *shared_mixer = nullptr;
int sound_instance_count = 0;
bool mixer_initialized = false;

std::filesystem::path resolve_sound_path(const std::string &path) {
	std::filesystem::path requested(path);
	if (requested.is_absolute() && std::filesystem::exists(requested))
		return requested;

	std::filesystem::path cwd = std::filesystem::current_path();
	std::array<std::filesystem::path, 4> candidates = {
		cwd / requested,
		cwd / "resources" / requested,
#ifdef ENGINE_RESOURCE_ROOT
		std::filesystem::path(ENGINE_RESOURCE_ROOT) / requested,
#else
		std::filesystem::path(),
#endif
		std::filesystem::path("resources") / requested,
	};

	for (const auto &candidate : candidates) {
		if (!candidate.empty() && std::filesystem::exists(candidate))
			return candidate;
	}

	return cwd / requested;
}

MIX_Mixer *acquire_mixer() {
	if (!mixer_initialized) {
		if (!MIX_Init()) {
			error(std::string("SDL3_mixer init failed: ") +
				SDL_GetError());
		}
		mixer_initialized = true;
	}

	if (shared_mixer == nullptr) {
		shared_mixer = MIX_CreateMixerDevice(
			SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
		if (shared_mixer == nullptr) {
			error(std::string("SDL3_mixer create mixer failed: ") +
				SDL_GetError());
		}
	}

	++sound_instance_count;
	return shared_mixer;
}

void release_mixer() {
	if (sound_instance_count > 0)
		--sound_instance_count;

	if (sound_instance_count != 0)
		return;

	if (shared_mixer != nullptr) {
		MIX_DestroyMixer(shared_mixer);
		shared_mixer = nullptr;
	}
	if (mixer_initialized) {
		MIX_Quit();
		mixer_initialized = false;
	}
}
}

Sound::Sound(const std::string &path) : path(path) {
	MIX_Mixer *mixer = acquire_mixer();
	std::filesystem::path resolved = resolve_sound_path(path);

	audio = MIX_LoadAudio(mixer, resolved.string().c_str(), true);
	if (audio == nullptr) {
		error("Failed to load sound " + resolved.string() + ": " +
			SDL_GetError());
	}

	track = MIX_CreateTrack(mixer);
	if (track == nullptr) {
		error(std::string("Failed to create sound track: ") +
			SDL_GetError());
	}
	if (!MIX_SetTrackAudio(track, audio)) {
		error(std::string("Failed to assign sound audio: ") +
			SDL_GetError());
	}

	loaded = true;
	set_volume(volume);
}

Sound::~Sound() { dispose(); }

void Sound::play() {
	if (!loaded || track == nullptr)
		return;

	frame_position = start_position;
	MIX_SetTrackPlaybackPosition(track, frame_position);
	if (!MIX_PlayTrack(track, 0)) {
		error(std::string("Failed to play sound: ") + SDL_GetError());
	}
	playing = true;
}

void Sound::pause() {
	if (track != nullptr)
		MIX_PauseTrack(track);
	playing = false;
}

void Sound::resume() {
	if (!loaded || track == nullptr)
		return;

	MIX_ResumeTrack(track);
	playing = true;
}

void Sound::restart() {
	set_frame_pos(start_position);
	if (playing)
		play();
}

void Sound::set_frame_pos(int position) {
	frame_position = std::max(0, position);
	if (track != nullptr)
		MIX_SetTrackPlaybackPosition(track, frame_position);
}

void Sound::set_volume(float volume) {
	this->volume = std::clamp(volume, 0.0f, 1.0f);
	if (track != nullptr)
		MIX_SetTrackGain(track, this->volume);
}

float Sound::get_volume() const { return volume; }

bool Sound::is_loaded() const { return loaded; }

bool Sound::is_playing() const {
	return track != nullptr && MIX_TrackPlaying(track);
}

void Sound::dispose() {
	if (!loaded && audio == nullptr && track == nullptr)
		return;

	if (track != nullptr) {
		MIX_StopTrack(track, 0);
		MIX_DestroyTrack(track);
		track = nullptr;
	}
	if (audio != nullptr) {
		MIX_DestroyAudio(audio);
		audio = nullptr;
	}

	playing = false;
	loaded = false;
	release_mixer();
}
