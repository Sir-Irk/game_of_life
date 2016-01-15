#include "linux_platform_layer.hpp"
#include "render.h"
#include "game.h"
//#include "AABB.h"
#include <time.h>

using namespace Renderer;

#define GL_LOG_FILE "gl.log"

internal bool32
handle_glGetError()
{
    bool32 noError = true;
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        switch (errorCode)
        {
            case GL_INVALID_ENUM:
            {
                gl_log(true, "ERROR from OpenGL: Invalid enum\n");
                noError = false;
            }
            break;
            case GL_INVALID_VALUE:
            {
                gl_log(true, "ERROR from OpenGL: Invalid value\n");
                noError = false;
            }
            break;
            case GL_INVALID_OPERATION:
            {
                gl_log(true, "ERROR from OpenGL: Invalid operation\n");
                noError = false;
            }
            break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
            {
                gl_log(true, "ERROR from OpenGL: Invalid frame-buffer operation\n");
                noError = false;
            }
            break;
            case GL_OUT_OF_MEMORY:
            {
                gl_log(true, "ERROR from OpenGL: Out of memory\n");
                noError = false;
            }
            break;
            default:
            {
                gl_log(true, "ERROR from OpenGL: Unkown Error\n");
                noError = false;
            }
            break;
        }
    }
    return noError;
}

internal bool32
Renderer::gl_start_new_log()
{
    FILE *file = fopen(GL_LOG_FILE, "w");
    if (!file)
    {
        fprintf(stderr, "ERROR: could not open GL_LOG_FILE log file %s for writing\n", GL_LOG_FILE);
        return false;
    }

    time_t now = time(NULL);
    char *date = ctime(&now);
    fprintf(file, "GL_LOG_FILE log. local time %s\n", date);
    fclose(file);
    return true;
}

internal bool32
Renderer::gl_log(bool32 error, const char *message, ...)
{
    va_list argptr;
    va_start(argptr, message);
    log_to_file(GL_LOG_FILE, message, argptr);
    va_end(argptr);
    if (error)
    {
        va_start(argptr, message);
        log_to_console(GL_LOG_FILE, message, argptr);
        va_end(argptr);
    }
    return true;
}

internal void
Renderer::gl_log_params()
{
    const int32 paramCount = 11;
    GLenum params[] = {
        GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
        GL_MAX_CUBE_MAP_TEXTURE_SIZE,
        GL_MAX_DRAW_BUFFERS,
        GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
        GL_MAX_TEXTURE_IMAGE_UNITS,
        GL_MAX_TEXTURE_SIZE,
        GL_MAX_VERTEX_ATTRIBS,
        GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
        GL_MAX_VERTEX_UNIFORM_COMPONENTS,
        GL_MAX_VIEWPORT_DIMS,
        GL_STEREO,
    };

    const char *names[] = {
        "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
        "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
        "GL_MAX_DRAW_BUFFERS",
        "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
        "GL_MAX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_TEXTURE_SIZE",
        "GL_MAX_VERTEX_ATTRIBS",
        "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
        "GL_MAX_VIEWPORT_DIMS",
        "GL_STEREO",
    };

    gl_log(false, "\n---------------------------------------\n");
    gl_log(false, "GL Context Params:\n");
    char msg[256];
    // NOTE: only works if the order is 0-10 integer return types
    for (int32 i = 0; i < paramCount - 2; ++i)
    {
        int32 v = 0;
        glGetIntegerv(params[i], &v);
        if (gl_print_if_error(NULL))
        {
            gl_log(true, "Failed to get integerv for param %d\n", i + 1);
            return;
        }
        gl_log(false, "%s %i\n", names[i], v);
    }

    int v[2];
    v[0] = v[1] = 0;
    glGetIntegerv(params[paramCount - 2], v);
    if (gl_print_if_error("Failed to get integerv for param 10\n")) return;
    gl_log(false, "%s %i %i\n", names[10], v[0], v[1]);
    unsigned char s = 0;
    glGetBooleanv(params[paramCount - 1], &s);
    if (gl_print_if_error("Failed to get booleanv for param 11\n")) return;
    gl_log(false, "%s %u\n", names[paramCount - 1], (uint32)s);
    gl_log(false, "---------------------------------------\n");
}

