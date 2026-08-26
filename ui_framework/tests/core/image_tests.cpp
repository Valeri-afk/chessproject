#include "ui_framework/components/image.hpp"

#include <cassert>

namespace
{
    void testDefaultState()
    {
        ui::Image image;
        assert(image.getTexture() == nullptr);
        assert(image.getIntrinsicSize() == ui::LayoutSize{});
        assert(image.getFitMode() == ui::Image::FitMode::CONTAIN);
        assert(image.getTint() == ui::Colors::white);
    }

    void testIntrinsicSizeAndFitMode()
    {
        ui::Image image;
        image.setIntrinsicSize({128.0f, 64.0f});
        assert(image.getIntrinsicSize() == ui::LayoutSize{128.0f, 64.0f});

        image.setFitMode(ui::Image::FitMode::STRETCH);
        assert(image.getFitMode() == ui::Image::FitMode::STRETCH);

        image.setFitMode(ui::Image::FitMode::COVER);
        assert(image.getFitMode() == ui::Image::FitMode::COVER);

        image.setFitMode(ui::Image::FitMode::CONTAIN);
        assert(image.getFitMode() == ui::Image::FitMode::CONTAIN);
    }

    void testTint()
    {
        ui::Image image;
        const ui::Color tint{10, 20, 30, 40};
        image.setTint(tint);
        assert(image.getTint() == tint);
    }

    void testIntrinsicMeasureIsIndependentFromFitMode()
    {
        ui::Image image;
        image.setIntrinsicSize({128.0f, 64.0f});

        for (const auto mode : {ui::Image::FitMode::STRETCH,
                                ui::Image::FitMode::CONTAIN,
                                ui::Image::FitMode::COVER})
        {
            image.setFitMode(mode);
            const ui::LayoutSize measured = image.measureContent({1000.0f, 900.0f});
            assert(measured == ui::LayoutSize{128.0f, 64.0f});
        }
    }
}

int main()
{
    testDefaultState();
    testIntrinsicSizeAndFitMode();
    testTint();
    testIntrinsicMeasureIsIndependentFromFitMode();
    return 0;
}
