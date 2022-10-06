#ifndef __SHADER_SOURCE_HPP
#define __SHADER_SOURCE_HPP

namespace glsl {

char const* color_vert = R"___(
#version 430

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

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

char const* fold_comp = R"___(
#version 430
layout (local_size_x = 8, local_size_y = 8) in;

uniform sampler2D src;
uniform writeonly restrict image2D dest;

#define tid gl_LocalInvocationIndex
const uint block_size = gl_WorkGroupSize.x * gl_WorkGroupSize.y;

shared vec4 sdata[block_size];

vec4 fold_op(vec4 acc, vec4 val) { return acc + val; }

void main() {
    ivec2 i = ivec2(2 * gl_WorkGroupSize.xy * gl_WorkGroupID.xy + gl_LocalInvocationID.xy);
    vec4 t0 = texelFetch(src, i, 0);
    vec4 t1 = texelFetch(src, i + ivec2(gl_WorkGroupSize.x, 0), 0);
    vec4 t2 = texelFetch(src, i + ivec2(0, gl_WorkGroupSize.y), 0);
    vec4 t3 = texelFetch(src, i + ivec2(gl_WorkGroupSize.xy), 0);
    sdata[tid] = fold_op(fold_op(fold_op(t0, t1), t2), t3);
    barrier();

    for (uint s = block_size/2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata[tid] = fold_op(sdata[tid], sdata[tid + s]);
        }
        barrier();
    }

    if (tid == 0) {
        imageStore(dest, ivec2(gl_WorkGroupID.xy), sdata[0]);
    }
}
)___";

char const* scale_comp = R"___(
#version 430
layout (local_size_x = 16, local_size_y = 16) in;

uniform sampler2D src;
uniform writeonly restrict image2D dest;

void main()
{
    vec2 img_size = vec2(gl_WorkGroupSize.xy * gl_NumWorkGroups.xy);
    vec2 uv = (vec2(gl_GlobalInvocationID.xy) + 0.5) / img_size;
    ivec2 i = ivec2(gl_GlobalInvocationID.xy);
    vec4 val = texture(src, uv);
    imageStore(dest, i, val);
}
)___";

char const* dsq_comp = R"___(
#version 430
layout (local_size_x = 16, local_size_y = 16) in;

uniform sampler2D src1;
uniform sampler2D src2;
uniform writeonly restrict image2D dest;

void main()
{
    ivec2 i = ivec2(gl_GlobalInvocationID.xy);
    vec4 diff = texelFetch(src1, i, 0) - texelFetch(src2, i, 0);
    imageStore(dest, i, diff * diff);
}
)___";

} // namespace glsl

#endif // __SHADER_SOURCE_HPP