internal void
Renderer::gl_log_exentions()
{
    // handle_glGetError();
    FILE *file = fopen("gl_extensions.log", "w");
    if (!file)
    {
        fprintf(stderr, "Failed to open filed for glExtension logging\n");
        return;
    }

    const unsigned char *extensions = glGetStringi(GL_EXTENSIONS, 0);
    if (gl_print_if_error("getString failed")) return;
    if (!extensions)
    {
        gl_log(true, "ERROR: no extensions to log\n");
        fclose(file);
        return;
    }
    int32 extLength = 0;
    for (; *(extensions + extLength) != '\0'; extLength++)
        ;
    extLength += 1;

    unsigned char *readable_extensions = (unsigned char *)malloc(extLength);
    if (!readable_extensions)
    {
        gl_log(true, "ERROR: failed to format gl_extensions log: log still printed but is hideous\n");
        fprintf(file, "Extensions:\n %s", extensions);
        fclose(file);
        return;
    }

    for (int32 i = 0; i < extLength; i++)
    {
        if (*(extensions + i) == ' ')
        {
            *(readable_extensions + i) = '\n';
            continue;
        }
        *(readable_extensions + i) = *(extensions + i);
    }
    *(readable_extensions + extLength - 1) = '\0';

    fprintf(file, "Extensions:\n %s", readable_extensions);
    fclose(file);
    free(readable_extensions);
}

internal bool32
Renderer::gl_print_if_error(char *message)
{
    // NOTE: handle_glGetError returns true if no errors were found
    if (handle_glGetError()) return false;
    gl_log(true, "OpenGL Error: %s", (message ? message : "no message"));
    return true;
}

internal char *
Renderer::gl_shader_to_string(char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        gl_log(true, "ERROR: Shader file not found: %s\n", filename);
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    size_t size = ftell(file) + 1;
    char *result = (char *)malloc(size);
    if (result)
    {
        rewind(file);
        fread(result, size, 1, file);
        result[size - 1] = '\0';
    }

    return result;
}

internal bool32
Renderer::gl_load_array_buffer(GLuint buffer, real32 *data, int32 dataCount)
{
    glGetError();
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(real32) * dataCount, data, GL_STATIC_DRAW);
    if (gl_print_if_error("Failed load data into vertex array buffer\n")) return false;
    return true;
}

internal void
Renderer::gl_load_array_buffer(vertex_array_data *v)
{
    gl_load_array_buffer(v->vao, v->data, v->size);
}

internal bool32
Renderer::gl_gen_array_buffer(GLuint *buffer, real32 *data, int32 dataCount)
{
    glGenBuffers(1, buffer);
    glBindBuffer(GL_ARRAY_BUFFER, *buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(real32) * dataCount, data, GL_STATIC_DRAW);
    if (gl_print_if_error("Failed to generate and fill vertex array buffer\n")) return false;
    return true;
}

