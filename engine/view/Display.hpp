#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowEnums.hpp>

#include "../core/Component.hpp"
#include "../core/Entity.hpp"
#include "../core/Manager.hpp"
#include "../util/Debugger.hpp"
#include "../util/Profiler.hpp"
#include "../util/Types.hpp"
#include "../handle/Input.hpp"

namespace mir{
    inline HandledWindow* Window = nullptr;

    enum class VideoMode{
        Windowed,
        Fullscreen,
        Borderless,
        Desktop
    };

    enum class Resolution{
        HD,  // 1280x720
        FHD, // 1920x1080
        QHD, // 2560x1440
        UHD, // 3840x2160
        Custom
    };

    namespace window{
        inline Key MapKey(sf::Keyboard::Key sfKey) {
            switch (sfKey) {
                case sf::Keyboard::Key::Escape:   return Key::Escape;
                case sf::Keyboard::Key::Space:    return Key::Space;
                case sf::Keyboard::Key::Enter:    return Key::Enter;
                case sf::Keyboard::Key::Up:       return Key::Up;
                case sf::Keyboard::Key::Left:     return Key::Left;
                case sf::Keyboard::Key::Down:     return Key::Down;
                case sf::Keyboard::Key::Right:    return Key::Right;
                case sf::Keyboard::Key::Tab:      return Key::Tab;
                case sf::Keyboard::Key::LShift:   return Key::Lshift;
                case sf::Keyboard::Key::LControl: return Key::Lctrl;
                case sf::Keyboard::Key::LAlt:     return Key::Lalt;
                case sf::Keyboard::Key::RShift:   return Key::Rshift;
                case sf::Keyboard::Key::RControl: return Key::Rctrl;
                case sf::Keyboard::Key::RAlt:     return Key::Ralt;
                
                case sf::Keyboard::Key::A: return Key::A;
                case sf::Keyboard::Key::B: return Key::B;
                case sf::Keyboard::Key::C: return Key::C;
                case sf::Keyboard::Key::D: return Key::D;
                case sf::Keyboard::Key::E: return Key::E;
                case sf::Keyboard::Key::F: return Key::F;
                case sf::Keyboard::Key::G: return Key::G;
                case sf::Keyboard::Key::H: return Key::H;
                case sf::Keyboard::Key::I: return Key::I;
                case sf::Keyboard::Key::J: return Key::J;
                case sf::Keyboard::Key::K: return Key::K;
                case sf::Keyboard::Key::L: return Key::L;
                case sf::Keyboard::Key::M: return Key::M;
                case sf::Keyboard::Key::N: return Key::N;
                case sf::Keyboard::Key::O: return Key::O;
                case sf::Keyboard::Key::P: return Key::P;
                case sf::Keyboard::Key::Q: return Key::Q;
                case sf::Keyboard::Key::R: return Key::R;
                case sf::Keyboard::Key::S: return Key::S;
                case sf::Keyboard::Key::T: return Key::T;
                case sf::Keyboard::Key::U: return Key::U;
                case sf::Keyboard::Key::V: return Key::V;
                case sf::Keyboard::Key::W: return Key::W;
                case sf::Keyboard::Key::X: return Key::X;
                case sf::Keyboard::Key::Y: return Key::Y;
                case sf::Keyboard::Key::Z: return Key::Z;

                case sf::Keyboard::Key::F1: return Key::F1;
                case sf::Keyboard::Key::F2: return Key::F2;
                case sf::Keyboard::Key::F3: return Key::F3;
                case sf::Keyboard::Key::F4: return Key::F4;
                case sf::Keyboard::Key::F5: return Key::F5;
                case sf::Keyboard::Key::F6: return Key::F6;
                case sf::Keyboard::Key::F7: return Key::F7;
                case sf::Keyboard::Key::F8: return Key::F8;
                case sf::Keyboard::Key::F9: return Key::F9;
                case sf::Keyboard::Key::F10: return Key::F10;
                case sf::Keyboard::Key::F11: return Key::F11;
                case sf::Keyboard::Key::F12: return Key::F12;
                default: return Key::COUNT;
            }
        }

