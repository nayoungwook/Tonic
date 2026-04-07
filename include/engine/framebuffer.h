#pragma once

class FrameBuffer {
      public:
        FrameBuffer(int width, int height);
        ~FrameBuffer();

        void bind();
        void unbind();

        unsigned get_color_texture() const { return color_texture; }
        unsigned get_fbo() const { return fbo; }

        void create(int width, int height);
        void upload_texture(int tex);

      private:
        unsigned fbo = 0;
        unsigned color_texture = 0;
        unsigned rbo = 0;

        int width = 0;
        int height = 0;
};