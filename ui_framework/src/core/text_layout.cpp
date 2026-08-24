#include "text_layout.hpp"

#include <algorithm>
#include <cmath>

namespace ui
{
    namespace
    {
        class MeasureFont final
        {
        public:
            explicit MeasureFont(TTF_Font *source, float requestedSize, float requestedLineHeight) noexcept
            {
                font_ = source;
                if (!source)
                    return;

                const float sourceSize = TTF_GetFontSize(source);
                const bool needsSize = requestedSize > 0.0f && sourceSize > 0.0f && std::abs(sourceSize - requestedSize) >= 0.001f;
                const int nativeLineSkip = TTF_GetFontLineSkip(source);
                const bool needsLineSkip = requestedLineHeight > 0.0f && std::abs(static_cast<float>(nativeLineSkip) - requestedLineHeight) >= 0.001f;

                if (!needsSize && !needsLineSkip)
                    return;

                TTF_Font *copy = TTF_CopyFont(source);
                if (!copy)
                {
                    font_ = nullptr;
                    return;
                }

                if (needsSize && !TTF_SetFontSize(copy, requestedSize))
                {
                    TTF_CloseFont(copy);
                    font_ = nullptr;
                    return;
                }

                if (requestedLineHeight > 0.0f)
                    TTF_SetFontLineSkip(copy, std::max(1, static_cast<int>(std::lround(requestedLineHeight))));

                owned_ = copy;
                font_ = copy;
            }

            ~MeasureFont()
            {
                if (owned_)
                    TTF_CloseFont(owned_);
            }

            TTF_Font *get() const noexcept { return font_; }

            MeasureFont(const MeasureFont &) = delete;
            MeasureFont &operator=(const MeasureFont &) = delete;

        private:
            TTF_Font *font_ = nullptr;
            TTF_Font *owned_ = nullptr;
        };
    }

    LayoutSize TextLayout::measure(float availableWidth) const noexcept
    {
        if (!font_ || text_.empty())
            return {};

        MeasureFont measureFont(font_, fontSize_, lineHeight_);
        TTF_Font *font = measureFont.get();
        if (!font)
            return {};

        int width = 0;
        int height = 0;

        if (wrapMode_ == WrapMode::WRAP && availableWidth > 0.0f)
        {
            const int wrapWidth = std::max(1, static_cast<int>(std::floor(availableWidth)));
            if (!TTF_GetStringSizeWrapped(font, text_.c_str(), 0, wrapWidth, &width, &height))
                return {};
        }
        else if (!TTF_GetStringSize(font, text_.c_str(), 0, &width, &height))
        {
            return {};
        }

        return {
            static_cast<float>(std::max(width, 0)),
            static_cast<float>(std::max(height, 0))
        };
    }
}
