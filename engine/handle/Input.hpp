#pragma once

#include <unordered_map>
#include <array>

#include "Event.hpp"
#include "../view/Display.hpp"
#include "../util/Types.hpp"

namespace mir{
    namespace input{
        namespace{
            static inline const Dictionary<sf::Keyboard::Key, event::type::Key> KEY_MAP = {
                {sf::Keyboard::Key::Escape, event::type::Key::Escape},
                {sf::Keyboard::Key::W, event::type::Key::W},
                {sf::Keyboard::Key::A, event::type::Key::A},
                {sf::Keyboard::Key::S, event::type::Key::S},
                {sf::Keyboard::Key::D, event::type::Key::D},
                {sf::Keyboard::Key::F1, event::type::Key::F1},
                {sf::Keyboard::Key::F2, event::type::Key::F2},
                {sf::Keyboard::Key::Space, event::type::Key::Space},
                {sf::Keyboard::Key::Enter, event::type::Key::Enter}
            };

            static inline Array<Bool, TypeCast<Size>(sf::Keyboard::KeyCount)> IsPressedState;
        }

        static inline Bool IsPressed(event::type::Key key){
            for(const auto& [input, value] : KEY_MAP){
                if(value == key) return IsPressedState[TypeCast<Size>(input)];
            }
            return false;
        }

        static inline void Process(){
            while(const auto event = Window->pollEvent()){
                if(event->is<sf::Event::Closed>()){
                    mir::window::Close();
                }
                else if(const auto* key = event->getIf<sf::Event::KeyPressed>()){
                    Size code = TypeCast<Size>(key->code);
                    if(KEY_MAP.count(key->code) && !IsPressedState[code]){
                        IsPressedState[code] = true;
                        event::Publish(event::type::KeyPressed{KEY_MAP.at(key->code)});
                    }
                }
                else if(const auto* key = event->getIf<sf::Event::KeyReleased>()){
                     Size code = TypeCast<Size>(key->code);
                     if(KEY_MAP.count(key->code)){
                        IsPressedState[code] = false;
                        event::Publish(event::type::KeyReleased{KEY_MAP.at(key->code)});
                    }
                }
                else if(const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()){
                    event::Publish(event::type::MousePressed{mouse->position.x, mouse->position.y});
                }
                else if(const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()){
                    event::Publish(event::type::MouseReleased{mouse->position.x, mouse->position.y});
                }
            }
        }
    }
}
