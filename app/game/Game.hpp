#pragma once

#include "Player.hpp"
#include "Ground.hpp"
#include "TestParticle.hpp"

namespace game{
    mir::ID PlayerID = 0;
    mir::ID GroundID = 0;
    mir::ID TestParticleID = 0;

    inline void Initialize(){
        PROFILE_SCOPE("Initialization"){
            mir::camera::Init();

            PlayerID = player::Create();
            GroundID = ground::Create();
            TestParticleID = particle::Create();

            mir::time::Register(1, [](){
                mir::stats::Healths[PlayerID]--;
                player::Score += 10;
            }, true);

            mir::Action<const mir::event::type::Death&> action
                = [](const mir::event::type::Death&){
                mir::window::Close();
            };

            mir::event::Subscribe(action);
        }
        mir::debug::Log("Game Initialized!");
    }
}
