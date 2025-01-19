#pragma once
#include "math.h"
#include "object3d.h"

/*
96 bytes
*/
union frustum
{
    struct
    {
        plane near;
        plane far;
        plane left;
        plane right;
        plane top;
        plane bottom;
    };
    plane p[6];
};

bool SphereInFrustum(const sphere& BoundingSphere,  const frustum& F);

/*
220 bytes
*/
class camera
{
private:
    frustum cullFrustum_;       // Defines the volume of visible world objects.
    mat4x4 viewMatrix_;         // Encodes the homogeneous transforms of the camera which when  applied to world object gives the illustion of motion.
    vec3f origin_;              // The camera's location in 3d space.
    vec3f foreward_;            // Vector pointing in the "forward" direction from the camera's perspective.
    vec3f up_;                  // Vector pointing in the "up" direction from the camera's perspective.
    vec3f right_;               // Vector pointing in the "right" direction from the camera's perspective.
    real32 focalLength_;        // Distance from the camera to the center of the viewport.
    real32 vFov_;               // Viewing angle from the top to the bottom of the viewport.
    real32 aspectRatio_;        // Ratio between the width and the height of the viewport.
    real32 viewportWidth_;      // Width of the viewport.
    real32 viewportHeight_;     // Height of the viewport.

public:
    camera(const vec3f& origin, float focalLength, float vFov, float aspectRatio, const mat3x3& basis) 
        : origin_{origin}, focalLength_{focalLength}, 
          vFov_{vFov}, aspectRatio_{aspectRatio},
          up_{basis.j()}, right_{basis.i()}, foreward_{basis.k()}
    {
        initialize();
    }

    camera() 
    {
        camera({0,0,0}, 2.0f, 20.0f, 1.0f, I_MATRIX_3X3);
    }

    const frustum& CameraFrustum() const { return cullFrustum_; }
    const mat4x4& CameraViewMatrix() const { return viewMatrix_; }
    const mat3x3 CameraRotation() const
    {
        return 
        {
            viewMatrix_.r0.x, viewMatrix_.r0.y, viewMatrix_.r0.z,
            viewMatrix_.r1.x, viewMatrix_.r1.y, viewMatrix_.r1.z,
            viewMatrix_.r2.x, viewMatrix_.r2.y, viewMatrix_.r2.z
        };
    }
    const vec3f& CameraOrigin() const { return origin_; }
    real32 CameraFocalLength() const { return focalLength_; }
    real32 CameraViewportWidth() const { return viewportWidth_; }
    real32 CameraViewportHeight() const { return viewportHeight_; }
    
    bool ObjectInFrustum(Object3D *O) const 
    {
        sphere bSphere = O->ObjectBoundingSphere();
        bSphere.center = bSphere.center *  viewMatrix_;
 
        // true if the plane is behind the listed plane
        bool near =  plane_sphere_intersection_check(cullFrustum_.near, bSphere) < -bSphere.radius;
        bool left =  plane_sphere_intersection_check(cullFrustum_.left, bSphere) < -bSphere.radius;
        bool right =  plane_sphere_intersection_check(cullFrustum_.right, bSphere) < -bSphere.radius;
        bool top =  plane_sphere_intersection_check(cullFrustum_.top, bSphere) < -bSphere.radius;
        bool bottom =  plane_sphere_intersection_check(cullFrustum_.bottom, bSphere) < -bSphere.radius;

        bool results[] = { near, left, right };
        for (auto result : results)
        {
            if (result)
            {
                return false;
            }

        }
        return true;
    }

    void MoveBy(const vec3f& position) 
    {
        mat4x4 move = I_MATRIX_4X4;
        move.r3 = {position.x, position.y, position.z, 1};
        viewMatrix_ *= move;
    }
    void RotateYBy(real32 deg)
    {
        viewMatrix_ *= get_y_rotation_mat_inverse(deg);
    }

    vec3f ViewDirection(const vec3f& v)
    {
        return origin_ - v;
    }
    
private:
    void initialize()
    {
        viewportHeight_ = 2 * focalLength_ * std::tan(vFov_ / 2);
        viewportWidth_ = viewportHeight_ * aspectRatio_;

        viewMatrix_ = 
        {
            right_.x,       right_.y,       right_.z,       0.0f,
            up_.x,          up_.y,          up_.z,          0.0f,
            foreward_.x,    foreward_.y,    foreward_.z,    0.0f,
            origin_.x,      origin_.y,      origin_.z,      1.0f
        };

        // We need the corners of the near plane
        vec3f v = viewportHeight_ * up_;                // Vector up along the viewport
        vec3f u = viewportWidth_ * right_;              // Vector right across the viewport
        vec3f d = focalLength_ * foreward_;             // Vector from camera center to viewport center
        vec3f topRightCorner = d + 0.5 * v + 0.5 * u;
        vec3f botRightCorner = d - 0.5 * v + 0.5 * u;
        vec3f topLeftCorner = d + 0.5 * v - 0.5 * u;
        vec3f botLeftCorner = d - 0.5 * v - 0.5 * u;

        cullFrustum_.near =    {{0, 0, 1}, -focalLength_};
        cullFrustum_.left =    {cross(topLeftCorner, botLeftCorner), 0};
        cullFrustum_.right =   {cross(botRightCorner, topRightCorner), 0};
        cullFrustum_.top =     {cross(topRightCorner, topLeftCorner), 0};
        cullFrustum_.bottom =  {cross(botLeftCorner, botRightCorner), 0};
    }
};

