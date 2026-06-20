#pragma once

#include <cassert>
#include <cmath>

#include "Simd.hpp"

namespace mir::math {
    struct alignas(16) Vector2 {
        union {
            struct {
                float X;
                float Y;
            };

            simd::Floats V;
        };

        ~Vector2() noexcept = default;

        ENGINE_INLINE Vector2() noexcept : V(simd::Reset()) {}
        ENGINE_INLINE Vector2(const float val) noexcept : V(simd::Set(val, val, 0.f, 0.f)) {}
        ENGINE_INLINE Vector2(const simd::Floats v) noexcept : V(v) {}
        ENGINE_INLINE Vector2(const float x, const float y) noexcept
            : V(simd::Set(x, y, 0.f, 0.f)) {}

        Vector2(const Vector2&) = default;
        Vector2(Vector2&&) = default;
        Vector2& operator=(const Vector2&) = default;
        Vector2& operator=(Vector2&&) = default;

        ENGINE_INLINE Vector2& ENGINE_VECTORCALL operator+=(const Vector2& other) noexcept {
            V = simd::Add(V, other.V);
            return *this;
        }

        ENGINE_INLINE Vector2 ENGINE_VECTORCALL operator+(Vector2 other) const noexcept { return other += *this; }

        ENGINE_INLINE Vector2& ENGINE_VECTORCALL operator-=(const Vector2& other) noexcept {
            V = simd::Sub(V, other.V);
            return *this;
        }

        ENGINE_INLINE Vector2 ENGINE_VECTORCALL operator-(Vector2 other) const noexcept {
            return Vector2(simd::Sub(V, other.V));
        }

        ENGINE_INLINE Vector2& ENGINE_VECTORCALL operator*=(const float val) noexcept {
            V = simd::Mul(V, simd::Set(val));
            return *this;
        }

        ENGINE_INLINE Vector2 ENGINE_VECTORCALL operator*(const float val) const noexcept {
            return Vector2(simd::Mul(V, simd::Set(val)));
        }

        ENGINE_INLINE Vector2& ENGINE_VECTORCALL operator/=(const float val) noexcept {
            assert(val != 0.f && "Division by zero!");

            const float inv = 1.f / val;
            V = simd::Mul(V, simd::Set(inv));
            return *this;
        }

        ENGINE_INLINE Vector2 ENGINE_VECTORCALL operator/(const float val) const noexcept {
            assert(val != 0.f && "Division by zero!");
            return Vector2(simd::Mul(V, simd::Set(1.f / val)));
        }

        ENGINE_INLINE bool ENGINE_VECTORCALL operator==(const Vector2& other) const noexcept {
            return std::abs(X - other.X) < 1e-5f && std::abs(Y - other.Y) < 1e-5f;
        }
        ENGINE_INLINE bool ENGINE_VECTORCALL operator!=(const Vector2& other) const noexcept {
            return !(*this == other);
        }

        ENGINE_INLINE Vector2 ENGINE_VECTORCALL Reciprocal() const noexcept { return Vector2(simd::Reciprocal(V)); }
        ENGINE_INLINE Vector2 ENGINE_VECTORCALL Sqrt() const noexcept { return Vector2(simd::Sqrt(V)); }

        ENGINE_INLINE float ENGINE_VECTORCALL Dot(const Vector2& other) const noexcept {
            return simd::GetFirst(simd::HorizonSum<0x31>(V, other.V));
        }

        ENGINE_INLINE float ENGINE_VECTORCALL Cross(const Vector2& other) const noexcept {
            return X * other.Y - Y * other.X;
        }

        ENGINE_INLINE float Length() const noexcept { return std::sqrt(Dot(*this)); }

        ENGINE_INLINE Vector2 Norm() const noexcept {
            const float len = Length();
            if (len > 1e-6f) return *this / len;
            return Vector2(0.f);
        }
    };
}
