#pragma once

#include "../util/Debugger.hpp"
#include "../util/Types.hpp"

namespace mir{
    using ID = Uint;

    static constexpr ID MAX_ENTITIES = 0xFFFF;

    namespace entity{
        static inline Array<Bool, MAX_ENTITIES> IsAvailables;

        static inline ID Create(){
            for(ID id = 1; id < MAX_ENTITIES; ++id){
                if(!IsAvailables[id]){
                    IsAvailables[id] = true;
                    return id;
                }
            }

            debug::Log("Entity Overflow!");
            return 0; // Overflow entity
        }

        static inline void Clear(){
            for(ID id = 1; id < MAX_ENTITIES; ++id){
                IsAvailables[id] = false;
            }
        }
    }
}