internal bool32
Renderer::initialize_opengl(GLFWwindow *window)
{
    if (!start_new_log(GL_LOG_FILE)) fprintf(stderr, "Opengl: Failed to open new log\n");

    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError)
    {
        gl_log(true, "glewinit() failed: %s\n", glewGetErrorString(glewError));
        return 0;
    }

    GLenum errorCode = glGetError();
    if (errorCode != GL_INVALID_ENUM && errorCode != GL_NO_ERROR)
    {
        gl_log(true, "Open GL: GLEW failed to initialize\n");
        return 0;
    }

    gl_log_params();
    gl_log_exentions();

    glViewport(0, 0, g_screenX, g_screenY);
    if (gl_print_if_error("Failed to set initial viewport\n")) return 0;
    glGenVertexArrays(1, &g_vertex_array_buffer);
    if (gl_print_if_error("Failed to generate global vao\n")) return 0;
    glBindVertexArray(g_vertex_array_buffer);
    if (gl_print_if_error("Failed to bind initial vertex array buffer\n")) return 0;

    // NOTE:Global Default Array and Element buffers
    glGenBuffers(1, &g_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, g_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_default_verts), g_default_verts, GL_STATIC_DRAW);
    if (gl_print_if_error("OpenGL Error: Failed to intialize global vertex array buffer")) return 0;

    glGenBuffers(1, &g_element_array_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_element_array_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
    if (gl_print_if_error("OpenGL Error: Failed to intialize global element array buffer")) return 0;

    // NOTE:Debug Square Vertex Array Buffer.
    glGenBuffers(1, &d_square_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, d_square_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(d_square_verts), d_square_verts, GL_STATIC_DRAW);
    if (glGetError() != GL_NO_ERROR)
    {
        gl_log(true, "OpenGL Error: Failed to intialize debug vertex array buffer\n");
        return 0;
    }

    // NOTE Vertex shader
    GLint status;
    g_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    char *vShaderSrc = gl_shader_to_string("basic_sprite_v.glsl");
    if (vShaderSrc)
    {
        glShaderSource(g_vertex_shader, 1, &vShaderSrc, NULL);
        glCompileShader(g_vertex_shader);
        glGetShaderiv(g_vertex_shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            gl_log(true, "failed to compile vertex shader\n");
        }
        if (gl_print_if_error("failed to initialize vertex shader\n")) return 0;
    }
    else
    {
        gl_log(true, "ERROR: Failed to load shader from file: %s\n", vShaderSrc);
    }

    // NOTE:Fragment shader
    g_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    char *fShaderSrc = gl_shader_to_string("basic_sprite_f.glsl");
    if (fShaderSrc)
    {
        glShaderSource(g_fragment_shader, 1, &fShaderSrc, NULL);
        glCompileShader(g_fragment_shader);
        glGetShaderiv(g_fragment_shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
        {
            gl_log(true, "failed to compile fragment shader\n");
            return 0;
        }
        if (gl_print_if_error("failed to initialize fragment shader\n")) return 0;
    }
    else
    {
        gl_log(true, "ERROR: Failed to load shader from file: %s\n", fShaderSrc);
    }

    // NOTE:Link the shaders into a program
    g_shader_program = glCreateProgram();
    glAttachShader(g_shader_program, g_vertex_shader);
    glAttachShader(g_shader_program, g_fragment_shader);
    glLinkProgram(g_shader_program);
    glUseProgram(g_shader_program);

    status = GL_TRUE;
    glGetProgramiv(g_shader_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) gl_log(true, "OpenGL: problem linking shader program\n");

    if (gl_print_if_error("global shader failed to compile")) return 0;

    glEnableVertexAttribArray(0);
    if (gl_print_if_error("Opengl: failed to set g_default_vert_attributes: position\n")) return 0;

    glEnableVertexAttribArray(1);
    if (gl_print_if_error("Opengl: failed to set g_default_vert_attributes: color\n")) return 0;

    glEnableVertexAttribArray(2);
    if (gl_print_if_error("Opengl: failed to set g_default_vert_attributes: texture-coordinates\n"))
        return 0;

    gl_apply_default_attributes(&g_default_vert_attributes);
    if (gl_print_if_error("Opengl: failed to load texture coordinates\n")) return 0;

    g_uniFrame = glGetUniformLocation(g_shader_program, "spriteFrame");

    g_model = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));
    g_uniModel = glGetUniformLocation(g_shader_program, "g_model");
    glUniformMatrix4fv(g_uniModel, 1, GL_FALSE, glm::value_ptr(g_model));

    g_view = glm::translate(g_view, glm::vec3(-1.0f, -1.0f, 0.0f));
    g_uniView = glGetUniformLocation(g_shader_program, "g_view");
    glUniformMatrix4fv(g_uniView, 1, GL_FALSE, glm::value_ptr(g_view));

    g_projection = glm::ortho(0.0f, 1920.0f, 1200.0f, 0.0f, 1.0f, -1.0f);
    g_uniProjection = glGetUniformLocation(g_shader_program, "g_projection");
    glUniformMatrix4fv(g_uniProjection, 1, GL_FALSE, glm::value_ptr(g_projection));

    glCullFace(GL_BACK);
    if (gl_print_if_error("Opengl: failed to complete end of opengl_initialization.\n")) return 0;
    gl_log(false, "\nOpengl initialized successfully\n");

    return 1;
}

