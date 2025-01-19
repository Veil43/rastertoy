#pragma once

/*
4 bytes
*/
struct color4
{
    unsigned char r, g, b, a;
};

const color4 NO_COLOR   = {0xFF, 0xFF, 0xFF, 0x00};
const color4 WHITE      = {0xFF, 0xFF, 0xFF, 0xFF};
const color4 BLACK      = {0x00, 0x00, 0x00, 0xFF};
const color4 RED        = {0xFF, 0x00, 0x00, 0xFF};
const color4 GREEN      = {0x00, 0xFF, 0x00, 0xFF};
const color4 BLUE       = {0x00, 0x00, 0xFF, 0xFF};
const color4 YELLOW     = {0xFF, 0xFF, 0x00, 0xFF};
const color4 PURPLE     = {0xFF, 0x00, 0xFF, 0xFF};
const color4 CYAN       = {0x00, 0xFF, 0xFF, 0xFF};
const color4 GREY       = {0xAA, 0xAA, 0xAB, 0xFF};
const color4 LIGHT_BG   = {0xAF, 0xBF, 0xAE, 0xFF};
const color4 DEFAULT    = {0x65, 0x39, 0xAA, 0xFF};

inline constexpr color4 operator*(double t, color4 c) 
{
    return { static_cast<unsigned char>(t * c.r), 
             static_cast<unsigned char>(t * c.g), 
             static_cast<unsigned char>(t * c.b), 
             c.a };
}

inline constexpr bool operator==(const color4& c1, const color4& c2)
{ 
    return c1.r == c2.r && 
           c1.g == c2.g && 
           c1.b == c2.b && 
           c1.a == c2.a;
}

inline constexpr bool operator!=(const color4& c1, const color4& c2)
{
    return !(c1 == c2);
}

unsigned int
color_uint32(const color4& color)
{
    return (static_cast<unsigned int>(color.r) << 24) |
           (static_cast<unsigned int>(color.g) << 16) |
           (static_cast<unsigned int>(color.b) << 8) |
           (static_cast<unsigned int>(color.a));
}

unsigned int
greyscale_color_uint32(float value)
{
    value = static_cast<float>((clamp(value, -50.0f, 50.0f) + 50 ) / 100.0f);

    const float gamma = 0.8f;
    value = std::pow(value, gamma);

    const unsigned int intensity = static_cast<unsigned int>(255.0f - (value * 255.0f));

    return (intensity << 24) | (intensity << 16) | (intensity << 8) | 0xFF;
}

color4
greyscale_color4(float value)
{
    value = static_cast<float>((clamp(value, -50.0f, 50.0f) + 50 ) / 100.0f);

    const float gamma = 0.8f;
    value = std::pow(value, gamma);

    const unsigned char intensity = static_cast<unsigned char>(255.0f - (value * 255.0f));

    return {intensity, intensity, intensity, 0xFF};
}

void
swap(color4& v, color4& u)
{
    auto t = v;
    v = u;
    u = t;
}

color4
linear_interpolate_color(float i, float start_value, color4 start_color, float end_value, color4 end_color)
{
    auto r = static_cast<unsigned char>(linear_interpolate(i, start_value, start_color.r, end_value, end_color.r)); 
    auto g = static_cast<unsigned char>(linear_interpolate(i, start_value, start_color.g, end_value, end_color.g)); 
    auto b = static_cast<unsigned char>(linear_interpolate(i, start_value, start_color.b, end_value, end_color.b)); 

    return { r, g, b, 0xFF }; 
}

inline std::ostream&
operator<<(std::ostream& o, const color4& c)
{
    o << "(" << (int) c.r << ", " << (int) c.g << ", " << (int) c.b << ", " << (int)c.a << ")";
    return o;
}

inline constexpr color4
operator*(float i, const color4& c)
{
    float r = clamp(i * c.r, 0, 255);
    float g = clamp(i * c.g, 0, 255);
    float b = clamp(i * c.b, 0, 255);
    return 
    {
        static_cast<unsigned char>(r),
        static_cast<unsigned char>(g),
        static_cast<unsigned char>(b),
        c.a
    };
}

inline constexpr color4
operator*(const color4& c, float i)
{
    return i * c;
}

inline constexpr color4&
operator*=(color4& c, float i)
{
    c = i * c;
    return c;
}