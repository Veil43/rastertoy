// vec3.hpp
#pragma once
#include <cmath>
#include <limits>
#include <iostream>
#include <iomanip>
#include <algorithm>

// TODO: use templates

const float pi = 3.14159f;
const float infinity = std::numeric_limits<float>::infinity();

/*
12 bytes
*/
struct vec3f
{
    float x, y, z;
} ;

const vec3f VECTOR_I3 = {1, 0, 0};
const vec3f VECTOR_J3 = {0, 1, 0};
const vec3f VECTOR_K3 = {0, 0, 1};

struct vec2f
{
    float x, y;
};

struct vec4f
{
    float x, y, z, w;
};

using point3f = vec3f;
using point3 = point3f;
using vec3 = vec3f;

inline constexpr bool operator==(const vec2f & v, const vec2f& u) noexcept { return v.x == u.x && v.y == u.y; }
inline constexpr bool operator==(const vec3f & v, const vec3f& u) noexcept { return v.x == u.x && v.y == u.y && v.z == u.z; }
inline constexpr bool operator==(const vec4f & v, const vec4f& u) noexcept { return v.x == u.x && v.y == u.y && v.z == u.z && v.w == u.w; }

inline constexpr vec2f operator-(vec2f v) noexcept { return { -v.x, -v.y }; }
inline constexpr vec3f operator-(vec3f v) noexcept { return { -v.x, -v.y, -v.z }; }
inline constexpr vec4f operator-(vec4f v) noexcept { return { -v.x, -v.y, -v.z, -v.w }; }

inline constexpr vec2f operator+(const vec2f& v, const vec2f& u) noexcept { return { v.x + u.x, v.y + u.y }; }
inline constexpr vec3f operator+(const vec3f& v, const vec3f& u) noexcept { return { v.x + u.x, v.y + u.y, v.z + u.z }; }
inline constexpr vec4f operator+(const vec4f& v, const vec4f& u) noexcept { return { v.x + u.x, v.y + u.y, v.z + u.z, v.w + u.w }; }

inline constexpr vec2f operator-(const vec2f& v, const vec2f& u) noexcept { return { v.x - u.x, v.y - u.y }; }
inline constexpr vec3f operator-(const vec3f& v, const vec3f& u) noexcept { return { v.x - u.x, v.y - u.y, v.z - u.z }; }
inline constexpr vec4f operator-(const vec4f& v, const vec4f& u) noexcept { return { v.x - u.x, v.y - u.y, v.z - u.z, v.w - u.w }; }

// Hadamard Product (when will you use it? probably never:D)
inline constexpr vec3f hadamardProductf(const vec3f& v, const vec3f& u) noexcept { return { v.x * u.x, v.y * u.y, v.z * u.z }; }

inline constexpr vec2f operator*(float lambda, const vec2f& v) noexcept { return { lambda * v.x, lambda * v.y }; }
inline constexpr vec3f operator*(float lambda, const vec3f& v) noexcept { return { lambda * v.x, lambda * v.y, lambda * v.z }; }
inline constexpr vec4f operator*(float lambda, const vec4f& v) noexcept { return { lambda * v.x, lambda * v.y, lambda * v.z, lambda * v.w }; }

inline constexpr vec2f operator*(const vec2f& v, float lambda) noexcept { return { lambda * v.x, lambda * v.y }; };
inline constexpr vec3f operator*(const vec3f& v, float lambda) noexcept { return { lambda * v.x, lambda * v.y, lambda * v.z }; };
inline constexpr vec4f operator*(const vec4f& v, float lambda) noexcept { return { lambda * v.x, lambda * v.y, lambda * v.z, lambda * v.w }; }

inline constexpr vec2f operator/(const vec2f& v, float lambda) noexcept { return { (1 / lambda) * v.x, (1 / lambda) * v.y }; }
inline constexpr vec3f operator/(const vec3f& v, float lambda) noexcept { return { (1 / lambda) * v.x, (1 / lambda) * v.y, (1 / lambda) * v.z }; }
inline constexpr vec4f operator/(const vec4f& v, float lambda) noexcept { return { (1 / lambda) * v.x, (1 / lambda) * v.y, (1 / lambda) * v.z, (1/lambda) * v.w }; }