// NOTE: Returns a texture ID
// TODO: Check for compatibility with older versions of Opengl(3.x and lower). Works on 4.x
internal uint32
Renderer::gl_load_bitmap_texture(bitmap_header *header, uint8 *pixels)
{
    uint32 result = 0;
    glGetError();
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, header->width, header->height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, header->width, header->height, GL_RGBA, GL_UNSIGNED_BYTE,
                    pixels);
    gl_apply_texture_settings(result);
    if (gl_print_if_error("Opengl: Failed to load bitmap to texture")) result = 0;
    return result;
}

internal void
Renderer::gl_create_quad(vertex_array_data *v, color3 color, AABB bounds)
{
    v->size = G_DEFAULT_QUAD_FLOATS_PER_VERTICE * 4;
    real32 *verts = v->data;
    gl_set_quad(verts, Renderer::g_default_verts_pos_stride, bounds.min, bounds.max);
    gl_set_quad_color(verts, 2, Renderer::g_default_verts_color_stride, color);
    gl_set_quad_tex_coords_to_default(verts, 5, Renderer::g_default_verts_tex_coord_stride);
    gl_gen_array_buffer(&v->vao, v->data, v->size);
    gl_print_if_error("Opengl: Failed to create new quad\n");
}

internal void
Renderer::gl_create_quad(memory_arena *memory, vertex_array_data *v, color3 color, AABB bounds)
{
    v->size = G_DEFAULT_QUAD_FLOATS_PER_VERTICE * 4;
    assert(v->data = PushArray(memory, v->size, real32));
    real32 *verts = v->data;
    gl_set_quad(verts, Renderer::g_default_verts_pos_stride, bounds.min, bounds.max);
    gl_set_quad_color(verts, 2, Renderer::g_default_verts_color_stride, color);
    gl_set_quad_tex_coords_to_default(verts, 5, Renderer::g_default_verts_tex_coord_stride);
    gl_gen_array_buffer(&v->vao, v->data, v->size);
    gl_print_if_error("Opengl: Failed to create new quad\n");
}

inline void
Renderer::gl_set_quad(real32 *verts, int32 stride, vector2 topLeft, vector2 bottomRight)
{
    int32 index = 0;
    verts[index++] = topLeft.x;
    verts[index] = topLeft.y;

    index += stride;
    verts[index++] = bottomRight.x;
    verts[index] = topLeft.y;

    index += stride;
    verts[index++] = bottomRight.x;
    verts[index] = bottomRight.y;

    index += stride;
    verts[index++] = topLeft.x;
    verts[index] = bottomRight.y;
}

inline void
Renderer::gl_set_quad(real32 *verts,
                      int32 stride,
                      vector2 topLeft,
                      vector2 topRight,
                      vector2 bottomLeft,
                      vector2 bottomRight)
{
    int32 index = 0;
    verts[index++] = topLeft.x;
    verts[index] = topLeft.y;

    index += stride;
    verts[index++] = topRight.x;
    verts[index] = topRight.y;

    index += stride;
    verts[index++] = bottomRight.x;
    verts[index] = bottomRight.y;

    index += stride;
    verts[index++] = bottomLeft.x;
    verts[index] = bottomLeft.y;
}

inline void
Renderer::gl_set_quad_tex_coords_to_default(real32 *verts, int32 startingIndex, int32 stride)
{
    int32 index = startingIndex;
    // top left
    verts[index++] = 0;
    verts[index] = 0;
    index += stride;
    // top right
    verts[index++] = 1;
    verts[index] = 0;
    index += stride;
    // bottom right
    verts[index++] = 1;
    verts[index] = 1;
    index += stride;
    // bottom left
    verts[index++] = 0;
    verts[index] = 1;
}

