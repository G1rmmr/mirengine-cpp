#pragma once

#include <Mir>

#include "Network.hpp"

namespace player{
    const mir::String NAME = "JIWON";

    const mir::Real POS_X = 200;
    const mir::Real POS_Y = 200;
    const mir::Real SPEED = 200;
    const mir::Real JUMP_POWER = 3000;
    const mir::Real MASS = 10;
    const mir::Real MAX_HEALTH = 100;
    const mir::Real INIT_DAMAGE = 10;

    const mir::Int SIZE_X = 100;
    const mir::Int SIZE_Y = 100;

    const mir::Bool IN_AIR = true;
    const mir::Bool IS_GHOST = false;

    mir::Uint Score = 0;

    namespace{
        inline void InitTransform(const mir::ID id){
            mir::transform::Positions[id] = mir::Point2<mir::Real>(POS_X, POS_Y);
            mir::transform::Scales[id] = mir::Point2<mir::Real>(1, 1);
        }

        inline void InitPhysics(const mir::ID id){
            mir::physics::Bounds[id] = mir::Point2<mir::Real>(SIZE_X, SIZE_Y);
            mir::physics::Masses[id] = MASS;
            mir::physics::InAirFlags[id] = IN_AIR;
            mir::physics::IsGhosts[id] = IS_GHOST;
        }

        inline void InitSprite(const mir::ID id){
            mir::sprite::Colors[id] = mir::Color(255, 255, 255);
            mir::sprite::Types[id] = mir::sprite::Type::ConvexHull;
            mir::sprite::NumSide[id] = 5;
            mir::texture::AllocFromType(id);
        }

        inline void InitStats(const mir::ID id){
            mir::stats::Healths[id] = MAX_HEALTH;
            mir::stats::Damages[id] = INIT_DAMAGE;
        }

        inline void SubscribeCollision(const mir::ID id){
            mir::Action<const mir::event::type::Collision&> action
                = [id](const mir::event::type::Collision& args){
                mir::Real left = mir::transform::Positions[id].x;
                mir::Real top = mir::transform::Positions[id].y;

                mir::Real otherLeft = mir::transform::Positions[args.Other].x;
                mir::Real otherTop = mir::transform::Positions[args.Other].y;
                mir::Real otherRight = otherLeft + mir::physics::Bounds[args.Other].x;
                mir::Real otherBottom = otherTop + mir::physics::Bounds[args.Other].y;

                mir::Real dx = (otherLeft + mir::physics::Bounds[args.Other].x / 2)
                    - (left + mir::physics::Bounds[id].x / 2);

                mir::Real dy = (otherTop + mir::physics::Bounds[args.Other].y / 2)
                    - (top + mir::physics::Bounds[id].y / 2);

                mir::Real overlapX = (mir::physics::Bounds[id].x / 2
                    + mir::physics::Bounds[args.Other].x / 2)
                    - std::abs(dx);

                mir::Real overlapY = (mir::physics::Bounds[id].y / 2
                    + mir::physics::Bounds[args.Other].y / 2)
                    - std::abs(dy);

                if(overlapX < overlapY){
                    mir::transform::Positions[id].x = dx > 0 ?
                        otherLeft - mir::physics::Bounds[id].x :
                        otherRight;
                }
                else{
                    mir::transform::Positions[id].y = dy > 0 ?
                    otherTop - mir::physics::Bounds[id].y :
                    otherBottom;

                    if(dy > 0) mir::physics::InAirFlags[id] = false;
                }
            };
            mir::event::Subscribe(action);
        }

        // 점프, 키 입력, 사망 이벤트 구독을 삭제하고 단일 폴링 함수로 대체합니다.
    }

    inline void UpdateInput(const mir::ID id) {
        // W 눌렸을 때 (점프)
        if (mir::input::IsJustPressed(mir::Key::W)) {
            if (!mir::physics::InAirFlags[id]) {
                const mir::Real force = JUMP_POWER / std::sqrt(mir::physics::Masses[id]);
                mir::transform::Velocities[id].y = -force;
                mir::physics::InAirFlags[id] = true;
                mir::camera::Shake(0.01, 1);
                mir::camera::Follow(id);
            }
        }

        // A/D 이동 폴링
        if (mir::input::IsPressed(mir::Key::A)) {
            mir::transform::Velocities[id].x = -SPEED;
        } 
        else if (mir::input::IsPressed(mir::Key::D)) {
            mir::transform::Velocities[id].x = SPEED;
        } 
        else {
            mir::transform::Velocities[id].x = 0;
        }

        // Space (카메라 팔로우)
        if (mir::input::IsJustPressed(mir::Key::Space)) {
            mir::camera::Follow(id);
        }

        // 실시간 사망 조건 검사 (구독 제거)
        if (mir::stats::Healths[id] <= 0) {
            network::PostScore(NAME, Score);
            network::GetTopPlayers();
        }
    }

    inline mir::ID Create(){
        const mir::ID id = mir::entity::Create();
        if(id == 0) return 0;

        InitTransform(id);
        InitPhysics(id);
        InitSprite(id);
        InitStats(id);

        return id;
    }
}
