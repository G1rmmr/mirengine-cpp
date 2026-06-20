#pragma once

#include "Mir.hpp"

namespace ground{
    const mir::Real POS_X = -100;
    const mir::Real POS_Y = 700;

    const mir::Int SIZE_X = 2000;
    const mir::Int SIZE_Y = 200;

    const mir::Bool IN_AIR = false;
    const mir::Bool IS_GHOST = false;

    namespace{
        inline void InitTransform(const mir::ID id){
            mir::transform::Positions[id] = mir::Point2<mir::Real>(POS_X, POS_Y);
            mir::transform::Scales[id] = mir::Point2<mir::Real>(1, 1);
        }

        inline void InitPhysics(const mir::ID id){
            mir::physics::Bounds[id] = mir::Point2<mir::Real>(SIZE_X, SIZE_Y);
            mir::physics::Masses[id] = 10000;
            mir::physics::InAirFlags[id] = IN_AIR;
            mir::physics::IsGhosts[id] = IS_GHOST;
            mir::stats::Healths[id] = 9999; // Make ground immortal
        }

        inline void InitSprite(const mir::ID id){
            mir::sprite::Colors[id] = mir::Color(0, 0, 0);
            mir::sprite::Types[id] = mir::sprite::Type::Rectangle;
            mir::texture::AllocFromType(id);
        }
    }

    inline mir::ID Create(){
        const mir::ID id = mir::entity::Create();
        if(id == 0) return 0;

        InitTransform(id);
        InitPhysics(id);
        InitSprite(id);

        return id;
    }
}
