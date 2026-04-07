#include "engine/render_context.h"

#include "engine/framebuffer.h"

RenderContext gen_render_context(FrameBuffer *frame_buffer, Texture *texture,
                                 Shader *shader, Vector &position, int slot,
                                 float rotation, float width, float height,
                                 float z_order) {
        RenderContext result;

        result.frame_buffer = frame_buffer;
        result.render_type = RT_TEXTURE;
        result.texture = texture;
        result.shader = shader;
        result.position = position;
        result.rotation = rotation;
        result.width = width;
        result.height = height;
        result.z_order = z_order;
        result.slot = slot;

        return result;
}