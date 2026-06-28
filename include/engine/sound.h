#pragma once

#include <string>

class Sound {
      public:
        explicit Sound(const std::string &path);
        ~Sound();

        void play();
        void pause();
        void resume();
        void restart();
        void set_frame_pos(int position);
        void set_volume(float volume);
        float get_volume() const;
        void dispose();

      private:
        std::string path;
        float volume = 1.0f;
        int start_position = 0;
        int frame_position = 0;
        bool loaded = false;
        bool playing = false;
};
