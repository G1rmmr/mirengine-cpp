#pragma once

#include <Mir>

namespace game{
    namespace particle{
        namespace{
            static inline void Initialize(const mir::ID id){
                mir::transform::Positions[id] = mir::Point2<mir::Real>(500, 400);
                mir::physics::IsGhosts[id] = true;

                mir::particle::MaxParticles[id] = 100;
                mir::particle::EmitRates[id] = 50;
                mir::particle::EmitAccumulators[id] = 0;

                mir::particle::StartColors[id] = mir::Color::Red;
                mir::particle::EndColors[id] = mir::Color::Blue;

                mir::particle::StartSizes[id] = 10;
                mir::particle::EndSizes[id] = 2;

                mir::particle::TargetLifeTimes[id] = 1;
                mir::particle::IsEmittings[id] = true;
                
                mir::stats::Healths[id] = 9999; // Make particle emitter immortal

                // mir::particle::BurstParticle(id, 50);
            }
        }

        inline mir::ID Create(){
            mir::ID id = mir::entity::Create();
            if(id == 0) return 0;

            Initialize(id);

            mir::debug::Log("Test Particle Emitter Created at ID: %d", id);
            return id;
        }
    }
}
