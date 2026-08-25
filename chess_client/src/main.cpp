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
#include <ui_framework/components/tab_control.hpp>
#include <ui_framework/components/typography.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace
{
    constexpr int logicalWidth = 1920;
    constexpr int logicalHeight = 1080;

    std::string getResourcePath(const char *relativePath)
    {
        const char *basePath = SDL_GetBasePath();
        if (!basePath)
            return relativePath;
        return std::string(basePath) + relativePath;
    }

    void configureButton(ui::Button &button, const ui::Color &background, const ui::Color &border = ui::Colors::gray)
    {
        button.setTextColor(ui::Colors::white);
        button.setBackgroundColor(background);
        button.setBorderColor(border);
        button.setVariant(ui::Button::Variant::FILLED);
        button.setPadding({18.0f, 18.0f, 10.0f, 10.0f});
    }

    std::unique_ptr<ui::Typography> makeTypography(const std::string &text, ui::Typography::Variant variant, TTF_Font *font)
    {
        auto typography = std::make_unique<ui::Typography>();
        typography->setText(text);
        typography->setFont(font);
        typography->setVariant(variant);
        typography->setColor(ui::Colors::white);
        return typography;
    }
}

int main()
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        return -1;
    if (!TTF_Init())
    {
        SDL_Quit();
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "ChessClient - UI Framework Visual Validation",
        logicalWidth,
        logicalHeight,
        SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
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
        SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    const std::string fontPath = getResourcePath("fonts/Roboto-Medium.ttf");
    TTF_Font *font = TTF_OpenFont(fontPath.c_str(), 24.0f);
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
        root->setPosition({60.0f, 40.0f});
        root->setSize(ui::LayoutSizeValue::fixed(1800.0f, 1000.0f));
        root->setPadding({20.0f, 20.0f, 20.0f, 20.0f});
        root->setGap(14.0f);

        root->addChild(makeTypography(
                           "UI Framework Visual Validation — 1920x1080 logical / LETTERBOX",
                           ui::Typography::Variant::H2, font),
                       root->getChildCount());

        auto scaleNote = makeTypography(
            "F11 toggles fullscreen. The logical 1920x1080 canvas is letterboxed on every display aspect ratio.",
            ui::Typography::Variant::BODY2, font);
        scaleNote->setColor(ui::Colors::yellow);
        root->addChild(std::move(scaleNote), root->getChildCount());

        root->addChild(makeTypography("1. Typography / logical layout", ui::Typography::Variant::H3, font), root->getChildCount());

        auto typographyRow = std::make_unique<ui::StackPanelNode>(ui::StackPanelNode::Orientation::Horizontal);
        typographyRow->setGap(18.0f);
        typographyRow->setSize(ui::LayoutSizeValue::fixed(1760.0f, 110.0f));

        auto h1 = makeTypography("H1 — Large logical type", ui::Typography::Variant::H1, font);
        h1->setSize(ui::LayoutSizeValue::fixed(560.0f, 90.0f));
        typographyRow->addChild(std::move(h1), typographyRow->getChildCount());

        auto centered = makeTypography("CENTER alignment", ui::Typography::Variant::H4, font);
        centered->setSize(ui::LayoutSizeValue::fixed(560.0f, 90.0f));
        centered->setHorizontalAlignment(ui::TextAlignment::CENTER);
        typographyRow->addChild(std::move(centered), typographyRow->getChildCount());

        auto endAligned = makeTypography("END alignment", ui::Typography::Variant::H4, font);
        endAligned->setSize(ui::LayoutSizeValue::fixed(560.0f, 90.0f));
        endAligned->setHorizontalAlignment(ui::TextAlignment::END);
        typographyRow->addChild(std::move(endAligned), typographyRow->getChildCount());
        root->addChild(std::move(typographyRow), root->getChildCount());

        auto wrap = makeTypography(
            "WRAP TEST: This paragraph deliberately contains enough text to exceed the available logical width. It should wrap naturally inside the allocated rectangle while its alignment remains independent from the physical renderer.",
            ui::Typography::Variant::BODY1, font);
        wrap->setSize(ui::LayoutSizeValue::fixed(1700.0f, 120.0f));
        wrap->setWrapMode(ui::WrapMode::WRAP);
        root->addChild(std::move(wrap), root->getChildCount());

        root->addChild(makeTypography("2. Text-bearing controls", ui::Typography::Variant::H3, font), root->getChildCount());

        auto controls = std::make_unique<ui::StackPanelNode>(ui::StackPanelNode::Orientation::Horizontal);
        controls->setGap(16.0f);
        controls->setSize(ui::LayoutSizeValue::fixed(1760.0f, 100.0f));

        auto filled = std::make_unique<ui::Button>();
        filled->setText("Filled Button");
        filled->setFont(font);
        configureButton(*filled, ui::Colors::blue);
        controls->addChild(std::move(filled), controls->getChildCount());

        auto outlined = std::make_unique<ui::Button>();
        outlined->setText("Outlined Button");
        outlined->setFont(font);
        configureButton(*outlined, ui::Colors::transparent, ui::Colors::white);
        outlined->setVariant(ui::Button::Variant::OUTLINED);
        controls->addChild(std::move(outlined), controls->getChildCount());

        auto textButton = std::make_unique<ui::Button>();
        textButton->setText("Text Button");
        textButton->setFont(font);
        configureButton(*textButton, ui::Colors::transparent, ui::Colors::transparent);
        textButton->setVariant(ui::Button::Variant::TEXT);
        controls->addChild(std::move(textButton), controls->getChildCount());

        auto dropdown = std::make_unique<ui::Dropdown>();
        dropdown->setSize(ui::LayoutSizeValue::fixed(360.0f, 80.0f));
        dropdown->getTrigger().setFont(font);
        dropdown->getTrigger().setTextColor(ui::Colors::white);
        dropdown->getTrigger().setBackgroundColor(ui::Colors::gray);
        dropdown->getTrigger().setBorderColor(ui::Colors::gray);
        dropdown->getTrigger().setVariant(ui::Button::Variant::FILLED);
        dropdown->setPlaceholder("Select an option");
        for (const char *label : {"Settings", "About", "Diagnostics", "Quit"})
        {
            auto item = std::make_unique<ui::MenuItem>();
            item->setText(label);
            item->setFont(font);
            item->setTextColor(ui::Colors::white);
            item->setBackgroundColor(ui::Colors::gray);
            dropdown->addItem(std::move(item));
        }
        controls->addChild(std::move(dropdown), controls->getChildCount());
        root->addChild(std::move(controls), root->getChildCount());

        root->addChild(makeTypography("3. TabControl / selection", ui::Typography::Variant::H3, font), root->getChildCount());

        auto tabs = std::make_unique<ui::TabControl>();
        tabs->setSize(ui::LayoutSizeValue::fixed(1760.0f, 100.0f));
        for (const char *label : {"Overview", "Board", "Analysis", "History"})
        {
            auto tab = std::make_unique<ui::TabItem>();
            tab->setText(label);
            tab->setFont(font);
            tab->setTextColor(ui::Colors::white);
            tab->setBackgroundColor(ui::Colors::gray);
            tabs->addTab(std::move(tab));
        }
        tabs->selectTab(0);
        root->addChild(std::move(tabs), root->getChildCount());

        root->addChild(makeTypography("4. Scroll behavior", ui::Typography::Variant::H3, font), root->getChildCount());

        auto scrollPanel = std::make_unique<ui::StackPanelNode>(ui::StackPanelNode::Orientation::Vertical);
        scrollPanel->setSize(ui::LayoutSizeValue::fixed(1760.0f, 250.0f));
        scrollPanel->setPadding({18.0f, 18.0f, 18.0f, 18.0f});
        scrollPanel->setGap(10.0f);
        scrollPanel->setOverflow(ui::Overflow::HIDDEN);
        for (int i = 0; i < 12; ++i)
        {
            auto item = std::make_unique<ui::Button>();
            item->setText("Scrollable item " + std::to_string(i + 1));
            item->setFont(font);
            configureButton(*item, i % 2 == 0 ? ui::Colors::gray : ui::Colors::blue);
            item->setSize(ui::LayoutSizeValue::fixed(1650.0f, 62.0f));
            scrollPanel->addChild(std::move(item), scrollPanel->getChildCount());
        }
        ui::Node *scrollNode = root->addChild(std::move(scrollPanel), root->getChildCount());
        uiManager.enableScrolling(*scrollNode);

        uiManager.addRoot(std::move(root));

        auto modal = std::make_unique<ui::StackPanelNode>(ui::StackPanelNode::Orientation::Vertical);
        modal->setPosition({560.0f, 300.0f});
        modal->setSize(ui::LayoutSizeValue::fixed(800.0f, 420.0f));
        modal->setPadding({32.0f, 32.0f, 24.0f, 24.0f});
        modal->setGap(18.0f);

        modal->addChild(makeTypography("Modal / backdrop test", ui::Typography::Variant::H3, font), modal->getChildCount());
        auto modalText = makeTypography(
            "This validates modal visibility, focus routing, backdrop rendering and close behavior.",
            ui::Typography::Variant::BODY1, font);
        modalText->setWrapMode(ui::WrapMode::WRAP);
        modal->addChild(std::move(modalText), modal->getChildCount());

        auto closeModal = std::make_unique<ui::Button>();
        closeModal->setText("Close modal");
        closeModal->setFont(font);
        configureButton(*closeModal, ui::Colors::gray);
        closeModal->setSize(ui::LayoutSizeValue::fixed(260.0f, 70.0f));
        closeModal->setOnActivate([&uiManager](ui::Button &)
                                  { uiManager.closeModal(); });
        modal->addChild(std::move(closeModal), modal->getChildCount());

        ui::Node *modalNode = uiManager.addOverlay(std::move(modal));

        auto launcher = std::make_unique<ui::Button>();
        launcher->setText("Show Modal");
        launcher->setFont(font);
        configureButton(*launcher, ui::Colors::red);
        launcher->setPosition({1440.0f, 940.0f});
        launcher->setSize(ui::LayoutSizeValue::fixed(320.0f, 76.0f));
        launcher->setOnActivate([&uiManager, modalNode](ui::Button &)
                                {
            if (modalNode)
                uiManager.showModal(*modalNode, ui::BackdropClickBehavior::Close); });
        uiManager.addRoot(std::move(launcher));

        bool running = true;
        bool fullscreen = false;
        while (running)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE))
                    running = false;
                else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F11)
                {
                    fullscreen = !fullscreen;
                    SDL_SetWindowFullscreen(window, fullscreen);
                }

                uiManager.processEvent(event, renderer);
            }

            SDL_SetRenderDrawColor(renderer, 18, 18, 24, 255);
            SDL_RenderClear(renderer);
            uiManager.runFrame(1.0f / 60.0f, renderer);
            SDL_RenderPresent(renderer);
            SDL_Delay(8);
        }
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
