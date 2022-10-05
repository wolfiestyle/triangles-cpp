#ifndef __SHADER_SOURCE_HPP
#define __SHADER_SOURCE_HPP

namespace glsl {

char const* color_vert = R"___(
#version 430

uniform float test;

in vec2 position;
in vec4 color;

out vec4 vcolor;

void main()
{
    vcolor = color * test;
    gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);
}
)___";

char const* color_frag = R"___(
#version 430

in vec4 vcolor;

out vec4 frag_color;

void main()
{
    frag_color = vcolor;
}
)___";

} // namespace glsl

#endif // __SHADER_SOURCE_HPP