inline std::ostream& 
operator<<(std::ostream& os, const vec3f& v) 
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

inline std::ostream& 
operator<<(std::ostream &os, const vec2f& v) 
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

inline std::ostream& 
operator<<(std::ostream &os, const vec4f& v) 
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return os;
}

inline constexpr vec2f &
operator+=(vec2f &v, const vec2f &u) noexcept
{
    v.x += u.x;
    v.y += u.y;
    return v;
}

inline constexpr vec3f &
operator+=(vec3f &v, const vec3f &u) noexcept
{
    v.x += u.x;
    v.y += u.y;
    v.z += u.z;
    return v;
}

inline constexpr vec4f &
operator+=(vec4f &v, const vec4f &u) noexcept
{
    v.x += u.x;
    v.y += u.y;
    v.z += u.z;
    v.w += u.w;
    return v;
}

inline constexpr vec2f &
operator-=(vec2f &v, const vec2f &u) noexcept
{
    v.x -= u.x;
    v.y -= u.y;
    return v;
}

inline constexpr vec3f &
operator-=(vec3f &v, const vec3f &u) noexcept
{
    v.x -= u.x;
    v.y -= u.y;
    v.z -= u.z;
    return v;
}

inline constexpr vec4f &
operator-=(vec4f &v, const vec4f &u) noexcept
{
    v.x -= u.x;
    v.y -= u.y;
    v.z -= u.z;
    v.w -= u.w;
    return v;
}

inline constexpr vec2f& 
operator*=(vec2f& v, float lambda) noexcept
{
    v.x *= lambda;
    v.y *= lambda;
    return v;
}

inline constexpr vec3f& 
operator*=(vec3f& v, float lambda) noexcept
{
    v.x *= lambda;
    v.y *= lambda;
    v.z *= lambda;
    return v;
}

inline constexpr vec4f& 
operator*=(vec4f& v, float lambda) noexcept
{
    v.x *= lambda;
    v.y *= lambda;
    v.z *= lambda;
    v.w *= lambda;
    return v;
}

inline constexpr vec2f& 
operator/=(vec2f& v, float lambda) noexcept
{
    v.x /= lambda;
    v.y /= lambda;
    return v;
}

inline constexpr vec3f& 
operator/=(vec3f& v, float lambda) noexcept
{
    v.x /= lambda;
    v.y /= lambda;
    v.z /= lambda;
    return v;
}

inline constexpr vec4f &
operator/=(vec4f &v, float lambda) noexcept
{
    v.x /= lambda;
    v.y /= lambda;
    v.z /= lambda;
    v.w /= lambda;
    return v;
}

float constexpr length_squared(const vec2f& v) noexcept { return v.x * v.x + v.y * v.y; }
float constexpr length_squared(const vec3f& v) noexcept { return v.x * v.x + v.y * v.y + v.z * v.z; }
float constexpr length_squared(const vec4f& v) noexcept { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }

float length(const vec2f& v) noexcept { return std::sqrt(length_squared(v)); }
float length(const vec3f& v) noexcept { return std::sqrt(length_squared(v)); }
float length(const vec4f& v) noexcept { return std::sqrt(length_squared(v)); }

inline constexpr float dot(const vec2f& v, const vec2f& u) noexcept { return v.x * u.x + v.y * u.y; }
inline constexpr float dot(const vec3f& v, const vec3f& u) noexcept { return v.x * u.x + v.y * u.y + v.z * u.z; }
inline constexpr float dot(const vec4f& v, const vec4f& u) noexcept { return v.x * u.x + v.y * u.y + v.z * u.z + v.w * u.w; }

inline constexpr vec3f 
cross(const vec3f &v, const vec3f &u) noexcept
{
    return 
    {
        v.y * u.z - v.z * u.y,
        v.z * u.x - v.x * u.z,
        v.x * u.y - v.y * u.x
    };
}

