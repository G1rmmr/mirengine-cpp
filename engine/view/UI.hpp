#pragma once

#include <string>

#include <SFML/Graphics/Color.hpp>

#include "Display.hpp"
#include "../core/Manager.hpp"
#include "../util/Types.hpp"
#include "../handle/Event.hpp"
#include "../core/Components.hpp"
#include "../core/Entity.hpp"

namespace mir{
    namespace ui{
        static inline void BuildText(
            const Tag tag,
            const sf::Color& color,
            const Point2<Real>& origin,
            const String& content,
            const Uint size){
            if(!font::Sources[tag]) return;

            font::Texts[tag]->setFillColor(color);
            font::Texts[tag]->setString(content);
            font::Texts[tag]->setCharacterSize(size);

            const Rect<Real> bounds = font::Texts[tag]->getLocalBounds();
            font::Texts[tag]->setOrigin({
                bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f   
            });

            font::Texts[tag]->setPosition(origin);
        }

        static inline ID BuildButton(
            const Action<const mir::event::type::MousePressed&>& onClick,
            const sf::Color& background,
            const Point2<Real>& pos,
            const Point2<Real>& size,
            const Tag tag = 0,
            const String& label = "",
            const sf::Color& foreground = {0, 0, 0, 0}){

            const ID id = entity::Create();
            if(id == 0) return 0;

            physics::Bounds[id] = size;

            transform::Positions[id] = {pos.x - size.x / 2.f, pos.y - size.y / 2.f};
            transform::Scales[id] = {1, 1};

            if(background.a != 0){
                sprite::Colors[id] = background;
                sprite::Types[id] = sprite::Type::Rectangle;
                texture::AllocFromType(id);
            }

            if(tag != 0 && label != "" && foreground.a != 0){
                const Point2<Real> center = pos;
                BuildText(tag, foreground, center, label, 30);
            }

            Action<const mir::event::type::MousePressed&> action
                = [id, pos, size, onClick](const event::type::MousePressed& e){
                if(!mir::Window) return;
                
                const Point2<Real> worldPos = mir::Window->mapPixelToCoords(Point2<Int>(e.X, e.Y));
                const Real cursorX = TypeCast<Real>(worldPos.x);
                const Real cursorY = TypeCast<Real>(worldPos.y);

                if(cursorX < pos.x - size.x / 2.f) return;
                if(cursorX > pos.x + size.x / 2.f) return;
                if(cursorY < pos.y - size.y / 2.f) return;
                if(cursorY > pos.y + size.y / 2.f) return;

                onClick(e);
            };
            event::Subscribe(action);

            return id;
        }
    }
}
