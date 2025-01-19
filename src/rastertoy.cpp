#include "math.h"
#include "platform.h"
#include "color.h"
#include "object3d.h"
#include "camera.h"
#include "lighting.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>

/*
MISC: Undertand c/c++ unions
// NOTE: Triangles gaps at "flat slopes" (caused by not transposing)
// NOTE: Triangle Filling methods
// NOTE: Transformations
// NOTE: Projection matrices
// NOTE: Camera Space and world transformations
// NOTE: Camera Rotation
// NOTE: Explore dropping the fourth column of a 4x4 matrix of affine tranformations
// NOTE: Starting with mat4x4 as a vec4f collection was a mistake and led to a rewrite
// NOTE: Inverse roattion matrix for viewmatrix rotation has the same effect as non inverse
//       rotation matrix.
*/
namespace rastertoy
{
enum RenderOption
{
    RENDER_WIREFRAME,
    RENDER_SOLID,
    RENDER_SOLID_WIREFRAME
};

enum ShadingOption
{
    SHADE_FLAT,
    SHADE_GOURAUD,
    SHADE_PHONG
};

// GLOBAL VARIABLES  --------------------------------------------------------------------
static PlatformScreenDevice globalScreenDevice;
static real32 *globalDepthBuffer;
static real32 globalDeltaTime;
static std::vector<Object3D *> worldObjects;
static camera globalCamera;
static RenderOption globalRenderMode;
static ShadingOption globalShadingMode;
static point_light globalOmniLight;
static ambient_light globalAmbientLight;
static uint8 globalObjectCursor = 0;
static bool globalRenderNormals = false;

// UTILITY SECTION STARTS HERE ----------------------------------------------------------------
// Only these function make direct access to the screen buffer
namespace screen_draw
{
static void
ScaleNDCToScreen(vertex2 &v)
{
    v.point = clamp(v.point, -1 , 1); // Enforce range - 1 to 1

    v.point.x = ((v.point.x + 1) / 2) * (globalScreenDevice.width - 1);
    v.point.y = ((1 - v.point.y) / 2) * (globalScreenDevice.height - 1);
}

static void
PutPixelNDC(vertex2 vertex, real32 z_value)
{
    if (vertex.point.y >  1 || vertex.point.x >  1 ||
        vertex.point.y < -1 || vertex.point.x < -1  )
    {
        return;
    }

    ScaleNDCToScreen(vertex);

    uint32 *tempBuffer = (uint32 *) globalScreenDevice.BufferMemory;
    uint32 index = globalScreenDevice.width * static_cast<int>(vertex.point.y) + static_cast<int>(vertex.point.x);

    if (z_value > globalDepthBuffer[index])
    {
        tempBuffer[index] = color_uint32(vertex.color);
        globalDepthBuffer[index] = z_value;
    }
}

static vertex2
ProjectVertexNDC(const vertex3& v)
{
    real32 d = globalCamera.CameraOrigin().z + globalCamera.CameraFocalLength();

    real32 py = (v.point.y * d) / v.point.z;;
    real32 px = (v.point.x * d) / v.point.z;;

    // Projects from viewport to NDC
    py = ((py * 2.0f) / globalCamera.CameraViewportHeight());
    px = ((px * 2.0f) / globalCamera.CameraViewportWidth());

    return {v.color, px, py};
}

static void
BlackoutScreenBuffer(color4 color)
{
    uint32 *tempBuffer = (uint32 *) globalScreenDevice.BufferMemory;
    std::fill(tempBuffer,
            tempBuffer + (globalScreenDevice.width * globalScreenDevice.height),
            color_uint32(color));

    std::fill(globalDepthBuffer,
            globalDepthBuffer + (globalScreenDevice.width * globalScreenDevice.height),
            0.0f);
            // used to be infinity until the 1/z shite
}
} // namspace screen_draw

// Polygon drawing ---------------------------------------------------------------------------
namespace polygon_draw
{

static void
DrawLine(vertex2 v0, vertex2 v1, color4 line_color = NO_COLOR, int z_priority = -100)
{
    color4 cmp = NO_COLOR;
    bool interpolate = line_color == cmp;

    // assumes the points are between -1 to 1
    real32 dy = 2.0f / globalScreenDevice.height;
    real32 dx = 2.0f / globalScreenDevice.width;

    bool steep = std::abs(v0.point.y - v1.point.y) > std::abs(v0.point.x - v1.point.x);
    if (steep) // Transpose
    {
        transpose(v0.point);
        transpose(v1.point);
    }
    if (v0.point.x > v1.point.x) std::swap(v0, v1);

    // FLAT LINE and x(y) for STEEP LINE
    // y (x) = mx + c
    // y (x+dx) = m(x + dx) + c => mx + c + mdx
    // y (x+1) = y(x) + mdx
    // y (x) = x0
    for (real32 x = v0.point.x; x <= v1.point.x; x+=dx)
    {
        real32 y = linear_interpolate(x, v0.point.x, v0.point.y, v1.point.x, v1.point.y);

        color4 color = interpolate ? linear_interpolate_color(x, v0.point.x, v0.color, v1.point.x, v1.color)
                                   : line_color;
        if (steep)
        {
            screen_draw::PutPixelNDC({color, y, x}, z_priority); // Undoes the transpose
        }
        else
        {
            screen_draw::PutPixelNDC({color, x, y}, z_priority);
        }
    }
}

static void
DrawNormal(const vertex3& v0)
{
    vec3f origin = v0.point;

    vertex2 start  = screen_draw::ProjectVertexNDC({origin, v0.normal, WHITE});
    vertex2 end = screen_draw::ProjectVertexNDC({origin + v0.normal, v0.normal, GREEN});

    DrawLine(start, end, NO_COLOR, 1000);
}

static void
DrawWireframeTriangle(vertex3 v0, vertex3 v1, vertex3 v2, color4 lineColor = NO_COLOR)
{
    vertex2 p0 = screen_draw::ProjectVertexNDC(v0);
    vertex2 p1 = screen_draw::ProjectVertexNDC(v1);
    vertex2 p2 = screen_draw::ProjectVertexNDC(v2);

    DrawLine({RED, p0.point}, {RED, p1.point}, lineColor, 1);
    DrawLine({RED, p1.point}, {RED, p2.point}, lineColor, 1);
    DrawLine({RED, p2.point}, {RED, p0.point}, lineColor, 1);
}

/*
BUG FIXED: Passing the vertices by reference caused an unintended consequence,
that is when said projected vertices are in an array Projected[], passing
ShadeTrianglePhong(Pojected[i]...) will result in altering the vertex order in the
array without changing the draw order. Thi obviously is BAD!

The hard gap problem was caused by a unifine scanline filling loop. Splitting it into
top and bottom seems to have resolved the problem.
*/
static void
ShadeTrianglePhong(const vertex3& v0, const vertex3& v1, const vertex3& v2, const point_light& light, real32 ambientIntensity)
{
     /*
     // TODO: Take notes on this algorithm
     // TODO: Understand the 1/z crap
     // This is an old school scan line filling method
                                        P2


                                            P1

                              P0
    // TECNIQUE: Go from P0 to P2 and to P1 starting at Height = P0.y  but only up to Height = P1.y
    // Then Go from P0 to P2 and from P1 to P2 starting at Height = P1.y ending at P2.y
    */
    const real32 dy = 2.0f / globalScreenDevice.height;
    const real32 dx = 2.0f / globalScreenDevice.width;
    const real32 d = globalCamera.CameraFocalLength();

    vertex2 p0 = screen_draw::ProjectVertexNDC(v0);
    vertex2 p1 = screen_draw::ProjectVertexNDC(v1);
    vertex2 p2 = screen_draw::ProjectVertexNDC(v2);

    real32 z0 = v0.point.z;
    real32 z1 = v1.point.z;
    real32 z2 = v2.point.z;

    vec3f n0 = v0.normal;
    vec3f n1 = v1.normal;
    vec3f n2 = v2.normal;

    // Sort the points in order y0 <= y1 <= y2
    if (p1.point.y < p0.point.y)
    {
        std::swap(p0, p1);
        std::swap(z0, z1);
        std::swap(n0, n1);
    }
    if (p2.point.y < p0.point.y)
    {
        std::swap(p0, p2);
        std::swap(z0, z2);
        std::swap(n0, n2);
    }
    if (p2.point.y < p1.point.y)
    {
        std::swap(p1, p2);
        std::swap(z1, z2);
        std::swap(n1, n2);
    }

    bool interpolate02 = ((p0.color != p2.color));
    bool interpolate01 = ((p0.color != p1.color));
    bool interpolate12 = ((p1.color != p2.color));

    for (real32 y = p0.point.y; y < p1.point.y; y+=dy)
    {
        real32 startX = linear_interpolate(y, p0.point.y, p0.point.x, p2.point.y, p2.point.x);
        real32 endX = linear_interpolate(y, p0.point.y, p0.point.x, p1.point.y, p1.point.x);

        real32 startZ = linear_interpolate(y, p0.point.y, 1/z0, p2.point.y, 1/z2);
        real32 endZ = linear_interpolate(y, p0.point.y, 1/z0, p1.point.y, 1/z1);

        // Normal Interpolation ------------------------------------------------------------------------
        real32 startNormalX = linear_interpolate(y, p0.point.y, n0.x, p2.point.y, n2.x);
        real32 endNormalX = linear_interpolate(y, p0.point.y, n0.x, p1.point.y, n1.x);
        real32 startNormalY = linear_interpolate(y, p0.point.y, n0.y, p2.point.y, n2.y);
        real32 endNormalY = linear_interpolate(y, p0.point.y, n0.y, p1.point.y, n1.y);
        real32 startNormalZ = linear_interpolate(y, p0.point.y, n0.z, p2.point.y, n2.z);
        real32 endNormalZ = linear_interpolate(y, p0.point.y, n0.z, p1.point.y, n1.z);

        // Color Interpolation -------------------------------------------------------------------------
        color4 startColor = interpolate02 ? linear_interpolate_color(y , p0.point.y, p0.color, p2.point.y, p2.color) : p0.color;
        color4 endColor = interpolate01 ? linear_interpolate_color(y, p0.point.y, p0.color, p1.point.y, p1.color) : p1.color;

        if (startX > endX)
        {
            std::swap(startX, endX);
            std::swap(startZ, endZ);
            std::swap(startNormalX, endNormalX);
            std::swap(startNormalY, endNormalY);
            std::swap(startNormalZ, endNormalZ);
            std::swap(startColor, endColor);
        }

        for (real32 x = startX; x <= endX; x+=dx)
        {
            color4 col = (interpolate02 || interpolate01) ? linear_interpolate_color(x, startX, startColor, endX, endColor) : startColor;
            const real32 z = linear_interpolate(x, startX, 1/startZ, endX, 1/endZ);

            const real32 normal_x = linear_interpolate(x, startX, startNormalX, endX, endNormalX);
            const real32 normal_y = linear_interpolate(x, startX, startNormalY, endX, endNormalY);
            const real32 normal_z = linear_interpolate(x, startX, startNormalZ, endX, endNormalZ);

            vertex3 p;
            p.point.x = x*d/z;
            p.point.y = y*d/z;
            p.point.z = z;
            p.normal = {normal_x, normal_y, normal_z};
            p.normal = normalize(p.normal);

            real32 intensity = (ambientIntensity +  light.GetIntensityPhong(p, globalCamera.CameraOrigin() - p.point));
            col *= intensity;

            screen_draw::PutPixelNDC({col, x, y}, 1/z);
        }
    }

    for (real32 y = p1.point.y; y <= p2.point.y; y+=dy)
    {
        real32 startX = linear_interpolate(y, p0.point.y, p0.point.x, p2.point.y, p2.point.x);
        real32 endX = linear_interpolate(y, p1.point.y, p1.point.x, p2.point.y, p2.point.x);

        real32 startZ = linear_interpolate(y, p0.point.y, 1/z0, p2.point.y, 1/z2);
        real32 endZ = linear_interpolate(y, p1.point.y, 1/z1, p2.point.y, 1/z2);

        // Normal Interpolation ------------------------------------------------------------------------
        real32 startNormalX = linear_interpolate(y, p0.point.y, n0.x, p2.point.y, n2.x);
        real32 endNormalX = linear_interpolate(y, p1.point.y, n1.x, p2.point.y, n2.x);
        real32 startNormalY = linear_interpolate(y, p0.point.y, n0.y, p2.point.y, n2.y);
        real32 endNormalY = linear_interpolate(y, p1.point.y, n1.y, p2.point.y, n2.y);
        real32 startNormalZ = linear_interpolate(y, p0.point.y, n0.z, p2.point.y, n2.z);
        real32 endNormalZ = linear_interpolate(y, p1.point.y, n1.z, p2.point.y, n2.z);

        // Color Interpolation -------------------------------------------------------------------------
        color4 startColor = interpolate02 ? linear_interpolate_color(y , p0.point.y, p0.color, p2.point.y, p2.color) : p0.color;
        color4 endColor = interpolate12 ? linear_interpolate_color(y, p1.point.y, p1.color, p2.point.y, p2.color) : p1.color;

        if (startX > endX)
        {
            std::swap(startX, endX);
            std::swap(startZ, endZ);
            std::swap(startNormalX, endNormalX);
            std::swap(startNormalY, endNormalY);
            std::swap(startNormalZ, endNormalZ);
            std::swap(startColor, endColor);
        }

        for (real32 x = startX; x <= endX; x+=dx)
        {
            color4 col = (interpolate02 || interpolate12) ? linear_interpolate_color(x, startX, startColor, endX, endColor) : startColor;
            const real32 z = linear_interpolate(x, startX, 1/startZ, endX, 1/endZ);

            const real32 normal_x = linear_interpolate(x, startX, startNormalX, endX, endNormalX);
            const real32 normal_y = linear_interpolate(x, startX, startNormalY, endX, endNormalY);
            const real32 normal_z = linear_interpolate(x, startX, startNormalZ, endX, endNormalZ);

            vertex3 p;
            p.point.x = x*d/z;
            p.point.y = y*d/z;
            p.point.z = z;
            p.normal = {normal_x, normal_y, normal_z};
            p.normal = normalize(p.normal);
            real32 intensity = (ambientIntensity +  light.GetIntensityPhong(p, globalCamera.CameraOrigin() - p.point));
            col *= intensity;

           screen_draw::PutPixelNDC({col, x, y}, 1/z);
        }
    }
}

static void
ShadeTriangleGouraudFlat(const vertex3& v0, const vertex3& v1, const vertex3& v2, const point_light& light, real32 ambientIntensity, ShadingOption mode = SHADE_FLAT)
{
    const real32 dy = 2.0f / globalScreenDevice.height;
    const real32 dx = 2.0f / globalScreenDevice.width;

    vertex2 p0 = screen_draw::ProjectVertexNDC(v0);
    vertex2 p1 = screen_draw::ProjectVertexNDC(v1);
    vertex2 p2 = screen_draw::ProjectVertexNDC(v2);

    real32 z0 = v0.point.z;
    real32 z1 = v1.point.z;
    real32 z2 = v2.point.z;

    if (mode == SHADE_FLAT)
    {
        real32 intensity = light.GetIntensityFlat(cross(v1.point - v0.point, v2.point - v0.point), average(v0.point, v1.point, v2.point));
        p0.color = p0.color * (ambientIntensity + intensity);
        p1.color = p1.color * (ambientIntensity + intensity);
        p2.color = p2.color * (ambientIntensity + intensity);
    }
    else
    {
        p0.color = p0.color * (ambientIntensity + light.GetIntensityGouraud(v0));
        p1.color = p1.color * (ambientIntensity + light.GetIntensityGouraud(v1));
        p2.color = p2.color * (ambientIntensity + light.GetIntensityGouraud(v2));
    }


    if (p1.point.y < p0.point.y)
    {
        std::swap(p0, p1);
        std::swap(z0, z1);
    }
    if (p2.point.y < p0.point.y)
    {
        std::swap(p0, p2);
        std::swap(z0, z2);
    }
    if (p2.point.y < p1.point.y)
    {
        std::swap(p1, p2);
        std::swap(z1, z2);
    }

    bool interpolate02 = ((p0.color != p2.color));
    bool interpolate01 = ((p0.color != p1.color));
    bool interpolate12 = ((p1.color != p2.color));

    for (real32 y = p0.point.y; y < p1.point.y; y+=dy)
    {
        real32 startX = linear_interpolate(y, p0.point.y, p0.point.x, p2.point.y, p2.point.x);
        real32 endX = linear_interpolate(y, p0.point.y, p0.point.x, p1.point.y, p1.point.x);

        real32 startZ = linear_interpolate(y, p0.point.y, 1/z0, p2.point.y, 1/z2);
        real32 endZ = linear_interpolate(y, p0.point.y, 1/z0, p1.point.y, 1/z1);

        color4 startColor = interpolate02 ? linear_interpolate_color(y , p0.point.y, p0.color, p2.point.y, p2.color) : p0.color;
        color4 endColor = interpolate01 ? linear_interpolate_color(y, p0.point.y, p0.color, p1.point.y, p1.color) : p1.color;

        if (startX > endX)
        {
            std::swap(startX, endX);
            std::swap(startZ, endZ);
            std::swap(startColor, endColor);
        }

        for (real32 x = startX; x <= endX; x+=dx)
        {
            color4 col = (interpolate02 || interpolate01) ? linear_interpolate_color(x, startX, startColor, endX, endColor) : startColor;
            const real32 z = linear_interpolate(x, startX, 1/startZ, endX, 1/endZ);
            screen_draw::PutPixelNDC({col, x, y}, 1/z);
        }
    }

    for (real32 y = p1.point.y; y <= p2.point.y; y+=dy)
    {
        real32 startX = linear_interpolate(y, p0.point.y, p0.point.x, p2.point.y, p2.point.x);
        real32 endX = linear_interpolate(y, p1.point.y, p1.point.x, p2.point.y, p2.point.x);

        real32 startZ = linear_interpolate(y, p0.point.y, 1/z0, p2.point.y, 1/z2);
        real32 endZ = linear_interpolate(y, p1.point.y, 1/z1, p2.point.y, 1/z2);

        color4 startColor = interpolate02 ? linear_interpolate_color(y , p0.point.y, p0.color, p2.point.y, p2.color) : p0.color;
        color4 endColor = interpolate12 ? linear_interpolate_color(y, p1.point.y, p1.color, p2.point.y, p2.color) : p1.color;

        if (startX > endX)
        {
            std::swap(startX, endX);
            std::swap(startZ, endZ);
            std::swap(startColor, endColor);
        }

        for (real32 x = startX; x <= endX; x+=dx)
        {
            color4 col = (interpolate02 || interpolate12) ? linear_interpolate_color(x, startX, startColor, endX, endColor) : startColor;
            const real32 z = linear_interpolate(x, startX, 1/startZ, endX, 1/endZ);
            screen_draw::PutPixelNDC({col, x, y}, 1/z);
        }
    }
}

static void
ShadeTriangleInMode(const vertex3& v0, const vertex3& v1, const vertex3& v2, const point_light& light, real32 ambientIntensity, ShadingOption option)
{
    switch (option)
    {
        case SHADE_FLAT:
        {
            ShadeTriangleGouraudFlat(v0, v1, v2, light, ambientIntensity, SHADE_FLAT);
        } break;

        case SHADE_PHONG:
        {
            ShadeTrianglePhong(v0, v1, v2, light, ambientIntensity);
        } break;

        case SHADE_GOURAUD:
        {
            ShadeTriangleGouraudFlat(v0, v1, v2, light, ambientIntensity, SHADE_GOURAUD);
        } break;
    }
}
} // namespace polygon_draw

// UTILITY SECTION ENDS HERE ------------------------------------------------------------
// CORE APPLICATION START HERE ----------------------------------------------------------
static bool
IsBackface(const vec3f& v0, const vec3f& v1, const vec3f& v2)
{
    vec3f normal = cross(v1 - v0, v2 - v0);
    vec3f camCenter = globalCamera.CameraOrigin();
    vec3f vToCam = camCenter - average(v0, v1, v2);

    return dot(vToCam, normal) <= 0;
}

static ClippedTriangle
ProcessTriangle(int32 index, Object3D *O, const mat4x4& transform, const mat4x4& rot, bool cullBackfaces = true)
{
    int i0 = O->ObjectModel->vertexIndices[index * 3];
    int i1 = O->ObjectModel->vertexIndices[index * 3 + 1];
    int i2 = O->ObjectModel->vertexIndices[index * 3 + 2];

    int in0 = O->ObjectModel->normalIndices[index * 3];
    int in1 = O->ObjectModel->normalIndices[index * 3 + 1];
    int in2 = O->ObjectModel->normalIndices[index * 3 + 2];

    vec3f v0 = O->ObjectModel->VertexPositions[i0];
    color4 c0 = O->ObjectModel->VertexColors[i0];
    vec3f v1 = O->ObjectModel->VertexPositions[i1];
    color4 c1 = O->ObjectModel->VertexColors[i1];
    vec3f v2 = O->ObjectModel->VertexPositions[i2];
    color4 c2 = O->ObjectModel->VertexColors[i2];

    vec3f n0 = O->ObjectModel->vertexNormals[in0];
    vec3f n1 = O->ObjectModel->vertexNormals[in1];
    vec3f n2 = O->ObjectModel->vertexNormals[in2];

    // Project into world and camera space
    v0 = v0 * transform;
    v1 = v1 * transform;
    v2 = v2 * transform;

    n0 = n0 * rot;
    n1 = n1 * rot;
    n2 = n2 * rot;

    // TODO(Reuel): All of this should be done camera side GlobalCamera.Project or something like this
    v0 = v0 * globalCamera.CameraViewMatrix();
    v1 = v1 * globalCamera.CameraViewMatrix();
    v2 = v2 * globalCamera.CameraViewMatrix();

    n0 = n0 * globalCamera.CameraRotation();
    n1 = n1 * globalCamera.CameraRotation();
    n2 = n2 * globalCamera.CameraRotation();

    ClippedTriangle T = {};
    T.IsIn = false;
    // Cull Backfaces
    if (IsBackface(v0, v1, v2) && cullBackfaces) return T;
    return ClipTriangle({v0, n0, c0}, {v1, n1, c1}, {v2, n2, c2}, globalCamera.CameraFrustum());
}

static void
DrawObjectSolid(Object3D *O)
{
    assert(O != nullptr);

    // Preliminary culling based on bounding volume (sphere here)
    if (!globalCamera.ObjectInFrustum(O))
    {
        return;
    }

    mat4x4 transform = O->ObjectTransform();
    mat4x4 rot = O->ObjectRotation();
    for (int i = 0; i < O->ObjectModel->in / 3; ++i)
    {
        ClippedTriangle triangles = ProcessTriangle(i, O, transform, rot);

        if (!triangles.IsIn) continue;

        if (globalRenderNormals) 
        {
            polygon_draw::DrawNormal(triangles.v0);
            polygon_draw::DrawNormal(triangles.v1);
            polygon_draw::DrawNormal(triangles.v2);
        }
        polygon_draw::ShadeTriangleInMode(triangles.v0, triangles.v1, triangles.v2, globalOmniLight, globalAmbientLight.intensity, globalShadingMode);

        if (triangles.IsSplit)
        {
            polygon_draw::ShadeTriangleInMode(triangles.v0, triangles.v2, triangles.v3, globalOmniLight, globalAmbientLight.intensity, globalShadingMode);
        }
    }
}

static void
DrawObjectWireframe(Object3D *O)
{
    assert(O != nullptr);

    // Preliminary culling based on bounding volume (sphere here)
    if (!globalCamera.ObjectInFrustum(O))
    {
        return;
    }

    mat4x4 transform = O->ObjectTransform();
    mat4x4 rot = O->ObjectRotation();
    for (int i = 0; i < O->ObjectModel->in / 3; ++i)
    {
        // Cull backfaces and faces outside frustum
        ClippedTriangle triangles = ProcessTriangle(i, O, transform, rot, false);

        if (!triangles.IsIn) continue;
        
        if (globalRenderNormals) 
        {
            polygon_draw::DrawNormal(triangles.v0);
            polygon_draw::DrawNormal(triangles.v1);
            polygon_draw::DrawNormal(triangles.v2);
        }

        polygon_draw::DrawWireframeTriangle(triangles.v0, triangles.v1, triangles.v2, RED);
        if (triangles.IsSplit)
        {
            polygon_draw::DrawWireframeTriangle(triangles.v0, triangles.v1, triangles.v2, RED);
        }
    }
}

static void
DrawObjectSolidWireframe(Object3D *O)
{
    assert(O != nullptr);

    // Preliminary culling based on bounding volume (sphere here)
    if (!globalCamera.ObjectInFrustum(O))
    {
        return;
    }

    mat4x4 transform = O->ObjectTransform();
    mat4x4 rot = O->ObjectRotation();
    for (int i = 0; i < O->ObjectModel->in / 3; ++i)
    {
        ClippedTriangle triangles = ProcessTriangle(i, O, transform, rot);

        if (!triangles.IsIn) continue;

        if (globalRenderNormals) 
        {
            polygon_draw::DrawNormal(triangles.v0);
            polygon_draw::DrawNormal(triangles.v1);
            polygon_draw::DrawNormal(triangles.v2);
        }

        polygon_draw::DrawWireframeTriangle(triangles.v0, triangles.v1, triangles.v2, YELLOW);
        polygon_draw::ShadeTriangleInMode(triangles.v0, triangles.v1, triangles.v2, globalOmniLight, globalAmbientLight.intensity, globalShadingMode);

        if (triangles.IsSplit)
        {
            polygon_draw::DrawWireframeTriangle(triangles.v0, triangles.v1, triangles.v2, YELLOW);
            polygon_draw::ShadeTriangleInMode(triangles.v0, triangles.v2, triangles.v3, globalOmniLight, globalAmbientLight.intensity, globalShadingMode);
        }
    }
}

static void
DrawObject(uint8 index)
{
    if (index >= worldObjects.size() || worldObjects.size() <= 0) return;

    if (globalRenderMode == RENDER_SOLID)
        DrawObjectSolid(worldObjects[index]);
    else if (globalRenderMode == RENDER_WIREFRAME)
        DrawObjectWireframe(worldObjects[index]);
    else if (globalRenderMode == RENDER_SOLID_WIREFRAME)
        DrawObjectSolidWireframe(worldObjects[index]);
}

void
ProcessInput(KeyCode Key)
{
    if (Key == KEY_F)
    {
        globalShadingMode = SHADE_FLAT;
    }

    if (Key == KEY_P)
    {
        globalShadingMode = SHADE_PHONG;
    }

    if (Key == KEY_G)
    {
        globalShadingMode = SHADE_GOURAUD;
    }

    if (Key == KEY_D)
    {
        globalRenderMode = RENDER_SOLID_WIREFRAME;
    }

    if (Key == KEY_W)
    {
        globalRenderMode = RENDER_WIREFRAME;
    }

    if (Key == KEY_S)
    {
        globalRenderMode = RENDER_SOLID;
    }

    if (Key == KEY_SPACE)
    {
        globalCamera.MoveBy(vec3f{0.f, -5.f, 0.f} * globalDeltaTime);
    }

    if (Key == KEY_LCTRL)
    {
        globalCamera.MoveBy(vec3f{0.f, 5.f, 0.f} * globalDeltaTime);
    }

    if (Key == KEY_Q)
    {
        worldObjects[globalObjectCursor]->RotateObjectY(60 * globalDeltaTime);
    }

    if (Key == KEY_E)
    {
        worldObjects[globalObjectCursor]->RotateObjectY(-60 * globalDeltaTime);
    }

    if (Key == KEY_UP)
    {
        globalOmniLight.position += vec3f{0, 50, 0} * globalDeltaTime;
    }

    if (Key == KEY_DOWN)
    {
        globalOmniLight.position += vec3f{0, -50, 0} * globalDeltaTime;
    }

    if (Key == KEY_LEFT)
    {
        globalOmniLight.position += vec3f{-50, 0, 0} * globalDeltaTime;
    }

    if (Key == KEY_RIGHT)
    {
        globalOmniLight.position += vec3f{50, 0, 0} * globalDeltaTime;
    }

    if (Key == KEY_N)
    {
        globalRenderNormals = !globalRenderNormals;
    }

    if (Key == KEY_1)
    {
        globalObjectCursor = 0;
    }

    if (Key == KEY_2)
    {
        globalObjectCursor = 1;
    }

    if (Key == KEY_3)
    {
        globalObjectCursor = 2;
    }

    if (Key == KEY_4)
    {
        globalObjectCursor = 3;
    }

    if (Key == KEY_5)
    {
        globalObjectCursor = 4;
    }

    if (Key == KEY_6)
    {
        globalObjectCursor = 5;
    }

    if (Key == KEY_7)
    {
        globalObjectCursor = 6;
    }

    if (Key == KEY_8)
    {
        globalObjectCursor = 7;
    }

    if (Key == KEY_9)
    {
        globalObjectCursor = 8;
    }
}

void
OnLaunchSetup(PlatformScreenDevice screenDevice, const std::vector<std::string>& objects)
{
    globalScreenDevice = screenDevice;
    globalDepthBuffer = new real32[globalScreenDevice.width * globalScreenDevice.height];
    globalRenderMode = RENDER_SOLID;
    globalShadingMode = SHADE_FLAT;
    globalOmniLight = point_light({-4, 10, 8}, 0.8f, 10.0f);
    globalAmbientLight = {0.2f};

    real32 vFov = 20.0f;
    real32 focalLength = 2.0f;
    vec3f origin = {0,0,0};
    globalCamera = camera(origin, focalLength, vFov, globalScreenDevice.aspectRatio, I_MATRIX_3X3);

    std::cout << "NUMBER OF OBJ FILES: " << objects.size() << std::endl;
    for (const auto& obj : objects)
    {
        Object3D *O = LoadObjectFromOBJ(obj.c_str(), {0,0,20}, 10.f);
        if (O != nullptr)
        {
            worldObjects.push_back(O);
        }
    }
    std::cout << "NUMBER OF LOADED OBJECTS: " << worldObjects.size() << std::endl;

    if (worldObjects.size() == 0)
    {
        worldObjects.push_back(CreateCube({0, 0, 12}, 4));
    }
}

void
OnShutdown()
{
    delete[] globalDepthBuffer;
    for (auto obj : worldObjects)
    {
        DestroyObject3D(obj);
    }
}

void
UpdateRenderLoop(real32 deltaTime)
{
    globalDeltaTime = deltaTime;
    screen_draw::BlackoutScreenBuffer(BLACK);
    DrawObject(globalObjectCursor);
}
// CORE APPLICATION END HERE --------------------------------------------------------------
} // namespace rastertoy
