#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <ui_framework/colors.hpp>
#include <ui_framework/ui_manager.hpp>
#include <ui_framework/panel_node.hpp>
#include <ui_framework/stack_panel_node.hpp>
#include <ui_framework/components/button.hpp>
#include <ui_framework/components/dropdown.hpp>
#include <ui_framework/components/menu_item.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
    std::string getResourcePath(const char *relativePath)
    {
        const char *basePath = SDL_GetBasePath();
        if (!basePath)
            return relativePath;

        return std::string(basePath) + relativePath;
    }

    class CustomSpacingPanel final : public ui::PanelNode
    {
    public:
        void setCustomSpacing(float spacing) noexcept
        {
            if (!std::isfinite(spacing) || spacing < 0.0f || customSpacing_ == spacing)
                return;
            customSpacing_ = spacing;
        }

        float getCustomSpacing() const noexcept { return customSpacing_; }

    protected:
        ui::LayoutSize measure(const ui::MeasureContext &context) const override
        {
            ui::LayoutSize desired{};
            std::size_t visibleChildCount = 0;

            if (!context.measureChild)
                return desired;

            for (std::size_t i = 0; i < getChildCount(); ++i)
            {
                ui::Node *child = getChild(i);
                if (!child || !child->isVisible() || child->getPositionMode() == ui::PositionMode::Absolute)
                    continue;

                ui::LayoutSize childConstraints = context.availableContentSize;
                if (visibleChildCount % 2 == 0)
                    childConstraints.width *= 0.65f;

                const ui::LayoutSize childDesired = context.measureChild(*child, childConstraints);
                desired.width = std::max(desired.width, childDesired.width);
                desired.height += childDesired.height;
                ++visibleChildCount;
            }

            if (visibleChildCount > 1)
                desired.height += customSpacing_ * static_cast<float>(visibleChildCount - 1);

            return desired;
        }

        void arrange(const ui::ArrangeContext &context) override
        {
            if (!context.arrangeChild || !context.desiredSize)
                return;

            float y = context.contentPosition.y;
            std::size_t visibleChildIndex = 0;

            for (std::size_t i = 0; i < getChildCount(); ++i)
            {
                ui::Node *child = getChild(i);
                if (!child || !child->isVisible() || child->getPositionMode() == ui::PositionMode::Absolute)
                    continue;

                const ui::LayoutSize desired = context.desiredSize(*child);
                const float allocatedWidth =
                    visibleChildIndex % 2 == 0
                        ? context.contentSize.width * 0.65f
                        : context.contentSize.width;

                context.arrangeChild(
                    *child,
                    {context.contentPosition.x, y},
                    {allocatedWidth, desired.height});

                y += desired.height + customSpacing_;
                ++visibleChildIndex;
            }
        }

    private:
        float customSpacing_ = 2.0f;
    };
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    constexpr int logicalWidth = 320;
    constexpr int logicalHeight = 180;
    constexpr float headerHeight = 24.0f;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return -1;
    if (!TTF_Init())
    {
        SDL_Quit();
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("ChessClient", logicalWidth * 3, logicalHeight * 3,
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window)
    {
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer)
    {
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_SetRenderLogicalPresentation(renderer, logicalWidth, logicalHeight, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const std::string fontPath = getResourcePath("fonts/Roboto-Medium.ttf");
    TTF_Font *font = TTF_OpenFont(fontPath.c_str(), 8.0f);
    TTF_Font *largeFont = TTF_OpenFont(fontPath.c_str(), 12.0f);
    if (!font || !largeFont)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "Failed to load font(s): %s (%s)",
                     fontPath.c_str(),
                     SDL_GetError());
        if (largeFont)
            TTF_CloseFont(largeFont);
        if (font)
            TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    {
        ui::UIManager uiManager;

        constexpr std::array<const char *, 4> buttonLabels{"Play", "Openings", "Academy", "Statistics"};
        constexpr float buttonWidth = 52.0f;
        constexpr float buttonGap = 3.0f;
        float x = 8.0f;

        for (size_t i = 0; i < buttonLabels.size(); ++i)
        {
            const char *label = buttonLabels[i];
            auto button = std::make_unique<ui::Button>();
            button->setText(label);
            button->setFont(i == buttonLabels.size() - 1 ? largeFont : font);
            button->setTextColor(ui::Colors::white);
            button->setBackgroundColor(ui::Colors::transparent);
            button->setBorderColor(ui::Colors::transparent);
            button->setVariant(ui::Button::Variant::TEXT);
            button->setPosition({x, 2.0f});
            button->setSize(ui::LayoutSizeValue::fixed(buttonWidth, 20.0f));
            uiManager.addRoot(std::move(button));
            x += buttonWidth + buttonGap;
        }

        auto customPanel = std::make_unique<CustomSpacingPanel>();
        CustomSpacingPanel *customPanelPtr = customPanel.get();
        customPanel->setSize(ui::LayoutSizeValue::fixed(120.0f, 82.0f));
        customPanel->setCustomSpacing(2.0f);

        for (const char *label : {"Custom A", "Custom B", "Custom C"})
        {
            auto child = std::make_unique<ui::Button>();
            child->setText(label);
            child->setFont(font);
            child->setTextColor(ui::Colors::white);
            child->setBackgroundColor(ui::Colors::gray);
            child->setBorderColor(ui::Colors::gray);
            child->setVariant(ui::Button::Variant::FILLED);
            customPanel->addChild(std::move(child), customPanel->getChildCount());
        }

        auto customLayoutRoot = std::make_unique<ui::StackPanelNode>(ui::StackPanelNode::Orientation::Vertical);
        customLayoutRoot->setPosition({8.0f, 38.0f});
        customLayoutRoot->setSize(ui::LayoutSizeValue::fixed(120.0f, 82.0f));
        customLayoutRoot->addChild(std::move(customPanel), 0);

        ui::Button *spacingButton = new ui::Button();
        std::unique_ptr<ui::Button> spacingButtonOwner(spacingButton);
        spacingButton->setText("Grow gap");
        spacingButton->setFont(font);
        spacingButton->setTextColor(ui::Colors::white);
        spacingButton->setBackgroundColor(ui::Colors::gray);
        spacingButton->setBorderColor(ui::Colors::gray);
        spacingButton->setVariant(ui::Button::Variant::FILLED);
        spacingButton->setPosition({140.0f, 38.0f});
        spacingButton->setSize(ui::LayoutSizeValue::fixed(70.0f, 20.0f));
        spacingButton->setOnActivate(
            [customPanelPtr, &uiManager](ui::Button &)
            {
                customPanelPtr->setCustomSpacing(customPanelPtr->getCustomSpacing() + 3.0f);
                uiManager.invalidateLayout(*customPanelPtr);
            });

        uiManager.addRoot(std::move(customLayoutRoot));
        uiManager.addRoot(std::move(spacingButtonOwner));

        auto dropdown = std::make_unique<ui::Dropdown>();
        dropdown->setPosition({239.0f, 2.0f});
        dropdown->setSize(ui::LayoutSizeValue::fixed(73.0f, 20.0f));
        dropdown->getTrigger().setFont(font);
        dropdown->getTrigger().setTextColor(ui::Colors::white);
        dropdown->getTrigger().setBackgroundColor(ui::Colors::gray);
        dropdown->getTrigger().setBorderColor(ui::Colors::gray);
        dropdown->getTrigger().setVariant(ui::Button::Variant::FILLED);
        dropdown->setPlaceholder("More");

        constexpr std::array<const char *, 3> dropdownItems{"Settings", "About", "Quit"};
        for (const char *label : dropdownItems)
        {
            auto item = std::make_unique<ui::MenuItem>();
            item->setText(label);
            item->setFont(font);
            item->setTextColor(ui::Colors::white);
            item->setBackgroundColor(ui::Colors::gray);
            dropdown->addItem(std::move(item));
        }

        dropdown->setOnSelectionChanged(
            [](ui::Dropdown &, ui::MenuItem &item)
            {
                SDL_Log("Selected menu item: %s", item.getText().c_str());
            });

        uiManager.addRoot(std::move(dropdown));

        bool running = true;
        while (running)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT)
                    running = false;
                uiManager.processEvent(event, renderer);
            }

            SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
            SDL_RenderClear(renderer);

            SDL_FRect header{0.0f, 0.0f, static_cast<float>(logicalWidth), headerHeight};
            SDL_SetRenderDrawColor(renderer, 24, 24, 24, 255);
            SDL_RenderFillRect(renderer, &header);

            uiManager.runFrame(1.0f / 60.0f, renderer);
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }
    }

    TTF_CloseFont(largeFont);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
