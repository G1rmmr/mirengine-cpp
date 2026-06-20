#pragma once

#include "Mir.hpp"

#include "game/Network.hpp"
#include "game/Player.hpp"
#include "menu/Menu.hpp"
#include "game/Game.hpp"

const mir::String WINDOW_TITLE = "art-gallery-ghost";
const mir::String SAVE_PATH = "record.dat";
const mir::Uint FPS = 60;

namespace app {
    inline void Initialize(){
        mir::window::Init(WINDOW_TITLE);
        mir::window::SetFPS(FPS);

        mir::scene::Register("Menu", menu::Initialize);
        mir::scene::Register("Game", game::Initialize);

        // Inline Loading Scene
        mir::scene::Register("Loading", [](){
            mir::camera::Init();

            // Display Loading Text
            const mir::ID textID = mir::font::Create("fonts/dieproud.ttf");
            mir::font::Alloc(textID);

            const mir::Point2<mir::Uint> displayRes = {
                mir::TypeCast<mir::Uint>(mir::Window->getSize().x),
                mir::TypeCast<mir::Uint>(mir::Window->getSize().y)
            };

            mir::ui::BuildText(
                textID,
                mir::Color(255, 255, 255),
                mir::Point2<mir::Real>(displayRes.x / 2, displayRes.y / 2),
                "LOADING...",
                80
            );

            mir::time::Register(0.1f, [](){
                if(!mir::scene::TargetScene.empty()){
                    mir::scene::Load(mir::scene::TargetScene);
                }
            });
        });

        mir::scene::Load("Menu");
        mir::scene::Update();
    }

    inline void ProcessInput(){
        PROFILE_SCOPE("Input"){
            mir::window::ProcessEvents();

            if(mir::input::IsJustPressed(mir::Key::Escape)){
                network::PostScore(player::NAME, player::Score);
                network::GetTopPlayers();

                mir::scene::Load("Menu");
            }
            if(mir::input::IsJustPressed(mir::Key::F1))
                mir::debug::ToggleDebug();

            if(mir::input::IsJustPressed(mir::Key::F2))
                mir::profile::ToggleProfile();
        }
    }

    inline void Update(const mir::Real deltaTime){
        mir::scene::Update();
        mir::time::Update(deltaTime);
        mir::profile::Update(deltaTime);
        mir::effect::Update(deltaTime);
        mir::movement::Update(deltaTime);
        mir::collision::Update();
        mir::event::Update();
        mir::animation::Update(deltaTime);
        mir::camera::Update(deltaTime);
        mir::combat::Update(deltaTime);
    }

    inline void Render(){
        PROFILE_SCOPE("Render"){
            mir::window::Clear(0, 0, 0);
            mir::window::Render();
            mir::window::Display();
        }
    }

    inline void Shutdown(){
        mir::Clear();
        mir::window::Shutdown();
    }
}
