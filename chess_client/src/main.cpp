#include <windows.h>

#include "game/game.hpp"

int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    SDLWrapper sdl("My Game", 320, 180, true);

    SDL_Renderer *sdlRenderer = sdl.getRenderer();
    if (!sdlRenderer)
    {
        return -1;
    }

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

    InputManager input(sdlRenderer);
    ResourceManager rsm(sdlRenderer);

    GameScene game(sdl, rsm, input);

    bool run = true;

    while (run)
    {
        input.update();

        if (input.shouldQuit() || input.isKeyJustPressed(SDLK_ESCAPE))
            run = false;

        game.update();

        sdl.clear(0, 0, 0, 255);
        game.draw();
        sdl.present();

        sdl.updateFPS();
        SDL_Delay(16);
    }

    return 0;
}

/*
#include <windows.h>
#include <iostream>
#include <array>
#include <vector>

#include "core/sdl_wrapper.hpp"
#include "core/input_manager.hpp"
#include "core/resource_manager.hpp"
#include "core/texture.hpp"
#include "ui/components/button.hpp"
#include "ui/components/menu.hpp"
#include "ui/components/panel.hpp"
#include "ui/components/modal.hpp"
#include "ui/base/ui_manager.hpp"

// ============================================================
// Обработчик исключений для Windows
// ============================================================

inline std::vector<std::unique_ptr<ui::Button>> createButton(SDL_Renderer *renderer, TTF_Font *font)
{
    std::array<std::string, 5> texts = {"PLAY", "OPENINGS", "ACADEMY", "STATISTICS", "SETTINGS"};
    std::vector<std::unique_ptr<ui::Button>> btns;
    btns.reserve(5);

    float btnWidth = 120.0f;
    float btnHeight = 48.0f;
    float startX = 50.0f;
    float menuHeight = 70.0f;

    for (int i = 0; i < texts.size(); ++i)
    {
        float xPos = startX + i * (btnWidth + 20.0f);
        float yPos = (menuHeight - btnHeight) * 0.5f;

        auto textButton = std::make_unique<ui::Button>();
        textButton->setPosition({xPos, yPos});
        textButton->setSize({btnWidth, btnHeight});
        textButton->setBorderRadius(4.0f);

        textButton->setText(texts[i]);
        textButton->setTextColor(ui::Colors::white);
        textButton->setFont(font);
        textButton->setType(ui::Button::Type::OUTLINED);
        textButton->setBackgroundColor(ui::Colors::transparent);
        textButton->setBorderColor(ui::Colors::white);
        textButton->setTextAlignment(ui::TextAlignment::CENTER);
        textButton->disableResizeEffect(true);

        btns.push_back(std::move(textButton));
    }

    return btns;
}

inline std::vector<std::unique_ptr<ui::Menu>> createMenu(SDL_Renderer *renderer, TTF_Font *font, const std::vector<std::unique_ptr<ui::Button>> &anchorButtons)
{
    std::array<std::string, 3> playMenu = {"PLAY", "CONTINUE", "LOAD"};
    std::array<std::string, 3> openingsMenu = {"ITALIAN GAME", "SICILIAN DEFENSE", "FRENCH DEFENSE"};
    std::array<std::string, 3> academyMenu = {"PIECES", "RULES", "SPECIAL MOVES"};
    std::array<std::string, 3> statisticsMenu = {"WINS", "LOOSES", "DRAWS"};
    std::array<std::string, 3> settingsMenu = {"GRAPHICS", "AUDIO", "THEME"};
    std::array<std::array<std::string, 3>, 5> menusData = {playMenu, openingsMenu, academyMenu, statisticsMenu, settingsMenu};

    std::vector<std::unique_ptr<ui::Menu>> menus;
    menus.reserve(5);

    for (int i = 0; i < menusData.size(); ++i)
    {
        ui::LayoutPosition anchorPos = anchorButtons[i]->getPosition();
        ui::LayoutSize anchorSize = anchorButtons[i]->getSize();

        auto testMenu = std::make_unique<ui::Menu>();
        testMenu->setPosition({anchorPos.x, anchorPos.y + anchorSize.height});
        testMenu->setSize({anchorSize.width, 200.0f});
        testMenu->setBackgroundColor(ui::Colors::gray);
        testMenu->setPadding({5, 5});

        for (int j = 0; j < menusData[i].size(); ++j)
        {
            ui::Button menuBtn;
            menuBtn.setType(ui::Button::Type::FILLED);
            menuBtn.setTextColor({0, 0, 0, 255});
            menuBtn.setFont(font);
            menuBtn.setFontSize(14.0f);
            menuBtn.setText(menusData[i][j]);
            menuBtn.setTextAlignment(ui::TextAlignment::CENTER);
            menuBtn.setBackgroundColor(ui::Colors::transparent);
            menuBtn.setBorderColor(ui::Colors::transparent);
            testMenu->addMenuButton(std::move(menuBtn));
        }

        menus.push_back(std::move(testMenu));
    }

    return menus;
}

// ============================================================
// main()
// ============================================================
int main()
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    SDLWrapper sdl("UI Container Test - Alignment Demo", 1280, 720, true);

    SDL_Renderer *sdlRenderer = sdl.getRenderer();
    if (!sdlRenderer)
    {
        return -1;
    }

    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

    InputManager input(sdlRenderer);
    ResourceManager rsm(sdlRenderer);

    bool run = true;

    rsm.loadTexture("chess_background.jpg");
    Texture *bg = rsm.getTexture("chess_background.jpg");

    TTF_Font *font = TTF_OpenFont("fonts/Roboto-Medium.ttf", 16);
    if (!font)
    {
        std::cerr << "Failed to load font!" << std::endl;
        return -1;
    }

    if (!bg)
    {
        std::cerr << "Failed to load background texture!" << std::endl;
        return -1;
    }

    static constexpr float MAX_DELTA_TIME = 0.1f;
    static constexpr float MIN_DELTA_TIME = 0.001f;
    float lastTickTime = 0.0f;

    // ============================================================
    // Главное меню
    // ============================================================
    std::vector<std::unique_ptr<ui::Button>> btns = createButton(sdlRenderer, font);
    std::vector<std::unique_ptr<ui::Menu>> menus = createMenu(sdlRenderer, font, btns);

    std::vector<ui::Menu *> ptr_menus;
    std::vector<ui::Button *> ptr_btns;

    for (const auto &btn : btns)
        ptr_btns.push_back(btn.get());

    for (const auto &menu : menus)
        ptr_menus.push_back(menu.get());

    auto modal = std::make_unique<ui::Modal>();
    auto closeButton = std::make_unique<ui::Button>();

    closeButton->setPosition({sdl.getWidth() - closeButton->getWidth() - 50.0f, 11.0f});
    closeButton->setType(ui::Button::Type::OUTLINED);
    closeButton->setTextColor(ui::Colors::white);
    closeButton->setFont(font);
    closeButton->setText("CLOSE");
    closeButton->setTextAlignment(ui::TextAlignment::CENTER);
    closeButton->setBorderColor(ui::Colors::white);
    closeButton->disableResizeEffect(true);
    closeButton->setBorderRadius(4.0f);

    closeButton->setOnClick([&run](ui::Button &closeBtn)
                            { run = false; });

    float xCenter = (sdl.getWidth() - modal->getWidth()) * 0.5f;
    float yCenter = (sdl.getHeight() - modal->getHeight()) * 0.5f;

    modal->setPosition({xCenter, yCenter});

    modal->setOnClose([](ui::Modal &mod)
                      { mod.setOpen(false); });

    size_t count = menus[0]->getButtonCount();

    auto cb = [&modal](ui::Button &button)
    {
        if (!modal->isOpen())
            modal->setOpen(true);
    };

    for (size_t i = 0; i < count; ++i)
    {
        auto btn = menus[0]->getMenuButton(i);

        btn->setOnClick(cb);
    };

    ui::UIManager uiManager;

    uiManager.addOverlayWidget(std::move(modal));
    uiManager.addNormalWidget(std::move(closeButton));

    for (auto &btn : btns)
    {
        uiManager.addNormalWidget(std::move(btn));
    }

    for (auto &menu : menus)
    {
        uiManager.addNormalWidget(std::move(menu));
    }

    // ============================================================
    // Главный цикл
    // ============================================================

    while (run)
    {
        input.update();

        auto [mouseX, mouseY] = input.getMousePosition();
        ui::MouseData mouseData;
        mouseData.mouseX = mouseX;
        mouseData.mouseY = mouseY;
        mouseData.leftPressed = input.isMouseButtonHeld(MouseButtonType::LEFT);
        mouseData.leftJustPressed = input.isMouseButtonJustPressed(MouseButtonType::LEFT);
        mouseData.leftJustReleased = input.isMouseButtonJustReleased(MouseButtonType::LEFT);

        Uint32 currentTime = SDL_GetTicks();
        if (lastTickTime == 0.0f)
        {
            lastTickTime = static_cast<float>(currentTime);
            continue;
        }

        float deltaTime = (currentTime - lastTickTime) / 1000.0f;
        lastTickTime = static_cast<float>(currentTime);

        if (deltaTime > MAX_DELTA_TIME)
            deltaTime = MAX_DELTA_TIME;
        if (deltaTime < MIN_DELTA_TIME)
            continue;

        uiManager.updateInputState(mouseData);
        uiManager.handleEvents(mouseData);
        uiManager.update(deltaTime);

        // ---- Логика меню ----
        for (int i = 0; i < ptr_menus.size(); ++i)
        {
            bool mouseOnAnchor = ptr_btns[i]->isHovered();
            bool mouseOnMenu = ptr_menus[i]->isHovered();

            if (ptr_menus[i]->isOpen())
            {
                if (!mouseOnMenu && !mouseOnAnchor)
                    ptr_menus[i]->setOpen(false);
            }
            else
            {
                if (mouseOnAnchor)
                    ptr_menus[i]->setOpen(true);
            }
        }

        // ---- Отрисовка ----
        sdl.clear(0, 0, 0, 255);
        bg->render(0, 0, nullptr, 1.74f);

        uiManager.draw(sdlRenderer);

        sdl.present();
        sdl.updateFPS();
        SDL_Delay(8);
    }

    TTF_CloseFont(font);
    TTF_Quit();

    return 0;
}


        menuButton.disableResizeEffect(true);
        menuButton.setBorderRadius(0.0f);
        menuButton.setBackgroundColor(Colors::transparent);


*/