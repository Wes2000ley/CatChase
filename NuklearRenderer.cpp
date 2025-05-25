#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#include "NuklearRenderer.h"
#include "resource_manager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <iostream>


NuklearRenderer::NuklearRenderer(GLFWwindow* window)
    : window_(window)
{
    nk_init_default(&ctx_, nullptr);
    ctx_.clip.copy = nullptr;
    ctx_.clip.paste = nullptr;

    nk_buffer_init_default(&cmds_);
    InitResources();
    UploadFont();
}

NuklearRenderer::~NuklearRenderer() {
    nk_buffer_free(&cmds_);
    nk_free(&ctx_);

    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteVertexArrays(1, &vao_);
}

void NuklearRenderer::InitResources() {
    shader_ = ResourceManager::LoadShader(
        "resources/shaders/nuklear.vert",
        "resources/shaders/nuklear.frag",
        nullptr, "nuklear"
    );

    // Setup VAO/VBO/EBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    std::size_t stride = sizeof(float) * 8;

    glEnableVertexAttribArray(0); // pos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); // uv
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 2));
    glEnableVertexAttribArray(2); // color
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*)(sizeof(float) * 4));

    glBindVertexArray(0);
}

void NuklearRenderer::UploadFont() {
    nk_font_atlas_init_default(&atlas_);
    nk_font_atlas_begin(&atlas_);

    struct nk_font* font = nk_font_atlas_add_from_file(&atlas_, "resources/fonts/OCRAEXT.TTF", 18.0f, nullptr);
    int w, h;
    const void* img = nk_font_atlas_bake(&atlas_, &w, &h, NK_FONT_ATLAS_RGBA32);

    fontTexture_ = std::make_shared<Texture2D>();
    fontTexture_->Internal_Format = GL_RGBA;
    fontTexture_->Image_Format = GL_RGBA;
    fontTexture_->Generate(w, h, reinterpret_cast<const unsigned char*>(img));

    nk_font_atlas_end(&atlas_, nk_handle_id(fontTexture_->ID), &nullTex_);
    if (font) nk_style_set_font(&ctx_, &font->handle);
}

void NuklearRenderer::BeginFrame() {
    nk_input_begin(&ctx_);

    double x, y;
    glfwGetCursorPos(window_, &x, &y);
    nk_input_motion(&ctx_, (int)x, (int)y);

    for (int i = 0; i < NK_BUTTON_MAX; i++) {
        nk_input_button(&ctx_, (nk_buttons)i, (int)x, (int)y,
                        glfwGetMouseButton(window_, i) == GLFW_PRESS);
    }

    nk_input_end(&ctx_);
}

void NuklearRenderer::EndFrame() {
    glfwGetFramebufferSize(window_, &width_, &height_);
    fbScaleX_ = 1.0f;
    fbScaleY_ = 1.0f;

    glm::mat4 ortho = glm::ortho(0.0f, (float)width_, (float)height_, 0.0f);

    shader_->Use();
    shader_->SetMatrix4("projection", ortho);
    shader_->SetInteger("Texture", 0);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    const struct nk_draw_command* cmd;
    const nk_draw_index* offset = nullptr;

    struct nk_convert_config config{};
    static const struct nk_draw_vertex_layout_element vertex_layout[] = {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, 0},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, sizeof(float) * 2},
        {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, sizeof(float) * 4},
        {NK_VERTEX_LAYOUT_END}
    };
    config.vertex_layout = vertex_layout;
    config.vertex_size = sizeof(float) * 8;
    config.vertex_alignment = NK_ALIGNOF(float) * 8;
    config.tex_null = nullTex_;
    config.circle_segment_count = 22;
    config.curve_segment_count = 22;
    config.arc_segment_count = 22;
    config.global_alpha = 1.0f;
    config.shape_AA = NK_ANTI_ALIASING_ON;
    config.line_AA = NK_ANTI_ALIASING_ON;

    nk_buffer vbuf, ebuf;
    const int maxVerts = 512 * 1024, maxElems = 128 * 1024;
    glBufferData(GL_ARRAY_BUFFER, maxVerts, nullptr, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxElems, nullptr, GL_STREAM_DRAW);

    void* vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    void* elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    nk_buffer_init_fixed(&vbuf, vertices, maxVerts);
    nk_buffer_init_fixed(&ebuf, elements, maxElems);

    nk_convert(&ctx_, &cmds_, &vbuf, &ebuf, &config);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    nk_draw_foreach(cmd, &ctx_, &cmds_) {
        if (!cmd->elem_count) continue;

        glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
        glScissor(
            (GLint)(cmd->clip_rect.x * fbScaleX_),
            (GLint)((height_ - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * fbScaleY_),
            (GLint)(cmd->clip_rect.w * fbScaleX_),
            (GLint)(cmd->clip_rect.h * fbScaleY_)
        );

        glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
        offset += cmd->elem_count;
    }

    nk_clear(&ctx_);
    glDisable(GL_SCISSOR_TEST);
    glBindVertexArray(0);
}

nk_context* NuklearRenderer::GetContext() {
    return &ctx_;
}