        inline void ProcessEvents() {
            if (!Window) return;
            
            mir::input::Update();

            while (const auto event = Window->pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    Close();
                }
                else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                    mir::Key targetKey = MapKey(key->code);
                    if (targetKey != mir::Key::COUNT) {
                        mir::input::SetKeyState(targetKey, true);
                    }
                }
                else if (const auto* key = event->getIf<sf::Event::KeyReleased>()) {
                    mir::Key targetKey = MapKey(key->code);
                    if (targetKey != mir::Key::COUNT) {
                        mir::input::SetKeyState(targetKey, false);
                    }
                }
                else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                    std::size_t btnIdx = 0;
                    if (mouse->button == sf::Mouse::Button::Right) btnIdx = 1;
                    else if (mouse->button == sf::Mouse::Button::Middle) btnIdx = 2;
                    mir::input::SetMouseState(btnIdx, true);
                    mir::input::SetMousePosition(static_cast<float>(mouse->position.x), static_cast<float>(mouse->position.y));
                }
                else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
                    std::size_t btnIdx = 0;
                    if (mouse->button == sf::Mouse::Button::Right) btnIdx = 1;
                    else if (mouse->button == sf::Mouse::Button::Middle) btnIdx = 2;
                    mir::input::SetMouseState(btnIdx, false);
                    mir::input::SetMousePosition(static_cast<float>(mouse->position.x), static_cast<float>(mouse->position.y));
                }
            }
        }

        static inline Bool IsOpening() { return Window && Window->isOpen(); }

        static inline void Display(){
            if(Window) Window->display();
        }

        static inline void SetFPS(const Uint fps){
            if(Window) {
                Window->setFramerateLimit(fps);
                Window->setVerticalSyncEnabled(true);
            }
        }

        static inline void Close(){
            if(Window) Window->close();
        }

        static inline void Clear(Uint red, Uint green, Uint blue){
            if(Window) Window->clear(sf::Color(red, green, blue));
        }

        static inline void Shutdown(){
            debug::Text.reset();
            debug::Font.reset();
            profile::Text.reset();
            profile::Font.reset();

            if(Window){
                Window->close();
                delete Window;
                Window = nullptr;
            }
        }

        static inline void Init(
            const String& title,
            VideoMode mode = VideoMode::Desktop,
            Resolution res = Resolution::FHD,
            uint32_t width = 0,
            uint32_t height = 0){
            if(Window) return;
            sf::VideoMode video;

            switch(res){
            case Resolution::HD:
                video = sf::VideoMode({1280, 720});
                break;

            case Resolution::FHD:
                video = sf::VideoMode({1920, 1080});
                break;

            case Resolution::QHD:
                video = sf::VideoMode({2560, 1440});
                break;

            case Resolution::UHD:
                video = sf::VideoMode({3840, 2160});
                break;

            case Resolution::Custom:
                video = sf::VideoMode({width, height});
                break;

            default:
                video = sf::VideoMode::getDesktopMode();
                break;
            }

            switch(mode){
            case VideoMode::Borderless:
                video = sf::VideoMode::getDesktopMode();
                Window = new sf::RenderWindow(video, title.c_str(), sf::Style::None, sf::State::Windowed);
                Window->setPosition({0, 0});
                break;

            case VideoMode::Fullscreen:
                Window = new sf::RenderWindow(video, title.c_str(), sf::State::Fullscreen);
                break;

            case VideoMode::Desktop:
                video = sf::VideoMode::getDesktopMode();
                Window = new sf::RenderWindow(video, title.c_str(), sf::State::Windowed);
                break;

            default:
                Window = new sf::RenderWindow(video, title.c_str(), sf::State::Windowed);
                break;
            }
        }

        namespace{
            static inline void BuildSprite(const ID id, sf::Sprite& sprite){
                sprite.setPosition(transform::Positions[id]);
                sprite.setRotation(sf::degrees(transform::Rotations[id]));
                sprite.setColor(sprite::Colors[id]);

                Point2<Real> finalScale = transform::Scales[id];
                if(sprite::ShouldFlipXs[id]) finalScale.x = -finalScale.x;
                if(sprite::ShouldFlipYs[id]) finalScale.y = -finalScale.y;

                sprite.setScale(finalScale);
            }

            static inline void DrawSprites(){
                for(ID id = 1; id < MAX_ENTITIES; ++id){
                    if(!sprite::Textures[id]) continue;

                    const List<sf::Rect<Int>>& frames
                        = animation::FrameSets[animation::States[id]];

                    sf::Sprite sprite = frames.empty() ?
                        sf::Sprite(*sprite::Textures[id]) :
                        sf::Sprite(*sprite::Textures[id], frames[animation::CurrFrames[id]]);

                    BuildSprite(id, sprite);
                    Window->draw(sprite);
                }
            }

            static inline void DrawParticles(){
                for(ID id = 1; id < MAX_ENTITIES; ++id){
                    if(particle::Positions[id].empty()) continue;

                    const size_t count = particle::Positions[id].size();
                    PointArr vertexArr(sf::PrimitiveType::Triangles, count * 6);

                    for (size_t i = 0; i < count; ++i) {
                        const Point2<Real>& pos = particle::Positions[id][i];
                        const Real size = particle::CurrentSizes[id][i];
                        const sf::Color& color = particle::CurrentColors[id][i];

                        const Real halfSize = size * 0.5f;

                        sf::Vector2f tl(pos.x - halfSize, pos.y - halfSize);
                        sf::Vector2f tr(pos.x + halfSize, pos.y - halfSize);
                        sf::Vector2f br(pos.x + halfSize, pos.y + halfSize);
                        sf::Vector2f bl(pos.x - halfSize, pos.y + halfSize);

                        vertexArr[i * 6 + 0].position = tl; vertexArr[i * 6 + 0].color = color;
                        vertexArr[i * 6 + 1].position = tr; vertexArr[i * 6 + 1].color = color;
                        vertexArr[i * 6 + 2].position = br; vertexArr[i * 6 + 2].color = color;

                        vertexArr[i * 6 + 3].position = br; vertexArr[i * 6 + 3].color = color;
                        vertexArr[i * 6 + 4].position = bl; vertexArr[i * 6 + 4].color = color;
                        vertexArr[i * 6 + 5].position = tl; vertexArr[i * 6 + 5].color = color;
                    }
                    Window->draw(vertexArr);
                }
            }

            static inline void DrawTexts(){
                for(Tag tag = 1; tag < MAX_RESOURCES; ++tag){
                    if(font::Sources[tag])
                        Window->draw(*font::Texts[tag]);
                }
            }

            static inline void DrawDebug(){
                if(!Window) return;

                if(!debug::Font){
                    debug::Font = std::make_unique<Font>();
                    if(debug::Font->openFromFile("assets/fonts/dieproud.ttf")){
                        debug::Text = std::make_unique<Text>(*debug::Font);
                        debug::Text->setCharacterSize(24);
                        debug::Text->setFillColor(sf::Color::Yellow);
                    }
                }

                if constexpr(DEBUG_ENABLED) {
                    if(mir::debug::IsEntityCountVisible && debug::Text){
                        sf::View oldView = Window->getView();
                        Window->setView(Window->getDefaultView());

                        mir::ID count = 0;
                        for(Bool available : entity::IsAvailables)
                            if(available) count++;

                        debug::Text->setString((String("[Toggle - F1] Entities: ") + ToString(count)).c_str());
                        debug::Text->setPosition({10.f, 10.f});

                        Window->draw(*debug::Text);
                        Window->setView(oldView);
                    }

                    if(mir::debug::IsColliderVisible){
                        for(ID id = 1; id < MAX_ENTITIES; ++id){
                            if(!entity::IsAvailables[id]) continue;

                            const Point2<Real>& size = physics::Bounds[id];
                            if(size.x == 0 && size.y == 0) continue;

                            sf::RectangleShape rect(size);
                            rect.setPosition(transform::Positions[id]);
                            rect.setFillColor(sf::Color::Transparent);
                            rect.setOutlineColor(sf::Color::Red);
                            rect.setOutlineThickness(1);

                            Window->draw(rect);
                        }
                    }
                }
            }

            static inline void DrawProfile(){
                if(!Window) return;

                if(!profile::Font){
                    profile::Font = std::make_unique<Font>();
                    if(profile::Font->openFromFile("assets/fonts/dieproud.ttf")){
                        profile::Text = std::make_unique<Text>(*profile::Font);
                        profile::Text->setCharacterSize(24);
                        profile::Text->setFillColor(sf::Color::Yellow);
                    }
                }

                if(mir::profile::IsEnable && profile::Text){
                    sf::View oldView = Window->getView();
                    Window->setView(Window->getDefaultView());

                    const Real fps = mir::profile::CurrentFPS;
                    profile::Text->setString((String("[Toggle - F2] FPS: ") + ToString(TypeCast<Int>(fps))).c_str());
                    profile::Text->setPosition({10.f, 40.f});

                    Window->draw(*profile::Text);
                    Window->setView(oldView);
                }
            }
        }

        static inline void Render(){
            if(!Window) return;

            DrawSprites();
            DrawParticles();
            DrawTexts();
            DrawDebug();
            DrawProfile();
        }
    }
}