// NOTE: Assumes a certain order of vertices.
internal void
Renderer::gl_scale_quad(real32 *verts, int32 stride, real32 _scale)
{
    int32 index = 0;
    real32 scale = AbsReal32(_scale);
    for (int32 i = 0; i < 4; ++i)
    {
        verts[index] *= scale;
        ++index;
        verts[index] *= scale;
        index += stride;
    }
}

internal void
Renderer::gl_set_quad_scale(real32 *verts, int32 stride, real32 _scale)
{
    real32 scale = _scale * 0.5f;
    int32 index = 0;
    for (int32 i = 0; i < 4; ++i)
    {
        verts[index] = (verts[index] > 0) ? scale : -scale;
        ++index;
        verts[index] = (verts[index] > 0) ? scale : -scale;
        index += stride;
    }
}

internal void
Renderer::gl_set_quad_color(real32 *verts, int32 startingIndex, int32 stride, color3 color)
{
    int32 index = startingIndex;
    for (int32 i = 0; i < 4; ++i)
    {
        verts[index++] = color.r;
        verts[index++] = color.g;
        verts[index] = color.b;
        index += stride;
    }
}

internal void
Renderer::gl_apply_texture_settings(GLuint textureID)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl_print_if_error("failed to apply texture settings\n");
}

internal void
Renderer::gl_apply_default_attributes(VertexAttributes *vertAttribs)
{
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void *)(5 * sizeof(GLfloat)));
}

internal void
Renderer::gl_draw_lines(vertex_array_data *v,
                        glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f),
                        GLenum mode = GL_LINES,
                        GLint uniModel = g_uniModel,
                        GLint uniFrame = g_uniFrame)
{
    gl_load_array_buffer(v);
    glBindBuffer(GL_ARRAY_BUFFER, v->vao);
    gl_apply_default_attributes(&g_default_vert_attributes);
    glm::mat4 modelTrans = glm::translate(glm::mat4(), pos);
    modelTrans = glm::rotate(modelTrans, 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(modelTrans));
    glUniform4f(uniFrame, 0.0f / 64.0f, 0.0f / 64.0f, 1.0f, 1.0f);
    glDrawArrays(mode, 0, v->size / 7);
}

internal void
Renderer::gl_draw_absolute(GLuint VAO,
                           GLuint texture,
                           bool32 alphaBlend = true,
                           GLint uniModel = g_uniModel,
                           GLint uniFrame = g_uniFrame)
{
    glBindBuffer(GL_ARRAY_BUFFER, VAO);
    glBindTexture(GL_TEXTURE_2D, texture);
    gl_apply_default_attributes(&g_default_vert_attributes);
    glm::mat4 model = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(uniFrame, 0.0f / 64.0f, 0.0f / 64.0f, 1.0f, 1.0f);

    if (alphaBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

internal void
Renderer::gl_draw_textured(GLuint VAO,
                           glm::vec3 position,
                           GLuint texture,
                           GLint uniModel = g_uniModel,
                           glm::mat4 model = glm::mat4(),
                           real32 rotation = 0.0f,
                           bool32 alphaBlend = true)
{
    glBindBuffer(GL_ARRAY_BUFFER, VAO);
    glBindTexture(GL_TEXTURE_2D, texture);
    gl_apply_default_attributes(&g_default_vert_attributes);
    // TODO: find out why these matrix transforms works even though they seem to be in he wrong order
    model = glm::translate(glm::mat4(), position);
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(g_uniFrame, 0.0f / 64.0f, 0.0f / 64.0f, 1.0f, 1.0f);

    if (alphaBlend)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

internal void
Renderer::gl_draw(GLFWwindow *window)
{
    glUseProgram(g_shader_program);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_element_array_buffer);

    glfwSwapBuffers(window);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

internal void
Renderer::gl_cleanup()
{
    glDeleteProgram(g_shader_program);
    glDeleteShader(g_fragment_shader);
    glDeleteShader(g_vertex_shader);
    glDeleteBuffers(1, &g_element_array_buffer);
    glDeleteBuffers(1, &g_vertex_buffer);
    glfwTerminate();
}
