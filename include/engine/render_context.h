#pragma once

#include <glm/glm.hpp>
#include "engine/mathf.h"

class Vector;
class Shader;
class Texture;
class FrameBuffer;

enum RenderType {
        RT_TEXTURE = 0,
};

class RenderContext {
      public:
        enum RenderType render_type;
        FrameBuffer *frame_buffer;
        Texture *texture;
        Shader *shader;
        Vector position;
        int slot;
        float width, height;
        float rotation;
        float z_order;
};

RenderContext gen_render_context(FrameBuffer *frame_buffer, Texture *texture,
                                 Shader *shader, Vector &position, int slot,
                                 float rotation, float width, float height,
                                 float z_order);
