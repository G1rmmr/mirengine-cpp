#pragma once

#include <Mir>

namespace title{
    const mir::String CONTENT = "Art Gallery Ghost";

    namespace{
        mir::ID TitleTag = 0;
        mir::ID AnounceTag = 0;

        mir::ID StartTag = 0;
        mir::ID SettingsTag = 0;
        mir::ID ExitTag = 0;

        inline void InitText(){
            const mir::Point2<mir::Uint> displayRes = {
                mir::TypeCast<mir::Uint>(mir::Window->getSize().x),
                mir::TypeCast<mir::Uint>(mir::Window->getSize().y)
            };

            TitleTag = mir::font::Create("fonts/dieproud.ttf");
            mir::font::Alloc(TitleTag);
            mir::ui::BuildText(
                TitleTag,
                mir::Color(255, 255, 255),
                mir::Point2<mir::Real>(displayRes.x / 2, displayRes.y / 2),
                CONTENT,
                100);

            AnounceTag = mir::font::Create("fonts/dieproud.ttf");
            mir::font::Alloc(AnounceTag);
            mir::ui::BuildText(
                AnounceTag,
                mir::Color(255, 255, 255),
                mir::Point2<mir::Real>(displayRes.x / 2, displayRes.y / 2 + 300),
                "Press Any Key",
                50);
        }

        inline void UpdateToMenus(){
            const mir::Point2<mir::Uint> displayRes = {
                mir::TypeCast<mir::Uint>(mir::Window->getSize().x),
                mir::TypeCast<mir::Uint>(mir::Window->getSize().y)
            };

            // Start
            StartTag = mir::font::Create("fonts/dieproud.ttf");
            mir::font::Alloc(StartTag);

            mir::Action<const mir::event::type::MousePressed&> onStartClicked =
                [](const mir::event::type::MousePressed& _){
                    mir::scene::Load("Game", "Loading");
                };

            mir::ui::BuildButton(
                onStartClicked,
                mir::Color(255, 255, 255, 0),
                mir::Point2<mir::Real>(displayRes.x / 2, displayRes.y / 2 - 100),
                {300, 20},
                StartTag,
                "START",
                mir::Color(255, 255, 255)
            );

            // Settings
            SettingsTag = mir::font::Create("fonts/dieproud.ttf");
            mir::font::Alloc(SettingsTag);

            mir::Action<const mir::event::type::MousePressed&> onSettingsClicked =
                [](const mir::event::type::MousePressed& _) {};

            mir::ui::BuildButton(
                onSettingsClicked,
                mir::Color(255, 255, 255, 0),
                mir::Point2<mir::Real>(displayRes.x / 2, displayRes.y / 2),
                {300, 20},
                 SettingsTag,
                "SETTING",
                mir::Color(255, 255, 255));

            // Exit
            ExitTag = mir::font::Create("fonts/dieproud.ttf");
            mir::font::Alloc(ExitTag);

            mir::Action<const mir::event::type::MousePressed&> onExitClicked =
                [](const mir::event::type::MousePressed& _) {
                    mir::window::Close();
                };

            mir::ui::BuildButton(
                onExitClicked,
                mir::Color(255, 255, 255, 0),
                mir::Point2<mir::Real>(displayRes.x / 2, displayRes.y / 2 + 100),
                {300, 20},
                ExitTag,
                "EXIT",
                mir::Color(255, 255, 255));
        }

        inline void InitSound(){
            const mir::Tag tag = mir::sound::Create("sounds/thunder-bgm.mp3");
            mir::sound::AllocMusic(tag);
        }

        inline void SubscribeKeyPressed(const mir::ID id){
            mir::Action<const mir::event::type::KeyPressed&> action
                = [id](const mir::event::type::KeyPressed& event){
                mir::font::Delete(TitleTag);
                mir::font::Delete(AnounceTag);

                mir::event::Clear();
                UpdateToMenus();
            };

            mir::event::Subscribe(action);
        }
    }

    inline mir::ID Create(){
        const mir::ID id = mir::entity::Create();
        if(id == 0) return 0;

        InitText();
        InitSound();

        SubscribeKeyPressed(id);
        return id;
    }
}
