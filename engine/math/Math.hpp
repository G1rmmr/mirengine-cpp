#pragma once

#include <random>
#include <cmath>
#include <numbers>

#include "Types.hpp"

namespace mir{
    namespace math{
        static constexpr Real PI = std::numbers::pi;

        static inline Real Cos(const Real angle){
            return std::cos(angle * (PI / 180));
        }

        static inline Real Sin(const Real angle){
            return std::sin(angle * (PI / 180));
        }

        static inline Real Tan(const Real angle){
            return std::tan(angle * (PI / 180));
        }

        static inline Real CosH(const Real angle){
            return std::cosh(angle * (PI / 180));
        }

        static inline Real SinH(const Real angle){
            return std::sinh(angle * (PI / 180));
        }

        static inline Real TanH(const Real angle){
            return std::tanh(angle * (PI / 180));
        }

        static inline Real Lerp(
            const Real start, const Real end, const Real interval){
            return (1 - interval) * start + interval * end;
        }

        static inline Point2<Real> Lerp(
            const Point2<Real>& start, const Point2<Real>& end, const Real interval){
            return {Lerp(start.x, end.x, interval), Lerp(start.y, end.y, interval)};
        }

        static inline Color Lerp(
            const Color& start, const Color& end, const Real interval){
            return {
                TypeCast<Byte>(Lerp(start.r, end.r, interval)),
                TypeCast<Byte>(Lerp(start.g, end.g, interval)),
                TypeCast<Byte>(Lerp(start.b, end.b, interval))
            };
        }

        static inline Int GetRandomInt(const Int start, const Int end){
            std::random_device random;
            std::mt19937 gen(random());

            std::uniform_int_distribution<Int> dist(start, end);
            return dist(random);
        }

        static inline Real GetRandomReal(const Real start, const Real end){
            std::random_device random;
            std::mt19937 gen(random());

            std::uniform_real_distribution<Real> dist(start, end);
            return dist(random);
        }

        static inline Point2<Real> GetRandomPoint(
            const Point2<Real>& start, const Point2<Real>& end){
            return {GetRandomReal(start.x, end.x), GetRandomReal(start.y, end.y)};
        }

        static inline Color GetRandomColor(
            const Color& start, const Color& end){
            return {
                TypeCast<Byte>(GetRandomInt(start.r, end.r)),
                TypeCast<Byte>(GetRandomInt(start.g, end.g)),
                TypeCast<Byte>(GetRandomInt(start.b, end.b))
            };
        }
    }
}