inline vec2f &
normalize(vec2f &v) noexcept
{
    v /= length(v);
    return v;
}

inline vec3f &
normalize(vec3f &v) noexcept
{
    v /= length(v);
    return v;
}

inline vec4f &
normalize(vec4f &v) noexcept
{
    v /= length(v);
    return v;
}

inline constexpr vec3f &
clamp01(vec3f &p) noexcept
{
    p.x = p.x > 1 ? 1 :
          p.x < 0 ? 0 :
          p.x;
    
    p.y = p.y > 1 ? 1 :
          p.y < 0 ? 0 :
          p.y;

    p.z = p.z > 1 ? 1 :
          p.z < 0 ? 0 :
          p.z;
    
    return p;
}

inline constexpr vec3f &
clamp(vec3f &p, float min, float max) noexcept
{
    p.x = p.x > max ? max :
          p.x < min ? min :
          p.x;
    
    p.y = p.y > max ? max :
          p.y < min ? min :
          p.y;

    p.z = p.z > max ? max :
          p.z < min ? min :
          p.z;
    
    return p;
}

inline constexpr vec2f 
clamp(vec2f &p, double min, double max) noexcept
{
    p.x = p.x > max ? max :
          p.x < min ? min :
          p.x;
    
    p.y = p.y > max ? max :
          p.y < min ? min :
          p.y;
    
    return p;
}

inline constexpr double 
clamp(double p, double min, double max) noexcept
{
    p = p > max ? max :
        p < min ? min :
        p;
    
    return p;
}

inline constexpr vec3f
average(const vec3f& v0, const vec3f& v1, const vec3f& v2)
{
    return
    {
        (v0.x + v1.x + v2.x) / 3,
        (v0.y + v1.y + v2.y) / 3,
        (v0.z + v1.z + v2.z) / 3
    };
}

// RANDOM NUMBERS ----------------------------------------------------------
/*
Returns a radom double in range [0, 1)
*/
inline double random_double()
{
	// Returns a random real in [0, 1).
	return std::rand() / (RAND_MAX + 1.0);
}

inline float random_float()
{
   	return static_cast<float>(std::rand()) / (RAND_MAX + 1.0f);
}

inline double random_double(double_t min, double_t max)
{
	return min + (max - min) * random_double();
}

inline float random_float(float min, float max)
{
	return min + (max - min) * random_float();
}

inline int32_t random_int(int32_t min, int32_t max)
{
	return int(random_double(min, max));
}

inline vec3f random_vec3f() { return {random_float(), random_float(), random_float()}; }

inline vec3f random_vec3f(float min, float max) { return { random_float(min, max), random_float(min, max), random_float(min, max) }; }

// NOTE: This swaps x and y values of the vector
inline vec2f transpose(vec2f& v)
{
    std::swap(v.x, v.y);
    return v;
}
/*
Finds the value of the dependent variable d when the independent variable i = i
With linear samples (i0, d0) and (i1, d1)
*/
inline constexpr float linear_interpolate(float i, float i0, float d0, float i1, float d1)
{
    //                  gradient dd/di   
    return d0 + ((d1 - d0) / (i1 - i0)) *(i - i0); 
}

// LINES ---------------------------------------------------------------------------

struct line3d
{
    point3 A, B;
};

// SPHERES -------------------------------------------------------------------------
/*
16 bytes
*/
struct sphere
{
    vec3f center;
    float radius;
};

// PLANES -------------------------------------------------------------------------
/*
16 bytes
*/
struct plane
{
    vec3f normal;
    float distance; // To origin (ie what k scalar you need for kN to be on the plane)
};

// TODO: Take notes on this plane representation and its intersection tests
// returns 0 for on the plane -a for behind and +a for in front
inline constexpr float
plane_point_intersect(const plane& Plane, const point3f& p)
{
    return dot(Plane.normal, p) + Plane.distance;
}

inline constexpr float
plane_sphere_intersection_check(const plane& p, const sphere& s)
{
    return plane_point_intersect(p, s.center);
}

