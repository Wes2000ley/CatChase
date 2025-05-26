//
// Created by Skyri on 5/25/2025.
//

#ifndef UTILITY_H
#define UTILITY_H
#include <nuklear.h>

#include "TEXT_RENDERER.h"

static glm::vec2 CalculateCenteredTextPosition(
    TextRenderer& renderer,
    const std::string& text,
    float scale,
    const struct nk_rect& boundingBox,
    float fallbackTextWidthForEmpty) // Add fallback width for measure when text is empty
{
    if (text.empty()) { // Handle empty string case to avoid issues with MeasureRenderedTextBounds
        float renderX = boundingBox.x + (boundingBox.w - fallbackTextWidthForEmpty) * 0.5f;
        float renderY = boundingBox.y + boundingBox.h * 0.5f; // Crude center for empty
        return {renderX, renderY};
    }

    // Measure text using (0,0) as reference to get relative metrics.
    glm::vec4 textMetrics = renderer.MeasureRenderedTextBounds(text, 0.0f, 0.0f, scale);
    float actualTextWidth = textMetrics.z;
    float actualTextHeight = textMetrics.w;

    // textMetrics.x is the offset from the provided 'x' (0 in this case) to the text's actual left.
    // textMetrics.y is the offset from the provided 'y' (0 in this case) to the text's actual top.

    float renderX = boundingBox.x + (boundingBox.w - actualTextWidth) * 0.5f - textMetrics.x;
    float renderY = boundingBox.y + (boundingBox.h - actualTextHeight) * 0.5f - textMetrics.y;

    // Safety for problematic text measurements (e.g., font not fully loaded yet, or all chars missing)
    if (actualTextHeight <= 0.001f && actualTextWidth <= 0.001f && !text.empty()) {
        // Try with simple MeasureTextWidth if bounds are zero but text is not empty
        actualTextWidth = renderer.MeasureTextWidth(text, scale);
        renderX = boundingBox.x + (boundingBox.w - actualTextWidth) * 0.5f;
        // For Y, could attempt to use FontSize or a fraction of boundingBox.h
        // This part is tricky without knowing a reliable text height fallback.
        // For now, let's assume MeasureRenderedTextBounds gives *some* height or it truly is problematic.
        // If textMetrics.w was 0, the original renderY calculation using it would be fine.
        // The issue is if textMetrics.y is also extreme.
        // A simpler fallback for Y if actualTextHeight is 0:
        renderY = boundingBox.y + boundingBox.h * 0.5f - (renderer.FontSize * scale * 0.5f); // Approx center based on FontSize
    } else if (actualTextHeight <= 0.001f && !text.empty()) {
        // Only height is an issue
        renderY = boundingBox.y + boundingBox.h * 0.5f - (renderer.FontSize * scale * 0.5f);
    }


    return {renderX, renderY};
}








#endif //UTILITY_H
