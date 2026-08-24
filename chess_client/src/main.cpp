#include <memory>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <ui_framework/colors.hpp>
#include <ui_framework/ui_manager.hpp>
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
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    constexpr int logicalWidth = 320;
    constexpr int logicalHeight = 180;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return -1;
    if (!TTF_Init())
    {
        SDL_Quit();
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "ChessClient",
        logicalWidth * 3,
        logicalHeight * 3,
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

    SDL_SetRenderLogicalPresentation(
        renderer,
        logicalWidth,
        logicalHeight,
        SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const std::string fontPath = getResourcePath("fonts/Roboto-Medium.ttf");
    TTF_Font *font = TTF_OpenFont(fontPath.c_str(), 8.0f);
    if (!font)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    {
        ui::UIManager uiManager;

        auto root = std::make_unique<ui::StackPanelNode>(ui::StackPanelNode::Orientation::Vertical);
        root->setPosition({8.0f, 38.0f});
        root->setSize(ui::LayoutSizeValue::fixed(120.0f, 120.0f));
        root->setPadding({4.0f, 4.0f, 4.0f, 4.0f});

        for (const char *label : {"Custom A", "Custom B", "Custom C"})
        {
            auto child = std::make_unique<ui::Button>();
            child->setText(label);
            child->setFont(font);
            child->setTextColor(ui::Colors::white);
            child->setBackgroundColor(ui::Colors::gray);
            child->setBorderColor(ui::Colors::gray);
            child->setVariant(ui::Button::Variant::FILLED);
            root->addChild(std::move(child), root->getChildCount());
        }

        uiManager.addRoot(std::move(root));

        auto dropdown = std::make_unique<ui::Dropdown>();
        dropdown->setPosition({239.0f, 2.0f});
        dropdown->setSize(ui::LayoutSizeValue::fixed(73.0f, 20.0f));
        dropdown->getTrigger().setFont(font);
        dropdown->getTrigger().setTextColor(ui::Colors::white);
        dropdown->getTrigger().setBackgroundColor(ui::Colors::gray);
        dropdown->getTrigger().setBorderColor(ui::Colors::gray);
        dropdown->getTrigger().setVariant(ui::Button::Variant::FILLED);
        dropdown->setPlaceholder("More");

        for (const char *label : {"Settings", "About", "Quit"})
        {
            auto item = std::make_unique<ui::MenuItem>();
            item->setText(label);
            item->setFont(font);
            item->setTextColor(ui::Colors::white);
            item->setBackgroundColor(ui::Colors::gray);
            dropdown->addItem(std::move(item));
        }

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
            uiManager.runFrame(1.0f / 60.0f, renderer);
            SDL_RenderPresent(renderer);
            SDL_Delay(16);
        }
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