inline constexpr point3f
plane_line_intersect(const plane& P, const line3d& L)
{
    float t = (-P.distance - dot(P.normal, L.A)) / dot(P.normal, (L.B - L.A));
    return L.A + t * (L.B - L.A);
}

// MATRICES -----------------------------------------------------------------------
// Is a row matrix
union mat3x3
{   
    struct
    {
        vec3f r0, r1, r2;
    };
    float m[3][3];
    vec3f i() const { return r0; }
    vec3f j() const { return r1; }
    vec3f k() const { return r2; }
};

// Is a row matrix
/*
64 bytes
*/
union mat4x4
{
    struct
    {
        vec4f r0, r1, r2, r3;
    };
    float m[4][4];
};

const mat3x3 I_MATRIX_3X3{ 1, 0, 0, 0, 1, 0, 0, 0, 1 };
const mat4x4 I_MATRIX_4X4 { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

inline std::ostream&
operator<<(std::ostream& os, const mat4x4& m)
{   
    os << std::fixed << std::setprecision(7);
    for (int row = 0; row < 4; ++row)
    {
        os << "[";
        for (int col = 0; col < 4; ++col)
        {
            os << m.m[row][col];  // Set column width
            if (col != 3) os << "\t";
        }
        os  << "]\n";
    }
    return os;
}

// We are using row  matrices for  this project
inline constexpr vec4f
operator*(const vec4f &v, const mat4x4 &T)
{
    return
    {
        //  x * i       +   y * j         +   z * k         +   w * l
        v.x * T.m[0][0] + v.y * T.m[1][0] + v.z * T.m[2][0] + v.w * T.m[3][0],
        v.x * T.m[0][1] + v.y * T.m[1][1] + v.z * T.m[2][1] + v.w * T.m[3][1],
        v.x * T.m[0][2] + v.y * T.m[1][2] + v.z * T.m[2][2] + v.w * T.m[3][2],
        v.x * T.m[0][3] + v.y * T.m[1][3] + v.z * T.m[2][3] + v.w * T.m[3][3] 
    };
}

inline constexpr vec3f
operator*(const vec3f &v, const mat3x3 &T)
{
    return
    {
        //  x * i       +   y * j         +   z * k        
        v.x * T.m[0][0] + v.y * T.m[1][0] + v.z * T.m[2][0],
        v.x * T.m[0][1] + v.y * T.m[1][1] + v.z * T.m[2][1],
        v.x * T.m[0][2] + v.y * T.m[1][2] + v.z * T.m[2][2]
    };
}

inline constexpr vec3f
operator*(const vec3f &v, const mat4x4 &T)
{
    vec4f result = vec4f{v.x, v.y, v.z, 1} * T;
    return {result.x, result.y, result.z};
}

inline constexpr vec3f&
operator*=(vec3f &v, const mat4x4 &T)
{
    v = v * T;
    return v;
}

inline constexpr mat4x4
operator*(const mat4x4 &m, const mat4x4 &T)
{
    return { m.r0 * T, m.r1 * T, m.r2 * T, m.r3 * T };
}

inline constexpr mat4x4 &
operator*(float n, mat4x4 &m)
{
    m.r0 *= n;
    m.r1 *= n;
    m.r2 *= n;
    m.r3 *= n;
    return m;
}

inline constexpr mat4x4 &
operator*(mat4x4 &m, float n)
{
    m.r0 *= n;
    m.r1 *= n;
    m.r2 *= n;
    m.r3 *= n;
    return m;
}

inline constexpr mat4x4
operator*=(mat4x4& m, const mat4x4& T)
{
    m = m * T;
    return m;
}

inline constexpr mat4x4 &
operator*=(mat4x4& m, float n)
{
    m = m * n;
    return m;
}

inline constexpr mat4x4
transpose(const mat4x4& m)
{
    mat4x4 results = m;
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            auto t = results.m[row][col];
            results.m[row][col] = results.m[col][row];
            results.m[col][row] = t;
        }
    }
    return results;
}

inline constexpr mat3x3
transpose(const mat3x3& m)
{
    return {};
}

