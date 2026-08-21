#include <array>
#include <memory>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <ui_framework/colors.hpp>
#include <ui_framework/ui_manager.hpp>
#include <ui_framework/components/button.hpp>
#include <ui_framework/components/dropdown.hpp>
#include <ui_framework/components/menu_item.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

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

    ui::UIManager uiManager;
    TTF_Font *font = TTF_OpenFont("fonts/Roboto-Medium.ttf", 8.0f);
    if (!font)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // Header buttons.
    constexpr std::array<const char *, 4> buttonLabels{"Play", "Openings", "Academy", "Statistics"};
    constexpr float buttonWidth = 52.0f;
    constexpr float buttonGap = 3.0f;
    float x = 8.0f;

    for (const char *label : buttonLabels)
    {
        auto button = std::make_unique<ui::Button>();
        button->setText(label);
        button->setFont(font);
        button->setTextColor(ui::Colors::white);
        button->setBackgroundColor(ui::Colors::transparent);
        button->setBorderColor(ui::Colors::transparent);
        button->setVariant(ui::Button::Variant::TEXT);
        button->setPosition({x, 2.0f});
        button->setSize(ui::LayoutSizeValue::fixed(buttonWidth, 20.0f));
        uiManager.addRoot(std::move(button));
        x += buttonWidth + buttonGap;
    }

    // One dropdown in the header.
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

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
