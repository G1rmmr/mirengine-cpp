#pragma once

#include <cassert>
#include <cmath>

#include "Simd.hpp"
#include "Vector2.hpp"

namespace mir::math {
    struct alignas(16) Matrix3 {
        union {
            simd::Floats Cols[3]; // Column 0 (X axis), Column 1 (Y axis), Column 2 (Translation)
            float Elements[12];
        };

        ENGINE_INLINE float* __restrict operator[](int index) noexcept {
            return reinterpret_cast<float*>(&Cols[index]);
        }

        ENGINE_INLINE const float* __restrict operator[](int index) const noexcept {
            return reinterpret_cast<const float*>(&Cols[index]);
        }

        ~Matrix3() noexcept = default;

        ENGINE_INLINE Matrix3() noexcept {
            Cols[0] = simd::Set(1.f, 0.f, 0.f, 0.f);
            Cols[1] = simd::Set(0.f, 1.f, 0.f, 0.f);
            Cols[2] = simd::Set(0.f, 0.f, 0.f, 1.f); // Translation defaults to 0, W = 1
        }

        ENGINE_INLINE Matrix3(const float val) noexcept {
            for (std::size_t i = 0; i < 3; ++i) Cols[i] = simd::Set(val);
        }

        ENGINE_INLINE Matrix3(const Vector2& v1, const Vector2& v2) noexcept {
            Cols[0] = v1.V;
            Cols[1] = v2.V;
            Cols[2] = simd::Set(0.f, 0.f, 0.f, 1.f);
        }

        ENGINE_INLINE Matrix3(const Vector2& v1, const Vector2& v2, const Vector2& translation) noexcept {
            Cols[0] = v1.V;
            Cols[1] = v2.V;
            Cols[2] = simd::Set(translation.X, translation.Y, 0.f, 1.f);
        }

        ENGINE_INLINE Matrix3(const Matrix3& other) noexcept {
            for (std::size_t i = 0; i < 3; ++i) Cols[i] = other.Cols[i];
        }

        ENGINE_INLINE Matrix3(Matrix3&& other) noexcept {
            for (std::size_t i = 0; i < 3; ++i) Cols[i] = other.Cols[i];
        }

        Matrix3& operator=(const Matrix3&) = default;
        Matrix3& operator=(Matrix3&&) = default;

        ENGINE_INLINE Matrix3& ENGINE_VECTORCALL operator+=(const Matrix3& other) noexcept {
            for (std::size_t i = 0; i < 3; ++i) Cols[i] = simd::Add(Cols[i], other.Cols[i]);
            return *this;
        }

        ENGINE_INLINE Matrix3 ENGINE_VECTORCALL operator+(Matrix3 other) const noexcept { return other += *this; }

        ENGINE_INLINE Matrix3& ENGINE_VECTORCALL operator-=(const Matrix3& other) noexcept {
            for (std::size_t i = 0; i < 3; ++i) Cols[i] = simd::Sub(Cols[i], other.Cols[i]);
            return *this;
        }

        ENGINE_INLINE Matrix3 ENGINE_VECTORCALL operator-(Matrix3 other) const noexcept {
            Matrix3 temp(*this);
            return temp -= other;
        }

        ENGINE_INLINE Matrix3& ENGINE_VECTORCALL operator*=(const float val) noexcept {
            const simd::Floats temp = simd::Set(val);
            for (std::size_t i = 0; i < 3; ++i) Cols[i] = simd::Mul(Cols[i], temp);
            return *this;
        }

        ENGINE_INLINE Matrix3 ENGINE_VECTORCALL operator*(const float val) const noexcept {
            Matrix3 result(*this);
            return result *= val;
        }

        ENGINE_INLINE Matrix3& ENGINE_VECTORCALL operator*=(const Matrix3& other) noexcept {
            Matrix3 original = *this;
            for (int i = 0; i < 3; ++i) {
                simd::Floats col = other.Cols[i];
                simd::Floats rX = simd::Swizzle<SIMD_MASK(0, 0, 0, 0)>(col);
                simd::Floats rY = simd::Swizzle<SIMD_MASK(1, 1, 1, 1)>(col);

                simd::Floats res = simd::Mul(original.Cols[0], rX);
                res = simd::Add(res, simd::Mul(original.Cols[1], rY));
                if (i == 2) {
                    res = simd::Add(res, original.Cols[2]);
                }
                this->Cols[i] = res;
            }
            return *this;
        }

        ENGINE_INLINE Matrix3 ENGINE_VECTORCALL operator*(const Matrix3& other) const noexcept {
            Matrix3 res{*this};
            return res *= other;
        }

        ENGINE_INLINE Matrix3& ENGINE_VECTORCALL operator/=(const float val) noexcept {
            assert(val != 0.f && "Division by zero!");
            return *this *= 1.f / val;
        }

        ENGINE_INLINE Matrix3 ENGINE_VECTORCALL operator/(const float val) const noexcept {
            Matrix3 result(*this);
            return result /= val;
        }

        ENGINE_INLINE bool ENGINE_VECTORCALL operator==(const Matrix3& other) const noexcept {
            return std::abs(Elements[0] - other.Elements[0]) < 1e-5f &&
                   std::abs(Elements[1] - other.Elements[1]) < 1e-5f &&
                   std::abs(Elements[4] - other.Elements[4]) < 1e-5f &&
                   std::abs(Elements[5] - other.Elements[5]) < 1e-5f &&
                   std::abs(Elements[8] - other.Elements[8]) < 1e-5f &&
                   std::abs(Elements[9] - other.Elements[9]) < 1e-5f;
        }

        ENGINE_INLINE bool ENGINE_VECTORCALL operator!=(const Matrix3& other) const noexcept {
            return !(*this == other);
        }

        ENGINE_INLINE Matrix3 ENGINE_VECTORCALL Transpose() const noexcept {
            Matrix3 orig = *this;
            float temp = orig.Elements[1];
            orig.Elements[1] = orig.Elements[4];
            orig.Elements[4] = temp;
            return orig;
        }
    };
}
