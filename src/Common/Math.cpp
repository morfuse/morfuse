#include <morfuse/Common/Math.h>
#include <morfuse/Common/Vector.h>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cfloat>
#include <cstdint>
#include <cstring>
#include <cstdlib>

static constexpr unsigned int X = 0;
static constexpr unsigned int Y = 1;
static constexpr unsigned int Z = 2;
static constexpr unsigned int W = 3;

static constexpr unsigned int PITCH = 0;
static constexpr unsigned int YAW = 1;
static constexpr unsigned int ROLL = 2;

static constexpr unsigned int BBOX_XBITS = 9;
static constexpr unsigned int BBOX_YBITS = 8;
static constexpr unsigned int BBOX_ZBOTTOMBITS = 5;
static constexpr unsigned int BBOX_ZTOPBITS = 9;

static constexpr int BBOX_MAX_X = (1 << BBOX_XBITS);
static constexpr int BBOX_MAX_Y = (1 << BBOX_YBITS);
static constexpr int BBOX_MAX_BOTTOM_Z = 1 << (BBOX_ZBOTTOMBITS - 1);
static constexpr int BBOX_REALMAX_BOTTOM_Z = (1 << BBOX_ZBOTTOMBITS);
static constexpr int BBOX_MAX_TOP_Z = (1 << BBOX_ZTOPBITS);

