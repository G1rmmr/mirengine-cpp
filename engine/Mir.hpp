#pragma once

#include <cstdint>
#include <limits>

#include <functional>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <queue>
#include <stack>

#include <SFML/Graphics.hpp>

#include "core/Entity.hpp"
#include "core/Manager.hpp"
#include "component/Collider.hpp"
#include "component/Rigidbody.hpp"
#include "component/Sprite.hpp"
#include "component/Transform.hpp"
#include "system/Collision.hpp"
#include "system/Movement.hpp"

#include "handle/Event.hpp"
#include "handle/Input.hpp"
#include "handle/HTTP.hpp"

#include "util/Debugger.hpp"
#include "util/Profiler.hpp"
#include "util/Timer.hpp"
#include "util/Math.hpp"
#include "util/Types.hpp"

#include "view/Camera.hpp"
#include "view/Display.hpp"
#include "view/Scene.hpp"
#include "view/UI.hpp"
