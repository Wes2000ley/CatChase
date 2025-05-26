// ──────────────────────────────────────────────────────────────────────────────
// NineSliceRenderer.cpp – final, HasUniform-free variant
// ──────────────────────────────────────────────────────────────────────────────
#include "../../NineSliceRenderer.h"

#include <array>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

// ── ctor / dtor ───────────────────────────────────────────────────────────────
NineSliceRenderer::NineSliceRenderer(std::shared_ptr<Shader> shader)
    : shader_(std::move(shader)) {
    initRenderData();
}

NineSliceRenderer::~NineSliceRenderer() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}


// ── public API ────────────────────────────────────────────────────────────────
void NineSliceRenderer::SetTexture(std::shared_ptr<Texture2D> texture,
                                   int border_pixels) {
    texture_ = std::move(texture);
    border_  = std::max(0, border_pixels);

    if (!texture_) return;

    // Match Texture2D defaults; clamp to edge to avoid bleeding.
    glBindTexture(GL_TEXTURE_2D, texture_->ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void NineSliceRenderer::Render(float x, float y, float w, float h,
                               const glm::mat4& projection,
                               const glm::vec4& tint) {
    if (!shader_ || !texture_ || w <= 0.0f || h <= 0.0f) return;

    const float tex_w = static_cast<float>(texture_->Width);
    const float tex_h = static_cast<float>(texture_->Height);
    float       b     = static_cast<float>(border_);

    // Clamp border so we never end up with negative-size slices.
    b = std::max(0.0f, std::min(b, std::min(w, h) * 0.5f));

    struct Quad { float px, py, pw, ph, tx, ty, tw, th; };
    std::array<Quad, 9> q{{
        // ─ top row ─
        {x,       y,       b,       b,        0,         0,          b,        b},
        {x + b,   y,       w - 2*b, b,        b,         0,          tex_w - 2*b, b},
        {x + w-b, y,       b,       b,        tex_w - b, 0,          b,        b},
        // ─ mid row ─
        {x,       y + b,   b,       h - 2*b,  0,         b,          b,        tex_h - 2*b},
        {x + b,   y + b,   w - 2*b, h - 2*b,  b,         b,          tex_w - 2*b, tex_h - 2*b},
        {x + w-b, y + b,   b,       h - 2*b,  tex_w - b, b,          b,        tex_h - 2*b},
        // ─ bot row ─
        {x,       y + h-b, b,       b,        0,         tex_h - b,  b,        b},
        {x + b,   y + h-b, w - 2*b, b,        b,         tex_h - b,  tex_w - 2*b, b},
        {x + w-b, y + h-b, b,       b,        tex_w - b, tex_h - b,  b,        b},
    }};

    // Enable blending if it wasn't already.
    GLboolean prevBlend = glIsEnabled(GL_BLEND);
    if (!prevBlend) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    shader_->Use();
    shader_->SetMatrix4("projection", projection, false);
    shader_->SetMatrix4("model", glm::mat4(1.0f), false);
    shader_->SetInteger("image", 0, false);
    auto setVec4 = [&](const char* name, const glm::vec4& v) {
        GLint loc = glGetUniformLocation(shader_->ID, name);
        if (loc != -1) glUniform4fv(loc, 1, glm::value_ptr(v));
    };

    setVec4("spriteColor", glm::vec4(1.0f));
    setVec4("tintColor",   glm::vec4(1.0f));

    // Set tint if the shader happens to declare it.
    GLint tintLoc = glGetUniformLocation(shader_->ID, "tintColor");
    if (tintLoc != -1) glUniform4fv(tintLoc, 1, glm::value_ptr(tint));

    glActiveTexture(GL_TEXTURE0);
    texture_->Bind();
    glBindVertexArray(VAO);

    for (const Quad& s : q) {
        float v[6][4] = {
            {s.px,           s.py + s.ph, s.tx            / tex_w, (s.ty + s.th) / tex_h},
            {s.px + s.pw,    s.py,        (s.tx + s.tw)   / tex_w,  s.ty          / tex_h},
            {s.px,           s.py,        s.tx            / tex_w,  s.ty          / tex_h},

            {s.px,           s.py + s.ph, s.tx            / tex_w, (s.ty + s.th) / tex_h},
            {s.px + s.pw,    s.py + s.ph,(s.tx + s.tw)    / tex_w, (s.ty + s.th) / tex_h},
            {s.px + s.pw,    s.py,        (s.tx + s.tw)   / tex_w,  s.ty          / tex_h},
        };
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(v), v);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    if (!prevBlend) glDisable(GL_BLEND);
}

// ── private helpers ───────────────────────────────────────────────────────────
void NineSliceRenderer::initRenderData() {
    constexpr GLsizeiptr bytes = sizeof(float) * 6 * 4; // one quad

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, bytes, nullptr, GL_DYNAMIC_DRAW);

    // pos (vec2) – attribute 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), reinterpret_cast<void*>(0));
    // uv  (vec2) – attribute 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void NineSliceRenderer::SetDepthTest(bool enable)
{
    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}
