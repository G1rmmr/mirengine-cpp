#pragma once

#include "Components.hpp"
#include "../handle/Event.hpp"
#include "../util/Math.hpp"
#include "../util/Types.hpp"
#include "Entity.hpp"

namespace mir{
    namespace movement{
        static inline void Update(const Real deltaTime){
            for(ID id = 1; id < MAX_ENTITIES; ++id){
                if(!entity::IsAvailables[id]) continue;

                Point2<Real>& velocity = transform::Velocities[id];

                velocity.y = physics::InAirFlags[id] ?
                    velocity.y + physics::GRAV_ACCEL * deltaTime :
                    velocity.y = 0;

                transform::Positions[id] += velocity * deltaTime;
            }
        }
    }

    namespace animation{
        static inline void Update(const Real deltaTime){
            for(ID id = 1; id < MAX_ENTITIES; ++id){
                if(!entity::IsAvailables[id]) continue;

                const List<Rect<Int>>& frames = animation::FrameSets[animation::States[id]];
                if(!animation::IsPlayings[id] || frames.empty()) continue;

                const Uint maxFrame = TypeCast<Uint>(frames.size());
                if(animation::DelayTimes[id] <= 0) {
                    animation::CurrFrames[id] = (animation::CurrFrames[id] + 1) % maxFrame;
                    continue;
                }

                animation::ElapsedTimes[id] += deltaTime;

                while(animation::ElapsedTimes[id] >= animation::DelayTimes[id]){
                    animation::ElapsedTimes[id] -= animation::DelayTimes[id];

                    if(++animation::CurrFrames[id] >= maxFrame){
                        if(!animation::IsLoopings[id]){
                            animation::CurrFrames[id] = maxFrame - 1;
                            animation::IsPlayings[id] = false;
                            break;
                        }
                        animation::CurrFrames[id] = 0;
                    }
                }
            }
        }
    }

    namespace collision{
        namespace{
            static inline Bool IsCollide(const ID lhs, const ID rhs){
                Rect<Int> leftBox(
                    TypeCast<Point2<Int>>(transform::Positions[lhs]),
                    TypeCast<Point2<Int>>(physics::Bounds[lhs])
                );

                Rect<Int> rightBox(
                    TypeCast<Point2<Int>>(transform::Positions[rhs]),
                    TypeCast<Point2<Int>>(physics::Bounds[rhs])
                );

                return leftBox.findIntersection(rightBox).has_value();
            }
        }

        static inline void Update(){
            List<ID> activated;

            for(ID id = 1; id < MAX_ENTITIES; ++id){
                if(entity::IsAvailables[id] && !physics::IsGhosts[id]){
                    activated.push_back(id);
                }
            }

            const Uint lastID = TypeCast<ID>(activated.size());

            for(Uint i = 0; i + 1 < lastID; i++){
                for(Uint j = i + 1; j < lastID; j++){
                    if(IsCollide(activated[i], activated[j])){
                        event::type::Collision event{activated[j]};
                        event::Publish(event);
                    }
                }
            }
        }
    }

    namespace effect{
        namespace{
            static inline void GenerateParticles(const ID id, const Real deltaTime){
                if(particle::IsEmittings[id])
                    particle::EmitAccumulators[id] += particle::EmitRates[id] * deltaTime;

                const Point2<Real>& emitterPos = transform::Positions[id];

                while(particle::EmitAccumulators[id] >= 1.0f){
                    particle::EmitAccumulators[id] -= 1.0f;

                    if(particle::Positions[id].size() < particle::MaxParticles[id]){
                        Real angle = math::GetRandomReal(0, 360);
                        Real speed = 50.0f + math::GetRandomReal(0, 100);

                        particle::Positions[id].push_back(emitterPos);
                        particle::Velocities[id].push_back({
                            math::Cos(angle) * speed,
                            math::Sin(angle) * speed
                        });
                        particle::CurrentColors[id].push_back(particle::StartColors[id]);
                        particle::CurrentSizes[id].push_back(particle::StartSizes[id]);
                        particle::CurrentLifeTimes[id].push_back(particle::TargetLifeTimes[id]);
                        particle::MaxLifeTimes[id].push_back(particle::TargetLifeTimes[id]);
                    }
                }
            }

            static inline void KillParticles(const ID id, const Uint index){
                const Size lastIdx = particle::Positions[id].size() - 1;

                particle::Positions[id][index] = particle::Positions[id][lastIdx];
                particle::Velocities[id][index] = particle::Velocities[id][lastIdx];
                particle::CurrentColors[id][index] = particle::CurrentColors[id][lastIdx];
                particle::CurrentSizes[id][index] = particle::CurrentSizes[id][lastIdx];
                particle::CurrentLifeTimes[id][index] = particle::CurrentLifeTimes[id][lastIdx];
                particle::MaxLifeTimes[id][index] = particle::MaxLifeTimes[id][lastIdx];

                particle::Positions[id].pop_back();
                particle::Velocities[id].pop_back();
                particle::CurrentColors[id].pop_back();
                particle::CurrentSizes[id].pop_back();
                particle::CurrentLifeTimes[id].pop_back();
                particle::MaxLifeTimes[id].pop_back();
            }

            static inline void UpdateParticles(const ID id, const Real deltaTime){
                const Int particleCount
                    = TypeCast<Int>(particle::Positions[id].size());

                for(Int i = particleCount - 1; i >= 0; --i){
                    Size index = TypeCast<Size>(i);

                    particle::CurrentLifeTimes[id][index] -= deltaTime;

                    if(particle::CurrentLifeTimes[id][index] <= 0){
                        KillParticles(id, TypeCast<Uint>(index));
                        continue;
                    }

                    particle::Positions[id][index] += particle::Velocities[id][index] * deltaTime;

                    if(!physics::IsGhosts[id])
                        particle::Velocities[id][index].y += physics::GRAV_ACCEL * deltaTime;

                    Real t = 1.0f - (particle::CurrentLifeTimes[id][index]
                        / particle::MaxLifeTimes[id][index]);

                    const Color& start = particle::StartColors[id];
                    const Color& end = particle::EndColors[id];

                    particle::CurrentColors[id][index] = math::Lerp(start, end, t);
                    particle::CurrentSizes[id][index] = particle::StartSizes[id]
                        + (particle::EndSizes[id] - particle::StartSizes[id]) * t;
                }
            }
        }

        static inline void Update(const Real deltaTime){
            for(ID id = 0; id < MAX_ENTITIES; ++id){
                if(!particle::IsEmittings[id] &&
                   particle::Positions[id].empty() &&
                   particle::EmitAccumulators[id] < 1.0f) continue;

                GenerateParticles(id, deltaTime);
                UpdateParticles(id, deltaTime);
            }
        }
    }

    namespace combat{
        static inline void Update(const Real deltaTime){
            for(ID id = 1; id < MAX_ENTITIES; ++id){
                if(!entity::IsAvailables[id]) continue;

                if(stats::Healths[id] <= 0){
                    event::type::Death event{};
                    event::Publish(event);
                    entity::IsAvailables[id] = false;
                }
            }
        }
    }
}
