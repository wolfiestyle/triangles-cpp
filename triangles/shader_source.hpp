#ifndef __SHADER_SOURCE_HPP
#define __SHADER_SOURCE_HPP

namespace glsl {

char const* color_vert = R"___(
#version 430

in vec2 position;
in vec4 color;

out vec4 vcolor;

void main()
{
    vcolor = color;
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

char const* tex_vert = R"___(
#version 430

in vec2 coord;

out vec2 vtexc;

void main()
{
    vtexc = coord;
    vec2 vpos = vec2(coord.x, 1.0 - coord.y) * 2.0 - 1.0;
    gl_Position = vec4(vpos, 0.0, 1.0);
}
)___";

char const* tex_frag = R"___(
#version 430

uniform sampler2D tex;

in vec2 vtexc;

out vec4 frag_color;

void main()
{
    frag_color = texture(tex, vtexc);
}
)___";

} // namespace glsl

#endif // __SHADER_SOURCE_HPP