namespace mfuse
{
	vec3_t vec3_origin = { 0,0,0 };
	vec3_t bytedirs[NUMVERTEXNORMALS] =
	{
		{ -0.525731f, 0.000000f, 0.850651f },{ -0.442863f, 0.238856f, 0.864188f },
		{ -0.295242f, 0.000000f, 0.955423f },{ -0.309017f, 0.500000f, 0.809017f },
		{ -0.162460f, 0.262866f, 0.951056f },{ 0.000000f, 0.000000f, 1.000000f },
		{ 0.000000f, 0.850651f, 0.525731f },{ -0.147621f, 0.716567f, 0.681718f },
		{ 0.147621f, 0.716567f, 0.681718f },{ 0.000000f, 0.525731f, 0.850651f },
		{ 0.309017f, 0.500000f, 0.809017f },{ 0.525731f, 0.000000f, 0.850651f },
		{ 0.295242f, 0.000000f, 0.955423f },{ 0.442863f, 0.238856f, 0.864188f },
		{ 0.162460f, 0.262866f, 0.951056f },{ -0.681718f, 0.147621f, 0.716567f },
		{ -0.809017f, 0.309017f, 0.500000f },{ -0.587785f, 0.425325f, 0.688191f },
		{ -0.850651f, 0.525731f, 0.000000f },{ -0.864188f, 0.442863f, 0.238856f },
		{ -0.716567f, 0.681718f, 0.147621f },{ -0.688191f, 0.587785f, 0.425325f },
		{ -0.500000f, 0.809017f, 0.309017f },{ -0.238856f, 0.864188f, 0.442863f },
		{ -0.425325f, 0.688191f, 0.587785f },{ -0.716567f, 0.681718f, -0.147621f },
		{ -0.500000f, 0.809017f, -0.309017f },{ -0.525731f, 0.850651f, 0.000000f },
		{ 0.000000f, 0.850651f, -0.525731f },{ -0.238856f, 0.864188f, -0.442863f },
		{ 0.000000f, 0.955423f, -0.295242f },{ -0.262866f, 0.951056f, -0.162460f },
		{ 0.000000f, 1.000000f, 0.000000f },{ 0.000000f, 0.955423f, 0.295242f },
		{ -0.262866f, 0.951056f, 0.162460f },{ 0.238856f, 0.864188f, 0.442863f },
		{ 0.262866f, 0.951056f, 0.162460f },{ 0.500000f, 0.809017f, 0.309017f },
		{ 0.238856f, 0.864188f, -0.442863f },{ 0.262866f, 0.951056f, -0.162460f },
		{ 0.500000f, 0.809017f, -0.309017f },{ 0.850651f, 0.525731f, 0.000000f },
		{ 0.716567f, 0.681718f, 0.147621f },{ 0.716567f, 0.681718f, -0.147621f },
		{ 0.525731f, 0.850651f, 0.000000f },{ 0.425325f, 0.688191f, 0.587785f },
		{ 0.864188f, 0.442863f, 0.238856f },{ 0.688191f, 0.587785f, 0.425325f },
		{ 0.809017f, 0.309017f, 0.500000f },{ 0.681718f, 0.147621f, 0.716567f },
		{ 0.587785f, 0.425325f, 0.688191f },{ 0.955423f, 0.295242f, 0.000000f },
		{ 1.000000f, 0.000000f, 0.000000f },{ 0.951056f, 0.162460f, 0.262866f },
		{ 0.850651f, -0.525731f, 0.000000f },{ 0.955423f, -0.295242f, 0.000000f },
		{ 0.864188f, -0.442863f, 0.238856f },{ 0.951056f, -0.162460f, 0.262866f },
		{ 0.809017f, -0.309017f, 0.500000f },{ 0.681718f, -0.147621f, 0.716567f },
		{ 0.850651f, 0.000000f, 0.525731f },{ 0.864188f, 0.442863f, -0.238856f },
		{ 0.809017f, 0.309017f, -0.500000f },{ 0.951056f, 0.162460f, -0.262866f },
		{ 0.525731f, 0.000000f, -0.850651f },{ 0.681718f, 0.147621f, -0.716567f },
		{ 0.681718f, -0.147621f, -0.716567f },{ 0.850651f, 0.000000f, -0.525731f },
		{ 0.809017f, -0.309017f, -0.500000f },{ 0.864188f, -0.442863f, -0.238856f },
		{ 0.951056f, -0.162460f, -0.262866f },{ 0.147621f, 0.716567f, -0.681718f },
		{ 0.309017f, 0.500000f, -0.809017f },{ 0.425325f, 0.688191f, -0.587785f },
		{ 0.442863f, 0.238856f, -0.864188f },{ 0.587785f, 0.425325f, -0.688191f },
		{ 0.688191f, 0.587785f, -0.425325f },{ -0.147621f, 0.716567f, -0.681718f },
		{ -0.309017f, 0.500000f, -0.809017f },{ 0.000000f, 0.525731f, -0.850651f },
		{ -0.525731f, 0.000000f, -0.850651f },{ -0.442863f, 0.238856f, -0.864188f },
		{ -0.295242f, 0.000000f, -0.955423f },{ -0.162460f, 0.262866f, -0.951056f },
		{ 0.000000f, 0.000000f, -1.000000f },{ 0.295242f, 0.000000f, -0.955423f },
		{ 0.162460f, 0.262866f, -0.951056f },{ -0.442863f, -0.238856f, -0.864188f },
		{ -0.309017f, -0.500000f, -0.809017f },{ -0.162460f, -0.262866f, -0.951056f },
		{ 0.000000f, -0.850651f, -0.525731f },{ -0.147621f, -0.716567f, -0.681718f },
		{ 0.147621f, -0.716567f, -0.681718f },{ 0.000000f, -0.525731f, -0.850651f },
		{ 0.309017f, -0.500000f, -0.809017f },{ 0.442863f, -0.238856f, -0.864188f },
		{ 0.162460f, -0.262866f, -0.951056f },{ 0.238856f, -0.864188f, -0.442863f },
		{ 0.500000f, -0.809017f, -0.309017f },{ 0.425325f, -0.688191f, -0.587785f },
		{ 0.716567f, -0.681718f, -0.147621f },{ 0.688191f, -0.587785f, -0.425325f },
		{ 0.587785f, -0.425325f, -0.688191f },{ 0.000000f, -0.955423f, -0.295242f },
		{ 0.000000f, -1.000000f, 0.000000f },{ 0.262866f, -0.951056f, -0.162460f },
		{ 0.000000f, -0.850651f, 0.525731f },{ 0.000000f, -0.955423f, 0.295242f },
		{ 0.238856f, -0.864188f, 0.442863f },{ 0.262866f, -0.951056f, 0.162460f },
		{ 0.500000f, -0.809017f, 0.309017f },{ 0.716567f, -0.681718f, 0.147621f },
		{ 0.525731f, -0.850651f, 0.000000f },{ -0.238856f, -0.864188f, -0.442863f },
		{ -0.500000f, -0.809017f, -0.309017f },{ -0.262866f, -0.951056f, -0.162460f },
		{ -0.850651f, -0.525731f, 0.000000f },{ -0.716567f, -0.681718f, -0.147621f },
		{ -0.716567f, -0.681718f, 0.147621f },{ -0.525731f, -0.850651f, 0.000000f },
		{ -0.500000f, -0.809017f, 0.309017f },{ -0.238856f, -0.864188f, 0.442863f },
		{ -0.262866f, -0.951056f, 0.162460f },{ -0.864188f, -0.442863f, 0.238856f },
		{ -0.809017f, -0.309017f, 0.500000f },{ -0.688191f, -0.587785f, 0.425325f },
		{ -0.681718f, -0.147621f, 0.716567f },{ -0.442863f, -0.238856f, 0.864188f },
		{ -0.587785f, -0.425325f, 0.688191f },{ -0.309017f, -0.500000f, 0.809017f },
		{ -0.147621f, -0.716567f, 0.681718f },{ -0.425325f, -0.688191f, 0.587785f },
		{ -0.162460f, -0.262866f, 0.951056f },{ 0.442863f, -0.238856f, 0.864188f },
		{ 0.162460f, -0.262866f, 0.951056f },{ 0.309017f, -0.500000f, 0.809017f },
		{ 0.147621f, -0.716567f, 0.681718f },{ 0.000000f, -0.525731f, 0.850651f },
		{ 0.425325f, -0.688191f, 0.587785f },{ 0.587785f, -0.425325f, 0.688191f },
		{ 0.688191f, -0.587785f, 0.425325f },{ -0.955423f, 0.295242f, 0.000000f },
		{ -0.951056f, 0.162460f, 0.262866f },{ -1.000000f, 0.000000f, 0.000000f },
		{ -0.850651f, 0.000000f, 0.525731f },{ -0.955423f, -0.295242f, 0.000000f },
		{ -0.951056f, -0.162460f, 0.262866f },{ -0.864188f, 0.442863f, -0.238856f },
		{ -0.951056f, 0.162460f, -0.262866f },{ -0.809017f, 0.309017f, -0.500000f },
		{ -0.864188f, -0.442863f, -0.238856f },{ -0.951056f, -0.162460f, -0.262866f },
		{ -0.809017f, -0.309017f, -0.500000f },{ -0.681718f, 0.147621f, -0.716567f },
		{ -0.681718f, -0.147621f, -0.716567f },{ -0.850651f, 0.000000f, -0.525731f },
		{ -0.688191f, 0.587785f, -0.425325f },{ -0.587785f, 0.425325f, -0.688191f },
		{ -0.425325f, 0.688191f, -0.587785f },{ -0.425325f, -0.688191f, -0.587785f },
		{ -0.587785f, -0.425325f, -0.688191f },{ -0.688191f, -0.587785f, -0.425325f }
	};
};

