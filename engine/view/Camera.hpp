#pragma once

#include <SFML/System/Vector2.hpp>

#include "../core/Entity.hpp"
#include "../util/Math.hpp"
#include "../util/Types.hpp"
#include "Display.hpp"

namespace mir{
    namespace camera{
        namespace{
            inline sf::View View;

            inline Real CurrentZoom = 1;
            inline Real ShakeTime = 0;
            inline Real ShakePower = 0;

            inline ID TargetID = 0;
        }

        static inline Point2<Real> GetCenter(){ return View.getCenter(); }
        static inline void Follow(ID target){ TargetID = target; }
        static inline void SetZoom(Real zoom){ CurrentZoom = zoom; }

        static inline void SetPosition(const Point2<Real>& pos){
            TargetID = 0;
            View.setCenter(pos);
        }

        static inline void SetPosition(Real x, Real y){
            TargetID = 0;
            View.setCenter({x, y});
        }

        static inline void Shake(Real intensity, Real duration){
            ShakePower = intensity;
            ShakeTime = duration;
        }

        static inline void Init(){
            TargetID = 0;

            if(Window){
                const Point2<Real> size = TypeCast<Point2<Real>>(Window->getSize());
                View.setSize(size);
                View.setCenter({size.x / 2, size.y / 2});
            }
        }

        static inline void Update(const Real deltaTime){
            Point2<Real> start = View.getCenter();
            Point2<Real> end = start;

            if(TargetID != 0 && entity::IsAvailables[TargetID]){
                Point2<Real> size = physics::Bounds[TargetID];
                Point2<Real> target = {
                    transform::Positions[TargetID].x + size.x / 2,
                    transform::Positions[TargetID].y + size.y / 2,
                };

                Real interval = deltaTime * 5.0f;
                if(interval > 1.0f) interval = 1.0f;

                end = math::Lerp(start, target, interval);
            }

            const Point2<Real> center = View.getCenter();
            if(center.x < end.x + 5 &&
                center.y > end.y - 5 &&
                center.x > end.x - 5 &&
                center.y < end.y + 5 &&
                !physics::InAirFlags[TargetID])
                TargetID = 0;

            if(ShakeTime > 0){
                ShakeTime -= deltaTime;
                Real offsetX = math::GetRandomReal(-50, 50) * ShakePower;
                Real offsetY = math::GetRandomReal(-50, 50) * ShakePower;
                end += Point2<Real>(offsetX, offsetY);
            }

            View.setCenter(end);

            if(Window) Window->setView(View);
        }
    }
}
