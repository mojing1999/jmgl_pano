/***********************************************      
  *     OpenGL Panoramic View
  *     Author:     Justin Mo 
  *     Email:      mojing1999@gmail.com
  *     Date:       2017.04.15
  *     Version:   V0.01
  ***********************************************
  *     Shader program
  ***********************************************
  */
#ifndef _JMGL_SHADER_H_
#define _JMGL_SHADER_H_

const char *vertext_id = "#version 330 core \n"
"layout(location = 0) in vec3 position; \n"
"layout(location = 2) in vec2 texCoord; \n"
"out vec2 TexCoord; \n"
"uniform mat4 model; \n"
"uniform mat4 view; \n"
"uniform mat4 proj; \n"
"void main()	\n"
"{	\n"
"	gl_Position = proj * view * model * vec4(position, 1.0f);	\n"
"	TexCoord = vec2(1.0 - texCoord.x, 1.0 - texCoord.y);	\n"
"}	\n\0";

const char *fragment_yuv_id = "#version 330 core	\n"
"in vec2 TexCoord;	\n"
"out vec4 color;	\n"
"uniform sampler2D tex_y;	\n"
"uniform sampler2D tex_u;	\n"
"uniform sampler2D tex_v;	\n"
"void main()	\n"
"{	\n"
" vec3 yuv, rgb;	\n"
"yuv.x = texture(tex_y, TexCoord).x; \n"
"yuv.y = texture(tex_u, TexCoord).x - 0.5; \n"
"yuv.z = texture(tex_v, TexCoord).x - 0.5; \n"
"rgb = mat3(1, 1, 1,       0, -0.39465, 2.03211,    1.13983, -0.58060, 0) * yuv; \n"
"	color = vec4(rgb, 1.0);	\n"
"}	\n\0";



const char *fragment_rgb_id = "#version 330 core	\n"
"in vec2 TexCoord;	\n"
"out vec4 color;	\n"
"uniform sampler2D tex_rgb;	\n"
"void main()	\n"
"{	\n"
"	color = texture(tex_rgb, TexCoord);	\n"
"}	\n\0";


#endif // !_JMGL_SHADER_H_