using namespace mfuse;

float mfuse::DegreesToRadians(float angle)
{
	return (angle * M_PI_FLOAT) / 180.0f;
}

float mfuse::RadiansToDegrees(float rad)
{
	return (rad * 180.0f) / M_PI_FLOAT;
}

double mfuse::vrsqrt(double number)
{
	union {
		double d;
		int64_t i;
	} t;
	double x2, y;
	const float threehalfs = 1.5;
	const int64_t magic = 0x5fe6eb50c7b537a9;

	x2 = number * 0.5;
	t.d = number;
	t.i = magic - (t.i >> 1); // what the fuck?
	y = t.d;
	y = y * (threehalfs - (x2 * y * y)); // 1st iteration
	return y;
}

float mfuse::vrsqrtf(float number)
{
	union {
		float f;
		int32_t i;
	} t;
	float x2, y;
	const float threehalfs = 1.5f;
	const int32_t magic = 0x5f3759df;

	x2 = number * 0.5f;
	t.f = number;
	t.i = magic - (t.i >> 1); // what the fuck?
	y = t.f;
	y = y * (threehalfs - (x2 * y * y)); // 1st iteration
	return y;
}

vec_t mfuse::DotProduct(const vec3_t vec1, const vec3_t vec2)
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2];
}

vec_t mfuse::DotProduct2D(const vec2_t vec1, const vec2_t vec2)
{
	return ((vec1)[0] * (vec2)[0] + (vec1)[1] * (vec2)[1]);
}

vec_t mfuse::DotProduct4(const vec4_t vec1, const vec4_t vec2)
{
	return vec1[0] * vec2[0] + vec1[1] * vec2[1] + vec1[2] * vec2[2] + vec1[3] * vec2[3];
}

float mfuse::AngleNormalize360(float angle)
{
	return (360.0f / 65536) * ((int)(angle * (65536 / 360.0f)) & 65535);
}

float mfuse::AngleNormalize180(float angle)
{
	angle = AngleNormalize360(angle);
	if (angle > 180.0) {
		angle -= 360.0;
	}
	return angle;
}

float mfuse::AngleMod(float a)
{
	a = (360.0f / 65536) * ((int)(a*(65536 / 360.0f)) & 65535);
	return a;
}

float mfuse::AngleSubtract(float a1, float a2) {
	float	a;

	a = a1 - a2;
	while (a > 180) {
		a -= 360;
	}
	while (a < -180) {
		a += 360;
	}
	return a;
}


void mfuse::AnglesSubtract(vec3_t v1, vec3_t v2, vec3_t v3) {
	v3[0] = AngleSubtract(v1[0], v2[0]);
	v3[1] = AngleSubtract(v1[1], v2[1]);
	v3[2] = AngleSubtract(v1[2], v2[2]);
}

float mfuse::LerpAngle(float from, float to, float frac) {
	float	a;

	if (to - from > 180) {
		to -= 360;
	}
	if (to - from < -180) {
		to += 360;
	}
	a = from + frac * (to - from);

	return a;
}

