#include "text_layout.hpp"

#include <algorithm>
#include <cmath>

namespace ui
{
    LayoutSize TextLayout::measure(float availableWidth) const noexcept
    {
        if (!font_ || text_.empty())
            return {};

        int width = 0;
        int height = 0;

        if (availableWidth > 0.0f)
        {
            const int wrapWidth = std::max(1, static_cast<int>(std::floor(availableWidth)));
            if (!TTF_GetStringSizeWrapped(font_, text_.c_str(), 0, wrapWidth, &width, &height))
                return {};
        }
        else if (!TTF_GetStringSize(font_, text_.c_str(), 0, &width, &height))
        {
            return {};
        }

        return {
            static_cast<float>(std::max(width, 0)),
            static_cast<float>(std::max(height, 0))
        };
    }
}
