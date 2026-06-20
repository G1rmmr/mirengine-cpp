#pragma once

#include "Title.hpp"

namespace menu{
    mir::ID TitleID = 0;

    inline void Initialize(){
        PROFILE_SCOPE("Initialization"){
            mir::camera::Init();
            TitleID = title::Create();
        }
        mir::debug::Log("Menu Initialized!");
    }
}