void mfuse::AngleVectors(const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float		angle;
	static float		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * float(M_PI_FLOAT * 2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * float(M_PI_FLOAT * 2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles[ROLL] * float(M_PI_FLOAT * 2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	if (forward)
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}
	if (right)
	{
		right[0] = (-1 * sr * sp * cy + -1 * cr * -sy);
		right[1] = (-1 * sr * sp * sy + -1 * cr * cy);
		right[2] = -1 * sr * cp;
	}
	if (up)
	{
		up[0] = (cr * sp * cy + -sr * -sy);
		up[1] = (cr * sp * sy + -sr * cy);
		up[2] = cr * cp;
	}
}

void mfuse::AngleVectorsLeft(const vec3_t angles, vec3_t forward, vec3_t left, vec3_t up)
{
	float		angle;
	static float		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[YAW] * float(M_PI_FLOAT * 2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * float(M_PI_FLOAT * 2 / 360);
	sp = sin(angle);
	cp = cos(angle);

	if (forward)
	{
		forward[0] = cp * cy;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if (left || up)
	{
		angle = angles[ROLL] * float(M_PI_FLOAT * 2 / 360);
		sr = sin(angle);
		cr = cos(angle);

		if (left)
		{
			left[0] = (sr * sp * cy + cr * -sy);
			left[1] = (sr * sp * sy + cr * cy);
			left[2] = sr * cp;
		}
		if (up)
		{
			up[0] = (cr * sp * cy + -sr * -sy);
			up[1] = (cr * sp * sy + -sr * cy);
			up[2] = cr * cp;
		}
	}
}

void mfuse::AnglesToAxis(float angles[3], float axis[3][3])
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;
	// to help MS compiler fp bugs

	angle = angles[1] * (M_PI_FLOAT * 2 / 360);
	sy = sinf(angle);
	cy = cosf(angle);
	angle = angles[0] * (M_PI_FLOAT * 2 / 360);
	sp = sinf(angle);
	cp = cosf(angle);
	angle = angles[2] * (M_PI_FLOAT * 2 / 360);
	sr = sinf(angle);
	cr = cosf(angle);

	axis[0][0] = cp*cy;
	axis[0][1] = cp*sy;
	axis[0][2] = -sp;

	axis[1][0] = (sr*sp*cy + cr*-sy);
	axis[1][1] = (sr*sp*sy + cr*cy);
	axis[1][2] = sr*cp;

	axis[2][0] = (cr*sp*cy + -sr*-sy);
	axis[2][1] = (cr*sp*sy + -sr*cy);
	axis[2][2] = cr*cp;
}

void mfuse::MatToQuat(float srcMatrix[3][3], quat_t destQuat)
{
	float trace, s;
	int i, j, k;
	int 	next[3] = { Y, Z, X };

	trace = srcMatrix[X][X] + srcMatrix[Y][Y] + srcMatrix[Z][Z];

	if (trace > 0.0)
	{
		s = sqrtf(trace + 1.0f);
		destQuat[W] = s * 0.5f;
		s = 0.5f / s;

		destQuat[X] = (srcMatrix[Z][Y] - srcMatrix[Y][Z]) * s;
		destQuat[Y] = (srcMatrix[X][Z] - srcMatrix[Z][X]) * s;
		destQuat[Z] = (srcMatrix[Y][X] - srcMatrix[X][Y]) * s;
	}
	else
	{
		i = X;
		if (srcMatrix[Y][Y] > srcMatrix[X][X])
			i = Y;
		if (srcMatrix[Z][Z] > srcMatrix[i][i])
			i = Z;
		j = next[i];
		k = next[j];

		s = sqrtf((srcMatrix[i][i] - (srcMatrix[j][j] + srcMatrix[k][k])) + 1.0f);
		destQuat[i] = s * 0.5f;

		s = 0.5f / s;

		destQuat[W] = (srcMatrix[k][j] - srcMatrix[j][k]) * s;
		destQuat[j] = (srcMatrix[j][i] + srcMatrix[i][j]) * s;
		destQuat[k] = (srcMatrix[k][i] + srcMatrix[i][k]) * s;
	}
}

void mfuse::EulerToQuat(float ang[3], float q[4])
{
	float mat[3][3];
	int *i;

	i = (int *)ang;
	if (!i[0] && !i[1] && !i[2])
	{
		q[0] = 0.f;
		q[1] = 0.f;
		q[2] = 0.f;
		q[3] = 1.0f;
	}
	else
	{
		AnglesToAxis(ang, mat);
		MatToQuat(mat, q);
	}
}

void mfuse::QuatToMat(const float q[4], float m[3][3])
{
	float wx, wy, wz;
	float xx, yy, yz;
	float xy, xz, zz;
	float x2, y2, z2;

	x2 = q[X] + q[X];
	y2 = q[Y] + q[Y];
	z2 = q[Z] + q[Z];

	xx = q[X] * x2;
	xy = q[X] * y2;
	xz = q[X] * z2;

	yy = q[Y] * y2;
	yz = q[Y] * z2;
	zz = q[Z] * z2;

	wx = q[W] * x2;
	wy = q[W] * y2;
	wz = q[W] * z2;

	m[0][0] = 1.0f - (yy + zz);
	m[0][1] = xy - wz;
	m[0][2] = xz + wy;

	m[1][0] = xy + wz;
	m[1][1] = 1.0f - (xx + zz);
	m[1][2] = yz - wx;

	m[2][0] = xz - wy;
	m[2][1] = yz + wx;
	m[2][2] = 1.0f - (xx + yy);
}

void mfuse::QuatSet(quat_t q, float x, float y, float z, float w)
{
	q[0] = x;
	q[1] = y;
	q[2] = z;
	q[3] = w;
}

void mfuse::QuatClear(quat_t q)
{
	q[0] = 0.f;
	q[1] = 0.f;
	q[2] = 0.f;
	q[3] = 1.f;
}

void mfuse::QuatInverse(quat_t q)
{
	q[0] = -q[0];
	q[1] = -q[1];
	q[2] = -q[2];
}

void mfuse::QuatToAngles(const quat_t q, vec3_t angles)
{
	quat_t q2;

	q2[0] = q[0] * q[0];
	q2[1] = q[1] * q[1];
	q2[2] = q[2] * q[2];
	q2[3] = q[3] * q[3];

	angles[0] = RadiansToDegrees(asinf(-2 * (q[2] * q[0] - q[3] * q[1])));
	angles[1] = RadiansToDegrees(atan2f(2 * (q[2] * q[3] + q[0] * q[1]), (q2[2] - q2[3] - q2[0] + q2[1])));
	angles[2] = RadiansToDegrees(atan2f(2 * (q[3] * q[0] + q[2] * q[1]), (-q2[2] - q2[3] + q2[0] + q2[1])));
}

void mfuse::QuatNormalize(quat_t quat)
{
	float length;
	float ilength;

	length = sqrtf(quat[0] * quat[0] + quat[1] * quat[1] + quat[2] * quat[2] + quat[3] * quat[3]);
	if (length)
	{
		ilength = 1.f / length;
		quat[0] *= ilength;
		quat[1] *= ilength;
		quat[2] *= ilength;
		quat[3] *= ilength;
	}
}

void mfuse::MatrixToEulerAngles(const float mat[3][3], vec3_t ang)
{
	float theta;
	float cp;
	float sp;

	sp = mat[0][2];

	// cap off our sin value so that we don't get any NANs
	if (sp > 1.0)
	{
		sp = 1.0;
	}
	if (sp < -1.0)
	{
		sp = -1.0;
	}

	theta = -asinf(sp);
	cp = cosf(theta);

	if (cp > 8192 * FLT_EPSILON)
	{
		ang[0] = theta * 180 / M_PI_FLOAT;
		ang[1] = atan2f(mat[0][1], mat[0][0]) * 180 / M_PI_FLOAT;
		ang[2] = atan2f(mat[1][2], mat[2][2]) * 180 / M_PI_FLOAT;
	}
	else
	{
		ang[0] = theta * 180 / M_PI_FLOAT;
		ang[1] = -atan2f(mat[1][0], mat[1][1]) * 180 / M_PI_FLOAT;
		ang[2] = 0;
	}
}

void mfuse::MatrixMultiply(const float in1[3][3], const float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
		in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
		in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
		in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
		in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
		in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
		in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
		in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
		in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
		in1[2][2] * in2[2][2];
}

void mfuse::MatrixTransformVector(const vec3_t in, const float mat[3][3], vec3_t out)
{
	out[0] = (in[0] * mat[0][0]) + (in[1] * mat[1][0]) + (in[2] * mat[2][0]);
	out[1] = (in[0] * mat[0][1]) + (in[1] * mat[1][1]) + (in[2] * mat[2][1]);
	out[2] = (in[0] * mat[0][2]) + (in[1] * mat[1][2]) + (in[2] * mat[2][2]);
}

void mfuse::TransposeMatrix(float in[3][3], float out[3][3])
{
	out[0][0] = in[0][0];
	out[0][1] = in[1][0];
	out[0][2] = in[2][0];
	out[1][0] = in[0][1];
	out[1][1] = in[1][1];
	out[1][2] = in[2][1];
	out[2][0] = in[0][2];
	out[2][1] = in[1][2];
	out[2][2] = in[2][2];
}

void mfuse::MatrixCopy(const matrix_t in, matrix_t out)
{
	out[0] = in[0];		out[4] = in[4];		out[8] = in[8];		out[12] = in[12];
	out[1] = in[1];		out[5] = in[5];		out[9] = in[9];		out[13] = in[13];
	out[2] = in[2];		out[6] = in[6];		out[10] = in[10];	out[14] = in[14];
	out[3] = in[3];		out[7] = in[7];		out[11] = in[11];	out[15] = in[15];
}

void mfuse::Matrix3Copy(float in[3][3], float out[3][3])
{
	out[0][0] = in[0][0]; out[1][0] = in[1][0]; out[2][0] = in[2][0];
	out[0][1] = in[0][1]; out[1][1] = in[1][1]; out[2][1] = in[2][1];
	out[0][2] = in[0][2]; out[1][2] = in[1][2]; out[2][2] = in[2][2];
}

void mfuse::Matrix4_3Copy(float in[4][3], float out[3][3])
{
	out[0][0] = in[0][0]; out[1][0] = in[1][0]; out[2][0] = in[2][0];
	out[0][1] = in[0][1]; out[1][1] = in[1][1]; out[2][1] = in[2][1];
	out[0][2] = in[0][2]; out[1][2] = in[1][2]; out[2][2] = in[2][2];
}

void mfuse::AddPointToBounds(const Vector& v, Vector& mins, Vector& maxs)
{
	for (int i = 0; i < 3; i++)
	{
		float val = (float)v[i];
		if (val < mins[i])
			mins[i] = val;
		if (val > maxs[i])
			maxs[i] = val;
	}
}

void mfuse::ClearBounds(Vector& mins, Vector& maxs)
{
	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;
}

void mfuse::AxisClear(vec3_t axis[3])
{
	axis[0][0] = 1;
	axis[0][1] = 0;
	axis[0][2] = 0;
	axis[1][0] = 0;
	axis[1][1] = 1;
	axis[1][2] = 0;
	axis[2][0] = 0;
	axis[2][1] = 0;
	axis[2][2] = 1;
}

void mfuse::AxisCopy(const vec3_t in[3], vec3_t out[3])
{
	VecCopy(in[0], out[0]);
	VecCopy(in[1], out[1]);
	VecCopy(in[2], out[2]);
}

static constexpr float NORMAL_EPSILON = 0.0001f;

void mfuse::SnapVector(vec3_t normal) {
	int		i;

	for (i = 0; i < 3; i++)
	{
		if (::fabs(normal[i] - 1) < NORMAL_EPSILON)
		{
			VectorClear(normal);
			normal[i] = 1;
			break;
		}
		if (::fabs(normal[i] - -1) < NORMAL_EPSILON)
		{
			VectorClear(normal);
			normal[i] = -1;
			break;
		}
	}
}

void mfuse::VecCopy(const vec3_t in, vec3_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

void mfuse::VecSet(vec3_t out, float x, float y, float z)
{
	out[0] = x;
	out[1] = y;
	out[2] = z;
}

void mfuse::Vec4Copy(const vec4_t in, vec4_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}

void mfuse::VecSubtract(const vec3_t veca, const vec3_t vecb, vec3_t out)
{
	out[0] = veca[0] - vecb[0];
	out[1] = veca[1] - vecb[1];
	out[2] = veca[2] - vecb[2];
}

void mfuse::VecAdd(const vec3_t veca, const vec3_t vecb, vec3_t out)
{
	out[0] = veca[0] + vecb[0];
	out[1] = veca[1] + vecb[1];
	out[2] = veca[2] + vecb[2];
}

bool mfuse::VecCompare(const vec3_t veca, const vec3_t vecb)
{
	return veca[0] == vecb[0] && veca[1] == vecb[1] && veca[2] == vecb[2];
}

bool mfuse::VecCompare(const vec4_t veca, const vec4_t vecb, float tolerance)
{

	return (veca[0] >= vecb[0] - tolerance && veca[0] <= vecb[0] + tolerance)
		&& (veca[1] >= vecb[1] - tolerance && veca[1] <= vecb[1] + tolerance)
		&& (veca[2] >= vecb[2] - tolerance && veca[2] <= vecb[2] + tolerance);
}

mfuse_EXPORTS bool mfuse::Vec4Compare(const vec4_t veca, const vec4_t vecb)
{
	return veca[0] == vecb[0] && veca[1] == vecb[1] && veca[2] == vecb[2] && veca[3] == vecb[3];
}

mfuse_EXPORTS bool mfuse::Vec4Compare(const vec4_t veca, const vec4_t vecb, float tolerance)
{

	return (veca[0] >= vecb[0] - tolerance && veca[0] <= vecb[0] + tolerance)
		&& (veca[1] >= vecb[1] - tolerance && veca[1] <= vecb[1] + tolerance)
		&& (veca[2] >= vecb[2] - tolerance && veca[2] <= vecb[2] + tolerance)
		&& (veca[3] >= vecb[3] - tolerance && veca[3] <= vecb[3] + tolerance);
}

vec_t mfuse::VectorLength(const vec3_t vec)
{
	return (vec_t)sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

vec_t mfuse::VectorLengthSquared(const vec3_t vec)
{
	return (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
}

vec_t mfuse::VectorNormalize(vec3_t vec)
{
	float length;
	float ilength;

	length = sqrtf(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	if (length)
	{
		ilength = 1.f / length;
		vec[0] *= ilength;
		vec[1] *= ilength;
		vec[2] *= ilength;
	}

	return length;
}

vec_t mfuse::VectorNormalize2(const vec3_t v, vec3_t out) {
	float	length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrtf(length);

	if (length)
	{
		ilength = 1 / length;
		out[0] = v[0] * ilength;
		out[1] = v[1] * ilength;
		out[2] = v[2] * ilength;
	}
	else {
		VectorClear(out);
	}

	return length;
}

void mfuse::VectorNormalizeFast(vec3_t vec)
{
	float ilength = vrsqrtf(DotProduct(vec, vec));

	vec[0] *= ilength;
	vec[1] *= ilength;
	vec[2] *= ilength;
}

void mfuse::VectorMA(const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale * vecb[0];
	vecc[1] = veca[1] + scale * vecb[1];
	vecc[2] = veca[2] + scale * vecb[2];
}

void mfuse::VectorInverse(vec3_t vec)
{
	vec[0] = -vec[0];
	vec[1] = -vec[1];
	vec[2] = -vec[2];
}

void mfuse::VecNegate(const vec3_t vec, vec3_t out)
{
	out[0] = -vec[0];
	out[1] = -vec[1];
	out[2] = -vec[2];
}

void mfuse::VectorScale(const vec3_t in, vec_t scale, vec3_t out)
{
	out[0] = in[0] * scale;
	out[1] = in[1] * scale;
	out[2] = in[2] * scale;
}

void mfuse::VectorClear(vec3_t vec)
{
	vec[0] = 0.f;
	vec[1] = 0.f;
	vec[2] = 0.f;
}

void mfuse::VecMatrixInverse(void* DstMatrix, const void* SrcMatrix)
{
	typedef float Float4x4[4][4];
	Float4x4 M;
	Float4x4 Result;
	float Det[4];
	Float4x4 Tmp;

	memcpy((void *)M, SrcMatrix, sizeof(Float4x4));

	Tmp[0][0] = M[2][2] * M[3][3] - M[2][3] * M[3][2];
	Tmp[0][1] = M[1][2] * M[3][3] - M[1][3] * M[3][2];
	Tmp[0][2] = M[1][2] * M[2][3] - M[1][3] * M[2][2];

	Tmp[1][0] = M[2][2] * M[3][3] - M[2][3] * M[3][2];
	Tmp[1][1] = M[0][2] * M[3][3] - M[0][3] * M[3][2];
	Tmp[1][2] = M[0][2] * M[2][3] - M[0][3] * M[2][2];

	Tmp[2][0] = M[1][2] * M[3][3] - M[1][3] * M[3][2];
	Tmp[2][1] = M[0][2] * M[3][3] - M[0][3] * M[3][2];
	Tmp[2][2] = M[0][2] * M[1][3] - M[0][3] * M[1][2];

	Tmp[3][0] = M[1][2] * M[2][3] - M[1][3] * M[2][2];
	Tmp[3][1] = M[0][2] * M[2][3] - M[0][3] * M[2][2];
	Tmp[3][2] = M[0][2] * M[1][3] - M[0][3] * M[1][2];

	Det[0] = M[1][1] * Tmp[0][0] - M[2][1] * Tmp[0][1] + M[3][1] * Tmp[0][2];
	Det[1] = M[0][1] * Tmp[1][0] - M[2][1] * Tmp[1][1] + M[3][1] * Tmp[1][2];
	Det[2] = M[0][1] * Tmp[2][0] - M[1][1] * Tmp[2][1] + M[3][1] * Tmp[2][2];
	Det[3] = M[0][1] * Tmp[3][0] - M[1][1] * Tmp[3][1] + M[2][1] * Tmp[3][2];

	float Determinant = M[0][0] * Det[0] - M[1][0] * Det[1] + M[2][0] * Det[2] - M[3][0] * Det[3];
	const float	RDet = 1.0f / Determinant;

	Result[0][0] = RDet * Det[0];
	Result[0][1] = -RDet * Det[1];
	Result[0][2] = RDet * Det[2];
	Result[0][3] = -RDet * Det[3];
	Result[1][0] = -RDet * (M[1][0] * Tmp[0][0] - M[2][0] * Tmp[0][1] + M[3][0] * Tmp[0][2]);
	Result[1][1] = RDet * (M[0][0] * Tmp[1][0] - M[2][0] * Tmp[1][1] + M[3][0] * Tmp[1][2]);
	Result[1][2] = -RDet * (M[0][0] * Tmp[2][0] - M[1][0] * Tmp[2][1] + M[3][0] * Tmp[2][2]);
	Result[1][3] = RDet * (M[0][0] * Tmp[3][0] - M[1][0] * Tmp[3][1] + M[2][0] * Tmp[3][2]);
	Result[2][0] = RDet * (
		M[1][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
		M[2][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) +
		M[3][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1])
		);
	Result[2][1] = -RDet * (
		M[0][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
		M[2][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
		M[3][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1])
		);
	Result[2][2] = RDet * (
		M[0][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) -
		M[1][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
		M[3][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
		);
	Result[2][3] = -RDet * (
		M[0][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1]) -
		M[1][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1]) +
		M[2][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
		);
	Result[3][0] = -RDet * (
		M[1][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
		M[2][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) +
		M[3][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
		);
	Result[3][1] = RDet * (
		M[0][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
		M[2][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
		M[3][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1])
		);
	Result[3][2] = -RDet * (
		M[0][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) -
		M[1][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
		M[3][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
		);
	Result[3][3] = RDet * (
		M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1]) -
		M[1][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1]) +
		M[2][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
		);

	memcpy(DstMatrix, &Result, 16 * sizeof(float));
}

void mfuse::CrossProduct(const vec3_t v1, const vec3_t v2, vec3_t cross)
{
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

unsigned int mfuse::DirToByte(const vec3_t dir)
{
	unsigned int best;
	float d, bestd;

	if (!dir) {
		return 0;
	}

	bestd = 0;
	best = 0;
	for (unsigned int i = 0; i < NUMVERTEXNORMALS; i++)
	{
		d = DotProduct(dir, bytedirs[i]);
		if (d > bestd)
		{
			bestd = d;
			best = i;
		}
	}

	return best;
}

void mfuse::ByteToDir(unsigned int b, vec3_t dir)
{
	if (b < 0 || b >= NUMVERTEXNORMALS)
	{
		VectorClear(dir);
		return;
	}
	VecCopy(bytedirs[b], dir);
}

mfuse::Vector mfuse::GetMovedir(float angle)
{
	if (angle == -1.0f)
	{
		return Vector(0.0f, 0.0f, 1.0f);
	}
	else if (angle == -2.0f)
	{
		return Vector(0.0f, 0.0f, -1.0f);
	}

	angle *= (M_PI_FLOAT * 2.0f / 360.0f);
	return Vector(cosf(angle), sinf(angle), 0.0f);
}

float mfuse::Random(float value)
{
	return fmodf((float)rand(), value);
}

uint8_t mfuse::AngleToByte(float v)
{
	return (uint8_t)(v * 256.f / 360.f) & 255;
}

float mfuse::ByteToAngle(uint8_t v)
{
	return (v) * (360.f / 255.f);
}

uint16_t mfuse::AngleToShort(float v)
{
	return (uint16_t)((v) * 65536.f / 360.f) & 65535;
}

float mfuse::ShortToAngle(uint16_t v)
{
	return (v) * (360.f / 65536.f);
}

int mfuse::BoundingBoxToInteger(vec3_t mins, vec3_t maxs)
{
	int x, y, zd, zu, result;

	x = int(maxs[0]);
	if (x < 0)
		x = 0;
	if (x >= BBOX_MAX_X)
		x = BBOX_MAX_X - 1;

	y = int(maxs[1]);
	if (y < 0)
		y = 0;
	if (y >= BBOX_MAX_Y)
		y = BBOX_MAX_Y - 1;

	zd = int(mins[2]) + BBOX_MAX_BOTTOM_Z;
	if (zd < 0)
		zd = 0;
	if (zd >= BBOX_REALMAX_BOTTOM_Z)
	{
		zd = BBOX_REALMAX_BOTTOM_Z - 1;
	}

	zu = int(maxs[2]);
	if (zu < 0)
		zu = 0;
	if (zu >= BBOX_MAX_TOP_Z)
		zu = BBOX_MAX_TOP_Z - 1;

	result = x |
		(y << BBOX_XBITS) |
		(zd << (BBOX_XBITS + BBOX_YBITS)) |
		(zu << (BBOX_XBITS + BBOX_YBITS + BBOX_ZBOTTOMBITS));

	return result;
}

void mfuse::IntegerToBoundingBox(int num, vec3_t mins, vec3_t maxs)
{
	int x, y, zd, zu;

	x = num & (BBOX_MAX_X - 1);
	y = (num >> (BBOX_XBITS)) & (BBOX_MAX_Y - 1);
	zd = (num >> (BBOX_XBITS + BBOX_YBITS)) & (BBOX_REALMAX_BOTTOM_Z - 1);
	zd -= BBOX_MAX_BOTTOM_Z;
	zu = (num >> (BBOX_XBITS + BBOX_YBITS + BBOX_ZBOTTOMBITS)) & (BBOX_MAX_TOP_Z - 1);

	mins[0] = float(-x);
	mins[1] = float(-y);
	mins[2] = float(zd);

	maxs[0] = float(x);
	maxs[1] = float(y);
	maxs[2] = float(zu);
}
