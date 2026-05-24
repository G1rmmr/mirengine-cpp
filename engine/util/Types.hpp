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
#include <thread>
#include <utility>

#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/SoundSource.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Network/Http.hpp>

namespace mir {
    using PrimType = sf::PrimitiveType;
    using PointArr = sf::VertexArray;
    using Circle = sf::CircleShape;
    using Rectangle = sf::RectangleShape;

    using Int = std::int32_t;
    using Uint = std::uint16_t;
    using Byte = std::uint8_t;
    using Size = std::size_t;
    using Real = float;
    using Bool = bool;
    using String = FixedString<256>;
    using Color = sf::Color;

    using RenderTex = sf::RenderTexture;
    using Texture = sf::Texture;
    using Sprite = sf::Sprite;

    using HandledWindow = sf::RenderWindow;

    using SoundBuf = sf::SoundBuffer;
    using SoundSrc = sf::SoundSource;
    using Sound = sf::Sound;
    using Music = sf::Music;

    using Font = sf::Font;
    using Text = sf::Text;

    using HTTP = sf::Http;

    template<typename T>
    using List = std::vector<T>;

    template<typename T, Size S>
    using Array = std::array<T, S>;

    template<typename K, typename V>
    using Dictionary = std::unordered_map<K, V>;

    template<typename T>
    using Queue = std::queue<T>;

    template<typename T>
    using Stack = std::stack<T>;

    template<typename... TArgs>
    using Action = std::function<void(TArgs...)>;

    template<typename TReturn, typename... TArgs>
    using Func = std::function<TReturn(TArgs...)>;

    template<typename T>
    using Point2 = sf::Vector2<T>;

    template<typename T>
    using Rect = sf::Rect<T>;

    static constexpr Int I_MAX = std::numeric_limits<Int>::max();
    static constexpr Int I_MIN = std::numeric_limits<Int>::min();
    static constexpr Uint U_MAX = std::numeric_limits<Uint>::max();
    static constexpr Uint U_MIN = std::numeric_limits<Uint>::min();
    static constexpr Byte B_MAX = std::numeric_limits<Byte>::max();
    static constexpr Byte B_MIN = std::numeric_limits<Byte>::min();
    static constexpr Real R_MAX = std::numeric_limits<Real>::max();
    static constexpr Real R_MIN = std::numeric_limits<Real>::min();

    template <typename T, typename U>
    constexpr static inline T TypeCast(U&& value){
        return static_cast<T>(std::forward<U>(value));
    }

    template <typename T, typename U>
    constexpr static inline T ConstCast(U&& value){
        return const_cast<T>(std::forward<U>(value));
    }

    template <typename T>
    static inline String ToString(T&& value){
        return std::to_string(std::forward<T>(value));
    }

    template <typename T, typename... Args>
    static inline void FireAndForget(T&& func, Args&&... args){
        std::thread(std::forward<T>(func), std::forward<Args>(args)...).detach();
    }
}
(args)...).detach();
    }
}