// CULLING OPERATIONS ----------------------------------------------------------------------------------------

struct FrustumPointResults
{
    real32 side;
    plane p;
};

FrustumPointResults
FrustumCullPoint(const point3f& p,  const frustum& F)
{
    FrustumPointResults Near =
    {
        plane_point_intersect(F.near, p),
        F.near
    };
    FrustumPointResults Left =
    {
        plane_point_intersect(F.left, p),
        F.left
    };
    FrustumPointResults Right =
    {
        plane_point_intersect(F.right, p),
        F.right
    };
    FrustumPointResults Top =
    {
        plane_point_intersect(F.top, p),
        F.top
    };
    FrustumPointResults Bottom  =
    {
        plane_point_intersect(F.bottom, p),
        F.bottom
    };

    FrustumPointResults Results[] = { Near, Left, Right };
    for (auto result : Results)
    {
        if (result.side < 0)
        {
            return {-1, result.p};
        }
    }

    FrustumPointResults out = {};
    out.side = 1;
    return out;
}

struct ClippedTriangle
{
    // Indices for these are {0 1 2} - {0 2 3} for double
    // and {0 1 2} for single
    vertex3 v0, v1, v2, v3;
    bool IsSplit;
    bool IsIn;
};

// Naive approach 1 plane intersection 
ClippedTriangle
ClipTriangle(const vertex3& v0, const vertex3& v1, const vertex3& v2, const frustum& F)
{
    ClippedTriangle results = {};
    results.IsIn = false;

    // How many vertices in front?
    FrustumPointResults d0 = FrustumCullPoint(v0.point, F);
    FrustumPointResults d1 = FrustumCullPoint(v1.point, F);
    FrustumPointResults d2 = FrustumCullPoint(v2.point, F);

    if (d0.side == 1 && d1.side == 1 && d2.side == 1)
    {
        results.IsIn = true;
        results.IsSplit = true;
        results.v0 = v0;
        results.v1 = v1;
        results.v2 = v2;
        return results;
    }
    
    if (d0.side == -1 && d1.side == -1 && d2.side == -1)
    {
        return results;
    }

    // Create new triangles from clipped triangle
    vertex3 A;
    vertex3 B;
    vertex3 C;
    plane P;
    // Case 1: 1 point above the plane
    if (d0.side * d1.side * d2.side == 1)
    {
        if (d0.side == 1) // if you are inside you have no planes to test intercept chose one of the other
        {
            A = v0;
            B = v1;
            C = v2;
            P = d1.p;
        }
        else if (d1.side == 1)
        {
            A = v1;
            B = v0;
            C = v2;
            P = d0.p;
        }
        else
        {
            A = v2;
            B = v0;
            C = v1;
            P = d1.p;
        }

        // Find intersection AB with a plane P (B') and AC with a plane P (C')
        // return AB'C'
        line3d AB = {A.point, B.point};
        line3d AC = {A.point, C.point};
        vec3f BPrime = plane_line_intersect(P, AB);
        vec3f CPrime = plane_line_intersect(P, AC);

        // NOTE: We could add linear interpolated color for B' and C' [but we won't ;)]
        results.IsIn = true;
        results.IsSplit = false;
        results.v0.point = BPrime;
        results.v0.normal = B.normal;
        results.v0.color = B.color;
        results.v1 = A;
        results.v2.point = CPrime;
        results.v2.normal = C.normal;
        results.v2.color = C.color;
    }
    // Case 2: 2 points above the plane
    else
    {
        if (d0.side == -1)
        {
            C = v0;
            A = v1;
            B = v2;
            P = d0.p;
        }
        else if (d1.side == -1)
        {
            C = v1;
            A = v0;
            B = v2;
            P = d1.p;
        }
        else
        {
            C = v2;
            A = v0;
            B = v1;
            P = d2.p;
        }

        line3d CA = {C.point, A.point};
        line3d CB = {C.point, B.point};
        vec3f APrime = plane_line_intersect(P, CA);
        vec3f BPrime = plane_line_intersect(P, CB);

        results.IsIn = true;
        results.IsSplit = true;
        results.v0.point = APrime;
        results.v0.normal = A.normal;
        results.v0.color = A.color;
        results.v1 = A;
        results.v2 = B;
        results.v3.point = BPrime;
        results.v3.normal = B.normal;
        results.v3.color = B.color;
    }
    return results;
}