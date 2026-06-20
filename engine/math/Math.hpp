#pragma once

#include <cmath>
#include <numbers>
#include <random>

#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Matrix3.hpp"
#include "Matrix4.hpp"
#include "Quaternion.hpp"

namespace mir::math {
    static constexpr float PI = std::numbers::pi;

    namespace {
        ENGINE_INLINE std::mt19937& GetRandomEngine() {
            static thread_local std::random_device rd;
            static thread_local std::mt19937 gen(rd());
            return gen;
        }
    }

    ENGINE_INLINE float RandomFloat(const float min, const float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(GetRandomEngine());
    }

    ENGINE_INLINE Vector2 CreateRandomVector2(const float min, const float max) {
        return Vector2(RandomFloat(min, max), RandomFloat(min, max));
    }

    ENGINE_INLINE Vector3 CreateRandomVector3(const float min, const float max) {
        return Vector3(RandomFloat(min, max), RandomFloat(min, max), RandomFloat(min, max), 1.f);
    }

    ENGINE_INLINE Matrix3 ENGINE_VECTORCALL CreateRandomMatrix3(const float min, const float max) {
        Matrix3 mat;
        for (int i = 0; i < 3; ++i) {
            mat.Cols[i] =
                simd::Set(RandomFloat(min, max), RandomFloat(min, max), 0.f, i == 2 ? 1.f : 0.f);
        }
        return mat;
    }

    ENGINE_INLINE Matrix4 ENGINE_VECTORCALL CreateRandomMatrix4(const float min, const float max) {
        Matrix4 mat;
        for (int i = 0; i < 4; ++i) {
            mat.Cols[i] =
                simd::Set(RandomFloat(min, max), RandomFloat(min, max), RandomFloat(min, max), RandomFloat(min, max));
        }
        return mat;
    }

    // Matrix * Vector multiplications
    ENGINE_INLINE Vector3 ENGINE_VECTORCALL operator*(const Matrix4& mat, const Vector3& vec) noexcept {
        simd::Floats res = simd::Mul(mat.Cols[0], simd::Set(vec.X));
        res = simd::Add(res, simd::Mul(mat.Cols[1], simd::Set(vec.Y)));
        res = simd::Add(res, simd::Mul(mat.Cols[2], simd::Set(vec.Z)));
        res = simd::Add(res, simd::Mul(mat.Cols[3], simd::Set(vec.W)));
        return Vector3(res);
    }

    ENGINE_INLINE Vector2 ENGINE_VECTORCALL operator*(const Matrix3& mat, const Vector2& vec) noexcept {
        simd::Floats res = simd::Mul(mat.Cols[0], simd::Set(vec.X));
        res = simd::Add(res, simd::Mul(mat.Cols[1], simd::Set(vec.Y)));
        res = simd::Add(res, mat.Cols[2]); // Assumes W = 1 for 2D position translation
        return Vector2(res);
    }

    ENGINE_INLINE Quaternion ENGINE_VECTORCALL FromAxisAngle(const Vector3& axis, const float radian) noexcept {
        return Quaternion(axis.Norm() * std::sin(radian * 0.5f), std::cos(radian * 0.5f));
    }

    ENGINE_INLINE Vector3 ENGINE_VECTORCALL GetBarycentric(const Vector3& pos, const Vector3& a, const Vector3& b,
                                                           const Vector3& c) noexcept {
        const float area = (b - a).Cross2D(c - a);

        if (std::abs(area) < 1e-6f) [[unlikely]]
            return Vector3(-1.f, -1.f, -1.f, 0.f);

        const float invArea = 1.f / area;

        const float wA = (b - pos).Cross2D(c - pos) * invArea;
        const float wB = (c - pos).Cross2D(a - pos) * invArea;
        const float wC = 1.f - wA - wB;

        return Vector3(wA, wB, wC, 0.f);
    }

    ENGINE_INLINE Vector2 ENGINE_VECTORCALL GetBarycentric(const Vector2& pos, const Vector2& a, const Vector2& b,
                                                           const Vector2& c) noexcept {
        const float area = (b - a).Cross(c - a);

        if (std::abs(area) < 1e-6f) [[unlikely]]
            return Vector2(-1.f, -1.f);

        const float invArea = 1.f / area;

        const float wA = (b - pos).Cross(c - pos) * invArea;
        const float wB = (c - pos).Cross(a - pos) * invArea;
        const float wC = 1.f - wA - wB;

        return Vector2(wA, wB);
    }

    ENGINE_INLINE Matrix4 ENGINE_VECTORCALL CreateViewport(const std::uint32_t screenWidth,
                                                           const std::uint32_t screenHeight) {
        const float width = static_cast<float>(screenWidth);
        const float height = static_cast<float>(screenHeight);

        Matrix4 mat;
        mat[0][0] = width * 0.5f;
        mat[1][1] = -height * 0.5f;
        mat[2][2] = 1.f;
        mat[3][0] = width * 0.5f;
        mat[3][1] = height * 0.5f;
        mat[3][3] = 1.f;

        return mat;
    }

    ENGINE_INLINE Matrix4 ENGINE_VECTORCALL CreateLookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
        const Vector3 z = (eye - target).Norm();
        const Vector3 x = up.Cross(z).Norm();
        const Vector3 y = z.Cross(x);

        Matrix4 mat;
        mat.Cols[0] = simd::Set(x.X, y.X, z.X, 0.f);
        mat.Cols[1] = simd::Set(x.Y, y.Y, z.Y, 0.f);
        mat.Cols[2] = simd::Set(x.Z, y.Z, z.Z, 0.f);
        mat.Cols[3] = simd::Set(-x.Dot(eye), -y.Dot(eye), -z.Dot(eye), 1.f);

