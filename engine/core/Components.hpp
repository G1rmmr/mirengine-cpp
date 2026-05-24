#pragma once

#include <array>
#include <vector>
#include <memory>

#include "Entity.hpp"
#include "../util/Timer.hpp"
#include "../util/Types.hpp"

namespace mir
{
    namespace transform
    {
        static inline Array<Point2<Real>, MAX_ENTITIES> Positions;
        static inline Array<Point2<Real>, MAX_ENTITIES> Velocities;
        static inline Array<Point2<Real>, MAX_ENTITIES> Scales;
        static inline Array<Real, MAX_ENTITIES> Rotations;

        static inline void Delete(const ID id) 
        {
            if (!entity::IsAvailables[id]) 
            {
                return;
            }

            Positions[id] = Point2<Real>(0, 0);
            Velocities[id] = Point2<Real>(0, 0);
            Scales[id] = Point2<Real>(0, 0);
            Rotations[id] = 0;

            entity::IsAvailables[id] = true;
        }

        static inline void Clear()
        {
            for(ID id = 1; id < MAX_ENTITIES; ++id) 
            {
                Delete(id);   
            }
        }
    }

    namespace physics{
        static constexpr Real GRAV_ACCEL = 980;

        static inline Array<Point2<Real>, MAX_ENTITIES> Bounds;
        static inline Array<Real, MAX_ENTITIES> Masses;

        static inline Array<Bool, MAX_ENTITIES> IsGhosts;
        static inline Array<Bool, MAX_ENTITIES> InAirFlags;

        static inline void Delete(const ID id) 
        {
            if(!entity::IsAvailables[id]) 
            {
                return;   
            }

            Bounds[id] = Point2<Real>(0, 0);
            Masses[id] = 0;
            IsGhosts[id] = false;
            InAirFlags[id] = false;

            entity::IsAvailables[id] = true;
        }

        static inline void Clear()
        {
            for(ID id = 1; id < MAX_ENTITIES; ++id) 
            {
                Delete(id);   
            }
        }
    }

    namespace sprite
    {
        enum class Type
        {
            None,
            Rectangle,
            Circle,
            Arch,
            ConvexHull,
            Triangle
        };

        static inline Array<Color, MAX_ENTITIES> Colors;
        static inline Array<Point2<Real>, MAX_ENTITIES> Sizes;

        static inline Array<std::unique_ptr<Texture>, MAX_ENTITIES> Textures;

        static inline Array<Real, MAX_ENTITIES> Archs;
        static inline Array<Type, MAX_ENTITIES> Types;
        static inline Array<Uint, MAX_ENTITIES> Layers;
        static inline Array<Uint, MAX_ENTITIES> NumSide;

        static inline Array<Bool, MAX_ENTITIES> ShouldFlipXs;
        static inline Array<Bool, MAX_ENTITIES> ShouldFlipYs;

        static inline void Delete(const ID id)
        {
            if(!entity::IsAvailables[id])
            {
                return;   
            }

            Colors[id] = Color(0, 0, 0);
            Sizes[id] = Point2<Real>(0, 0);
            Textures[id].reset();
            Types[id] = Type::None;
            Layers[id] = 0;
            ShouldFlipXs[id] = false;
            ShouldFlipYs[id] = false;

            entity::IsAvailables[id] = true;
        }

        static inline void Clear()
        {
            for(ID id = 1; id < MAX_ENTITIES; ++id)
            {
                Delete(id);
            } 
        }
    }

    namespace animation
    {
        static constexpr Uint MAX_STATE = 0x08;
        static constexpr Uint MAX_FRAME = 0xFF;

        using Set = List<Rect<Int>>;

        static inline Array<Set, MAX_ENTITIES> FrameSets;
        static inline Array<Real, MAX_ENTITIES> ElapsedTimes;
        static inline Array<Real, MAX_ENTITIES> DelayTimes;
        static inline Array<Uint, MAX_STATE> States;
        static inline Array<Uint, MAX_FRAME> CurrFrames;
        static inline Array<Bool, MAX_ENTITIES> IsPlayings;
        static inline Array<Bool, MAX_ENTITIES> IsLoopings;

        static inline void Delete(const ID id) 
        {
            if(!entity::IsAvailables[id])
            {
                return;  
            } 

            FrameSets[id] = Set();
            ElapsedTimes[id] = 0;
            DelayTimes[id] = 0;
            States[id] = 0;
            CurrFrames[id] = 0;
            IsPlayings[id] = false;
            IsLoopings[id] = false;

            entity::IsAvailables[id] = true;
        }

        static inline void Clear()
        {
            for(ID id = 1; id < MAX_ENTITIES; ++id)
            {
                Delete(id);   
            }
        }
    }

    namespace particle
    {
        static inline Array<List<Point2<Real>>, MAX_ENTITIES> Positions;
        static inline Array<List<Point2<Real>>, MAX_ENTITIES> Velocities;
        static inline Array<List<Color>, MAX_ENTITIES> CurrentColors;
        static inline Array<List<Real>, MAX_ENTITIES> CurrentSizes;
        static inline Array<List<Real>, MAX_ENTITIES> CurrentLifeTimes;
        static inline Array<List<Real>, MAX_ENTITIES> MaxLifeTimes;
        static inline Array<Color, MAX_ENTITIES> StartColors;
        static inline Array<Color, MAX_ENTITIES> EndColors;
        static inline Array<Real, MAX_ENTITIES> EmitRates;
        static inline Array<Real, MAX_ENTITIES> EmitAccumulators;
        static inline Array<Real, MAX_ENTITIES> StartSizes;
        static inline Array<Real, MAX_ENTITIES> EndSizes;
        static inline Array<Real, MAX_ENTITIES> TargetLifeTimes;
        static inline Array<Uint, MAX_ENTITIES> MaxParticles;
        static inline Array<Bool, MAX_ENTITIES> IsEmittings;

        static inline void BurstParticle(const ID id, const Uint count) {
            EmitAccumulators[id] += TypeCast<Real>(count);
            IsEmittings[id] = true;

            time::Register(0.01f, [id]() { particle::IsEmittings[id] = false; });
        }

        static inline void Delete(const ID id) {
            if(!entity::IsAvailables[id]) return;
            Positions[id].clear();
            Velocities[id].clear();
            CurrentColors[id].clear();
            CurrentSizes[id].clear();
            CurrentLifeTimes[id].clear();
            MaxLifeTimes[id].clear();
            StartColors[id] = Color(0, 0, 0);
            StartColors[id] = Color(0, 0, 0);
            EmitRates[id] = 0.0;
            EmitAccumulators[id] = 0.0;
            StartSizes[id] = 0.0;
            EndSizes[id] = 0.0;
            TargetLifeTimes[id] = 0.0;
            MaxParticles[id] = 0;
            IsEmittings[id] = false;

            entity::IsAvailables[id] = true;
        }

        static inline void Clear() {
            for(ID id = 1; id < MAX_ENTITIES; ++id)
                Delete(id);
        }
    }

    namespace stats{
        static constexpr Uint MAX_HEALTH = 0xFFFF;
        static constexpr Uint MAX_DAMAGE = 0xFF;

        static inline Array<Real, MAX_ENTITIES> Healths;
        static inline Array<Real, MAX_ENTITIES> Damages;

        static inline void Delete(const ID id) {
            if(!entity::IsAvailables[id]) return;
            Healths[id] = 0;
            Damages[id] = 0;

            entity::IsAvailables[id] = true;
        }

        static inline void Clear(){
            for(ID id = 1; id < MAX_ENTITIES; ++id) Delete(id);
        }
    }
}
