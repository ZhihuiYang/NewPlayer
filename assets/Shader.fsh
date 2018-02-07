#version 300 es
precision mediump float; 
in vec2 texCoord;  
uniform sampler2D y_texture;  
uniform sampler2D u_texture;  
uniform sampler2D v_texture;
layout(location = 0) out vec4 outColor;   
void main(void)  
{  
    mediump vec3 yuv;  
    lowp vec3 rgb;      
    yuv.x = texture(y_texture, texCoord).r;
    yuv.y = texture(u_texture, texCoord).r - 0.5;
    yuv.z = texture(v_texture, texCoord).r - 0.5;
    rgb = mat3( 1,       1,         1,  
                0,       -0.39465,  2.03211,  
                1.13983, -0.58060,  0) * yuv;      
    outColor = vec4(rgb, 1);
}  