        return mat;
    }

    ENGINE_INLINE Matrix4 ENGINE_VECTORCALL CreatePerspective(const float fov, const float aspect, const float near,
                                                              const float far) {
        const float tanHalfFov = std::tan(fov * 0.5f);

        Matrix4 mat(0.f);
        mat[0][0] = 1.f / (aspect * tanHalfFov);
        mat[1][1] = 1.f / tanHalfFov;
        mat[2][2] = far / (near - far);
        mat[2][3] = -1.f;
        mat[3][2] = (far * near) / (near - far);
        mat[3][3] = 0.f;

        return mat;
    }

    ENGINE_INLINE Matrix4 ENGINE_VECTORCALL CreateOrtho(float left, float right, float bottom, float top, float near,
                                                        float far) {
        Matrix4 mat(0.f);
        mat[0][0] = 2.f / (right - left);
        mat[1][1] = 2.f / (top - bottom);
        mat[2][2] = -2.f / (far - near);
        mat[0][3] = -(right + left) / (right - left);
        mat[1][3] = -(top + bottom) / (top - bottom);
        mat[2][3] = -(far + near) / (far - near);
        mat[3][3] = 1.f;

        return mat;
    }

    ENGINE_INLINE Matrix4 ENGINE_VECTORCALL CreateScale(const Vector3& scale) {
        Matrix4 mat;
        mat[0][0] = scale.X;
        mat[1][1] = scale.Y;
        mat[2][2] = scale.Z;
        return mat;
    }

    ENGINE_INLINE Matrix3 ENGINE_VECTORCALL CreateScale(const Vector2& scale) {
        Matrix3 mat;
        mat[0][0] = scale.X;
        mat[1][1] = scale.Y;
        return mat;
    }

    ENGINE_INLINE Matrix4 ENGINE_VECTORCALL CreateRotation(const Vector3& axis, const float radian) {
        return FromAxisAngle(axis, radian).ToMatrix();
    }

    ENGINE_INLINE Matrix3 ENGINE_VECTORCALL CreateRotation(const float radian) {
        const float c = std::cos(radian);
        const float s = std::sin(radian);
        Matrix3 mat;
        mat.Cols[0] = simd::Set(c, s, 0.f, 0.f);
        mat.Cols[1] = simd::Set(-s, c, 0.f, 0.f);
        mat.Cols[2] = simd::Set(0.f, 0.f, 0.f, 1.f);
        return mat;
    }

    ENGINE_INLINE Matrix4 ENGINE_VECTORCALL CreateTranslation(const Vector3& position) {
        Matrix4 mat;
        mat.Cols[3] = simd::Set(position.X, position.Y, position.Z, 1.f);
        return mat;
    }

    ENGINE_INLINE Matrix3 ENGINE_VECTORCALL CreateTranslation(const Vector2& position) {
        Matrix3 mat;
        mat.Cols[2] = simd::Set(position.X, position.Y, 0.f, 1.f);
        return mat;
    }

    ENGINE_INLINE constexpr float ToRadian(const float degree) noexcept {
        return degree * (std::numbers::pi_v<float> / 180.f);
    }

    ENGINE_INLINE constexpr float ToDegree(const float radian) noexcept {
        return radian * (180.f / std::numbers::pi_v<float>);
    }

    ENGINE_INLINE float Smoothstep(const float edge0, const float edge1, const float x) noexcept {
        const float t = std::clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f);
        return t * t * (3.f - 2.f * t);
    }

    ENGINE_INLINE float Cos(const float angle) noexcept { return std::cos(ToRadian(angle)); }

    ENGINE_INLINE float Sin(const float angle) noexcept { return std::sin(ToRadian(angle)); }

    ENGINE_INLINE float Tan(const float angle) noexcept { return std::tan(ToRadian(angle)); }

    ENGINE_INLINE float CosH(const float angle) noexcept { return std::cosh(ToRadian(angle)); }

    ENGINE_INLINE float SinH(const float angle) noexcept { return std::sinh(ToRadian(angle)); }

    ENGINE_INLINE float TanH(const float angle) noexcept { return std::tanh(ToRadian(angle)); }

    ENGINE_INLINE float Lerp(const float start, const float end, const float interval) noexcept {
        return (1.f - interval) * start + interval * end;
    }

    ENGINE_INLINE Point2<float> Lerp(const Point2<float>& start, const Point2<float>& end, const float interval) {
        return {Lerp(start.x, end.x, interval), Lerp(start.y, end.y, interval)};
    }

    ENGINE_INLINE Color Lerp(const Color& start, const Color& end, const float interval) {
        return {TypeCast<Byte>(Lerp(start.r, end.r, interval)), TypeCast<Byte>(Lerp(start.g, end.g, interval)),
                TypeCast<Byte>(Lerp(start.b, end.b, interval))};
    }

    ENGINE_INLINE Int GetRandomInt(const Int start, const Int end) {
        std::uniform_int_distribution<Int> dist(start, end);
        return dist(GetRandomEngine());
    }

    ENGINE_INLINE Point2<float> GetRandomPoint(const Point2<float>& start, const Point2<float>& end) {
        return {RandomFloat(start.x, end.x), RandomFloat(start.y, end.y)};
    }

    ENGINE_INLINE Color GetRandomColor(const Color& start, const Color& end) {
        return {TypeCast<Byte>(GetRandomInt(start.r, end.r)), TypeCast<Byte>(GetRandomInt(start.g, end.g)),
                TypeCast<Byte>(GetRandomInt(start.b, end.b))};
    }
}