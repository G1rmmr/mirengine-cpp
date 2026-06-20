#pragma once

#include <cassert>
#include <cmath>

#include "Simd.hpp"

namespace mir::math {
    struct alignas(16) Vector3 {
        union {
            struct {
                float X;
                float Y;
                float Z;
                float W;
            };

            simd::Floats V;
        };

        ~Vector3() noexcept = default;

        ENGINE_INLINE Vector3() noexcept : V(simd::Reset()) {}
        ENGINE_INLINE Vector3(const float val) noexcept : V(simd::Set(val)) {}
        ENGINE_INLINE Vector3(const simd::Floats v) noexcept : V(v) {}
        ENGINE_INLINE Vector3(const float x, const float y, const float z, const float w = 0.f) noexcept
            : V(simd::Set(x, y, z, w)) {}

        Vector3(const Vector3&) = default;
        Vector3(Vector3&&) = default;
        Vector3& operator=(const Vector3&) = default;
        Vector3& operator=(Vector3&&) = default;

        ENGINE_INLINE Vector3& ENGINE_VECTORCALL operator+=(const Vector3& other) noexcept {
            V = simd::Add(V, other.V);
            return *this;
        }

        ENGINE_INLINE Vector3 ENGINE_VECTORCALL operator+(Vector3 other) const noexcept { return other += *this; }

        ENGINE_INLINE Vector3& ENGINE_VECTORCALL operator-=(const Vector3& other) noexcept {
            V = simd::Sub(V, other.V);
            return *this;
        }

        ENGINE_INLINE Vector3 ENGINE_VECTORCALL operator-(Vector3 other) const noexcept {
            return Vector3(simd::Sub(V, other.V));
        }

        ENGINE_INLINE Vector3& ENGINE_VECTORCALL operator*=(const float val) noexcept {
            V = simd::Mul(V, simd::Set(val));
            return *this;
        }

        ENGINE_INLINE Vector3 ENGINE_VECTORCALL operator*(const float val) const noexcept {
            return Vector3(simd::Mul(V, simd::Set(val)));
        }

        ENGINE_INLINE Vector3& ENGINE_VECTORCALL operator/=(const float val) noexcept {
            assert(val != 0.f && "Division by zero!");

            const float inv = 1.f / val;
            V = simd::Mul(V, simd::Set(inv));
            return *this;
        }

        ENGINE_INLINE Vector3 ENGINE_VECTORCALL operator/(const float val) const noexcept {
            assert(val != 0.f && "Division by zero!");
            return Vector3(simd::Mul(V, simd::Set(1.f / val)));
        }

        ENGINE_INLINE bool ENGINE_VECTORCALL operator==(const Vector3& other) const noexcept {
            return simd::AllClose(V, other.V);
        }
        ENGINE_INLINE bool ENGINE_VECTORCALL operator!=(const Vector3& other) const noexcept {
            return !simd::AllClose(V, other.V);
        }

        ENGINE_INLINE Vector3 ENGINE_VECTORCALL Reciprocal() const noexcept { return Vector3(simd::Reciprocal(V)); }
        ENGINE_INLINE Vector3 ENGINE_VECTORCALL Sqrt() const noexcept { return Vector3(simd::Sqrt(V)); }

        ENGINE_INLINE float ENGINE_VECTORCALL Dot(const Vector3& other) const noexcept {
            return simd::GetFirst(simd::HorizonSum<0x71>(V, other.V));
        }

        ENGINE_INLINE float ENGINE_VECTORCALL Cross2D(const Vector3& other) const noexcept {
            return X * other.Y - Y * other.X;
        }

        ENGINE_INLINE Vector3 ENGINE_VECTORCALL Cross(const Vector3& other) const noexcept {
            const std::uint8_t leftMask = SIMD_MASK(3, 0, 2, 1);
            const std::uint8_t rightMask = SIMD_MASK(3, 1, 0, 2);

            Vector3 left{simd::Mul(simd::Shuffle<leftMask>(V, V), simd::Shuffle<rightMask>(other.V, other.V))};
            Vector3 right{simd::Mul(simd::Shuffle<rightMask>(V, V), simd::Shuffle<leftMask>(other.V, other.V))};

            return left - right;
        }

        ENGINE_INLINE float Length() const noexcept { return std::sqrt(Dot(*this)); }

        ENGINE_INLINE Vector3 Norm() const noexcept {
            const float len = Length();
            if (len > 1e-6f) return *this / len;
            return Vector3(0.f);
        }
    };
}
