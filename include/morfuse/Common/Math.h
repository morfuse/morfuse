#pragma once

#include "../Global.h"
#include <cstdint>
#include <climits>
#include <type_traits>

namespace mfuse
{
    using vec_t = float;
    using vec2_t = float[2];
    using vec3_t = float[3];
    using vec4_t = float[4];
    using quat_t = vec4_t;
    using matrix_t = vec_t[16];


    static constexpr float M_PI_FLOAT = 3.14159265358979323846f;
    static constexpr unsigned int NUMVERTEXNORMALS = 162;
    
    extern vec3_t vec3_origin;
    extern vec3_t bytedirs[NUMVERTEXNORMALS];

    class Vector;

    mfuse_EXPORTS float DegreesToRadians(float angle);
    mfuse_EXPORTS float RadiansToDegrees(float rad);
    mfuse_EXPORTS vec_t DotProduct(const vec3_t vec1, const vec3_t vec2);
    mfuse_EXPORTS vec_t DotProduct2D(const vec2_t vec1, const vec2_t vec2);
    mfuse_EXPORTS vec_t DotProduct4(const vec4_t vec1, const vec4_t vec2);
    mfuse_EXPORTS double vrsqrt(double number);
    mfuse_EXPORTS float vrsqrtf(float number);
    mfuse_EXPORTS float AngleNormalize360(float angle);
    mfuse_EXPORTS float AngleNormalize180(float angle);
    mfuse_EXPORTS float AngleMod(float a);
    mfuse_EXPORTS float AngleSubtract(float a1, float a2);
    mfuse_EXPORTS void AnglesSubtract(vec3_t v1, vec3_t v2, vec3_t v3);
    mfuse_EXPORTS float LerpAngle(float from, float to, float frac);
    mfuse_EXPORTS void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
    mfuse_EXPORTS void AngleVectorsLeft(const vec3_t angles, vec3_t forward, vec3_t left, vec3_t up);
    mfuse_EXPORTS void AnglesToAxis(float angles[3], float axis[3][3]);
    mfuse_EXPORTS void MatToQuat(float srcMatrix[3][3], quat_t destQuat);
    mfuse_EXPORTS void EulerToQuat(float ang[3], float q[4]);
    mfuse_EXPORTS void QuatToMat(const float q[4], float m[3][3]);
    mfuse_EXPORTS void QuatSet(quat_t q, float x, float y, float z, float w);
    mfuse_EXPORTS void QuatClear(quat_t q);
    mfuse_EXPORTS void QuatInverse(quat_t q);
    mfuse_EXPORTS void QuatToAngles(const quat_t q, vec3_t angles);
    mfuse_EXPORTS void QuatNormalize(quat_t quat);
    mfuse_EXPORTS void MatrixToEulerAngles(const float mat[3][3], vec3_t ang);
    mfuse_EXPORTS void MatrixMultiply(const float in1[3][3], const float in2[3][3], float out[3][3]);
    mfuse_EXPORTS void MatrixTransformVector(const vec3_t in, const float mat[3][3], vec3_t out);
    mfuse_EXPORTS void TransposeMatrix(float in[3][3], float out[3][3]);
    mfuse_EXPORTS void MatrixCopy(const matrix_t in, matrix_t out);
    mfuse_EXPORTS void Matrix3Copy(float in[3][3], float out[3][3]);
    mfuse_EXPORTS void Matrix4_3Copy(float in[4][3], float out[3][3]);
    mfuse_EXPORTS void AddPointToBounds(const Vector& v, Vector& mins, Vector& maxs);
    mfuse_EXPORTS void ClearBounds(Vector& mins, Vector& maxs);
    mfuse_EXPORTS void AxisClear(vec3_t axis[3]);
    mfuse_EXPORTS void AxisCopy(const vec3_t in[3], vec3_t out[3]);
    mfuse_EXPORTS void SnapVector(vec3_t normal);
    mfuse_EXPORTS void VecCopy(const vec3_t in, vec3_t out);
    mfuse_EXPORTS void VecSet(vec3_t out, float x, float y, float z);
    mfuse_EXPORTS void Vec4Copy(const vec4_t in, vec4_t out);
    mfuse_EXPORTS void VecSubtract(const vec3_t veca, const vec3_t vecb, vec3_t out);
    mfuse_EXPORTS void VecAdd(const vec3_t veca, const vec3_t vecb, vec3_t out);
    mfuse_EXPORTS bool VecCompare(const vec3_t veca, const vec3_t vecb);
    mfuse_EXPORTS bool VecCompare(const vec3_t veca, const vec3_t vecb, float tolerance);
    mfuse_EXPORTS bool Vec4Compare(const vec4_t veca, const vec4_t vecb);
    mfuse_EXPORTS bool Vec4Compare(const vec4_t veca, const vec4_t vecb, float tolerance);
    mfuse_EXPORTS void VecMatrixInverse(void* DstMatrix, const void* SrcMatrix);
    mfuse_EXPORTS vec_t VectorLength(const vec3_t vec);
    mfuse_EXPORTS vec_t VectorLengthSquared(const vec3_t vec);
    mfuse_EXPORTS vec_t VectorNormalize(vec3_t vec);
    mfuse_EXPORTS vec_t VectorNormalize2(const vec3_t v, vec3_t out);
    mfuse_EXPORTS void VectorNormalizeFast(vec3_t vec);
    mfuse_EXPORTS void VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc);
    mfuse_EXPORTS void VectorInverse(vec3_t vec);
    mfuse_EXPORTS void VecNegate(const vec3_t vec, vec3_t out);
    mfuse_EXPORTS void VectorScale(const vec3_t in, vec_t scale, vec3_t out);
    mfuse_EXPORTS void VectorClear(vec3_t vec);
    mfuse_EXPORTS void CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross);

    mfuse_EXPORTS unsigned int DirToByte(const vec3_t dir);
    mfuse_EXPORTS void ByteToDir(unsigned int b, vec3_t dir);
    mfuse_EXPORTS Vector GetMovedir(float angle);
    mfuse_EXPORTS float Random(float value);
    mfuse_EXPORTS uint8_t AngleToByte(float v);
    mfuse_EXPORTS float ByteToAngle(uint8_t v);
    mfuse_EXPORTS uint16_t AngleToShort(float v);
    mfuse_EXPORTS float ShortToAngle(uint16_t v);
    mfuse_EXPORTS int BoundingBoxToInteger(vec3_t mins, vec3_t maxs);
    mfuse_EXPORTS void IntegerToBoundingBox(int num, vec3_t mins, vec3_t maxs);

    template <typename INT>
    constexpr INT rotl(INT val, intptr_t len)
    {
        constexpr unsigned int mask = CHAR_BIT * sizeof(val) - 1;
        static_assert(std::is_unsigned<INT>::value, "Rotate Left is only allowed for unsigned types");
        return (val << len) | ((unsigned)val >> (-len & mask));
    }

    template <typename INT>
    constexpr INT rotr(INT val, intptr_t len)
    {
        constexpr unsigned int mask = CHAR_BIT * sizeof(val) - 1;
        static_assert(std::is_unsigned<INT>::value, "Rotate Right is only allowed for unsigned types");
        return (val >> len) | ((unsigned)val << (-len & mask));
    }
}
