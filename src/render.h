#ifndef _render_h
#define _render_h
#include "linux_platform_layer.hpp"
//#include <ft2build.h>
//#include FT_FREETYPE_H

#pragma pack(push, 1)
struct bitmap_header
{
    uint16 fileType;
    uint32 fileSize;
    uint16 reserved1;
    uint16 reserved2;
    uint32 bitmapOffset;
    uint32 size;
    int32 width;
    int32 height;
    uint16 planes;
    uint16 bitsPerPixel;
    /*
    uint32 compression;
    uint32 sizeOfBitmap;
    int32 horzResolution;
    int32 vertResolution;
    uint32 colorsUsed;
    uint32 colorsImportant;

    uint32 redMask;
    uint32 greenMask;
    uint32 blueMask;
    */
};
#pragma pack(pop)

struct memory_arena;
struct AABB;

namespace Renderer
{
global int32 g_screenX = 1920;
global int32 g_screenY = 1200;
global real32 g_spacingX = 32.0f;
global real32 g_spacingY = 32.0f;

struct vertex_array_data
{
    GLuint vao;
    real32 *data;
    int64 size;
};

struct VertexAttributes
{
    GLint position;
    GLint color;
    GLint texture_coords;
};

global int32 g_default_verts_pos_stride = 6;
global int32 g_default_verts_color_stride = 5;
global int32 g_default_verts_tex_coord_stride = 6;
#define G_DEFAULT_VERT_COUNT 28
#define G_DEFAULT_QUAD_FLOATS_PER_VERTICE 7

real32 g_default_verts[G_DEFAULT_VERT_COUNT] = {
    -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f,  1.0f, 1.0f, 1.0f, 0.0f, 1.0f};

struct color3
{
    union
    {
        struct
        {
            real32 r, g, b;
        };
        real32 v[3];
    };
};

global GLuint g_lineVAO;

global GLuint g_vertex_shader;
global GLuint g_fragment_shader;
global GLuint g_shader_program;
global GLuint g_element_array_buffer;
global GLuint g_vertex_array_buffer;
global GLuint g_vertex_buffer;

global GLuint elements[] = {0, 1, 2, 2, 3, 0};
global VertexAttributes g_default_vert_attributes;
global GLint g_uniFrame;
global glm::mat4 g_model;
global GLint g_uniModel;
global glm::vec3 g_view_pos;
global glm::mat4 g_view;
global GLint g_uniView;
global glm::mat4 g_projection;
global GLint g_uniProjection;

// NOTE:Temporary rectangle to use for debugging
global real32 d_square_dimension = 8.0f;
global GLuint d_square_vertex_buffer;
real32 d_square_verts[] = {
    /*  ------------   Position  ----------- |  --- Color ---  | TexCoords */
    -d_square_dimension, -d_square_dimension, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, // Top-left
    d_square_dimension, -d_square_dimension, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // Top-right
    d_square_dimension, d_square_dimension, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,   // Bottom-left
    -d_square_dimension, d_square_dimension, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f   // Bottom-right
};

internal bool32 gl_start_new_log();
internal bool32 gl_log(bool32 error, const char *message, ...);
internal void gl_log_params();
internal void gl_log_exentions();

internal bool32 gl_print_if_error(char *message);
internal bool32 initialize_opengl(GLFWwindow *window);

internal char *gl_shader_to_string(char *filename);

internal uint32 gl_load_bitmap_texture(bitmap_header *header, uint8 *pixels);

internal void gl_create_quad(vertex_array_data *v, color3 color, AABB bounds);
internal void gl_create_quad(memory_arena *memory, vertex_array_data *v, color3 color, AABB bounds);
inline void gl_set_quad_tex_coords_to_default(real32 *verts, int32 startingIndex, int32 stride);
internal void gl_set_quad_scale(real32 *verts, int32 stride, real32 _scale);
internal void gl_set_quad_color(real32 *verts, int32 startingIndex, int32 stride, color3 color);
inline void gl_set_quad(real32 *verts, int32 stride, vector2 topLeft, vector2 bottomRight);
inline void gl_set_quad(real32 *verts,
                        int32 stride,
                        vector2 topLeft,
                        vector2 topRight,
                        vector2 bottomLeft,
                        vector2 bottomRight);
internal void gl_scale_quad(real32 *verts, int32 stride, real32 scale);

internal bool32 gl_load_array_buffer(GLuint buffer, real32 *data, int32 dataCount);
internal void gl_load_array_buffer(vertex_array_data *v);
internal bool32 gl_gen_array_buffer(GLuint *buffer, real32 *data, int32 dataCount);

internal void gl_apply_texture_settings(GLuint textureID);
internal void gl_apply_default_attributes(VertexAttributes *vertAttribs);

internal void
gl_draw_lines(vertex_array_data *v, glm::vec3 pos, GLenum mode, GLint uniModel, GLint uniFrame);
internal void gl_draw_lines(vertex_array_data *v, GLenum mode, GLint uniModel, GLint uniFrame);

internal void gl_draw_textured(GLuint VAO,
                               glm::vec3 position,
                               GLuint texture,
                               GLint uniModel,
                               glm::mat4 model,
                               real32 rotation,
                               bool32 alphaBlend);
internal void
gl_draw_absolute(GLuint VAO, GLuint texture, GLint uniModel, GLint uniFrame, bool32 alphaBlend);

internal void gl_draw(GLFWwindow *window);
internal void gl_cleanup();

internal bool32 gl_print_if_error(char *message);
}

#endif