mat4x4
get_x_rotation_mat(float deg)
{
    float rad = deg * pi / 180.0f;
    return 
    {
         1,               0,             0, 0,
         0,   std::cos(rad), std::sin(rad), 0,
         0,  -std::sin(rad), std::cos(rad), 0,
         0,               0,             0, 1
    };
}

mat4x4
get_y_rotation_mat(float deg)
{
    float rad = deg * pi / 180.0f;
    return 
    {
        std::cos(rad), 0, -std::sin(rad), 0,
                    0, 1,              0, 0,
        std::sin(rad), 0,  std::cos(rad), 0,
                    0, 0,              0, 1
    };
}

mat4x4
get_z_rotation_mat(float deg)
{
    float rad = deg * pi / 180.0f;
    return 
    {
        std::cos(rad), std::sin(rad), 0, 0,
       -std::sin(rad), std::cos(rad), 0, 0,
                    0,             0, 1, 0,
                    0,             0, 0, 1
    };
}

mat4x4
get_x_rotation_mat_inverse(float deg)
{
    return transpose(get_x_rotation_mat(deg));
}

mat4x4
get_y_rotation_mat_inverse(float deg)
{
    return transpose(get_y_rotation_mat(deg));
}

mat4x4
get_z_rotation_mat_inverse(float deg)
{
    return transpose(get_z_rotation_mat(deg));
}

mat4x4
gauss_inverse(const mat4x4& matrix)
{
    // NOTE: floating point errors accumulate in gaussina elimination
    /*
    NOTE: Iteration pattern
        +----------------------+
        |   iteration 0        |
        +  +-------------------+
        |  |   iteration 1     |
        |  |  +----------------+
        |  |  |   iteration 2  |
        |  |  |  +-------------+
        |  |  |  | iteration 3 |
        +--+--+--+-------------+

        With each iteration, the pivot must be in the same row and column
    */
    const int size = 4;
    mat4x4 aug = matrix;
    mat4x4 identity = I_MATRIX_4X4;

    // Row echelon form
    for (int curr_row = 0; curr_row < size; ++curr_row)
    {
        // The pivot shall be at [curr_row][curr_row]
        int pivot_row = curr_row;
        // Find the row with the largest pivot pivot
        for (int candidate_row = curr_row + 1; candidate_row < size; ++candidate_row)
        {
            if (std::fabs(aug.m[candidate_row][curr_row] > std::fabs(aug.m[pivot_row][curr_row])))
            {
                pivot_row = candidate_row;
            }
        }

        // Swap the current row with the row containing the largest pivot
        for (int col = 0; col < size; ++col)
        {
            std::swap(aug.m[curr_row][col], aug.m[pivot_row][col]);
            std::swap(identity.m[curr_row][col], identity.m[pivot_row][col]);
        }

        // Check if the matrix is singular (contains an empty column cannot be inverted)
        float pivot_value = aug.m[curr_row][curr_row];
        if (std::fabs(pivot_value) < 1.0e-6)
        {
            return {};
        }

        // Normalize the pivot row
        for (int col = 0; col < size; ++col)
        {
            aug.m[curr_row][col] /= pivot_value;
            identity.m[curr_row][col] /= pivot_value;
        }

        // Eliminate the current column in all other rows
        for (int target_row = curr_row + 1; target_row < size; ++target_row)
        {
            float factor = aug.m[target_row][curr_row];
            for (int col = 0; col < size; ++col)
            {
                aug.m[target_row][col] -= factor * aug.m[curr_row][col];
                identity.m[target_row][col] -= factor * identity.m[curr_row][col];
            }
        }
    }
    
    // Reduced row echelon form
    for (int curr_row = size - 1; curr_row >= 0; --curr_row)
    {
        for (int target_row = curr_row - 1; target_row >= 0; --target_row)
        {
            float factor = aug.m[target_row][curr_row];
            for (int col = 0; col < size; ++ col)
            {
                aug.m[target_row][col] -= factor * aug.m[curr_row][col];
                identity.m[target_row][col] -= factor * identity.m[curr_row][col];
            }
        }
    }

    return identity;
}
