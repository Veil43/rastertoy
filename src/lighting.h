#pragma once

/*
16 bytes
*/
class point_light
{
public:
    point3f position;
    real32 intensity;
    real32 specularity;

    point_light() {}
    point_light(vec3f p, real32 i, real32 s) : position{p}, intensity{i}, specularity{s} {}
    
    real32 GetIntensityPhong(const vertex3& v, const vec3f cameraDirection) const
    {
        vec3f lightDirection = position - v.point;
        lightDirection = normalize(lightDirection);
        real32 normDotdir = dot(v.normal, lightDirection);
        if (normDotdir > 0)
        {
            vec3f reflectionVector = 2 * v.normal * dot(v.normal, lightDirection) - lightDirection;
            real32 specularIntensity = dot(reflectionVector, cameraDirection) /
                                    std::sqrt(length_squared(reflectionVector) * length_squared(cameraDirection)); // you sombich!
            return intensity * (normDotdir + std::pow(specularIntensity, specularity));
        }

        return 0.0f;
    }

    real32 GetIntensityFlat(const vec3f& n, const vec3f& center) const
    {
        vec3f lightDirection = position - center;
        lightDirection = normalize(lightDirection);
        return intensity * dot(n, lightDirection);
    }

    real32 GetIntensityGouraud(const vertex3& v) const
    {
        vec3f lightDirection = position - v.point;
        lightDirection = normalize(lightDirection);
        return intensity * dot(v.normal, lightDirection);
    }
};

struct ambient_light
{
    real32 intensity;
};