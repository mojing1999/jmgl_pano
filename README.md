
## JMGL_PANO
OpenGL Panorama Player.

star_war_eve source 1

![Source1](http://mojing1999.github.io/media/pano_source1.jpg)


star_war_eve source 2

![Source1](http://mojing1999.github.io/media/pano_source2.jpg)

## 1. 介绍
JMGL_PANO 是Justin开源的一个全景视频播放器（[Github](https://github.com/mojing1999/jmgl_pano)）。基于硬件加速的视频解码和OpenGL渲染。仅供个人学习使用，商业使用请联系作者(Justin's Email: mojing1999@gmail.com)

## 2. 相关库

- OpenGL - OpenGL V3.3
- GLEW - OpenGL核心库操作
- GLFW - 和window相关操作
- FFmpeg - 媒体解码库
- pthread - 线程库
- Intel Media SDK - 解码库
- NV CUDA - 解码库
- Video codec - 作者基于硬件加速的视频库
- AnyOption	- http://www.hackorama.com/anyoption/

---

## 3. 框架结构
 - FFmpeg/video_codec 媒体处理，输出YUV420图像
 - OpenGL 创建模型，贴图，渲染
 - GLFW 创建显示窗口，处理鼠标键盘输入


## 4. 使用
 - Usage: jmgl_pano [option] input_file
 - option: -d decode type, user can select decode library.
    - 0: Auto, 1: Nvidia, 2: Intel, 3: ffmpeg
 - switch view mode by keyboard input:
    - F1 : Rectilinear
    - F2 : Mirror Ball
    - F3 : Fisheye
    - F4 : Little Planet
    - 'a' : Auto rotation
    - 't' : Stop auto rotation
    


## 程序截图
- 全景预览

![rectilinear](http://mojing1999.github.io/media/pano_rectilinear.jpg)

- 水晶球

![mirror](http://mojing1999.github.io/media/pano_mirror_ball.jpg)

- 鱼眼

![fisheye](http://mojing1999.github.io/media/pano_fisheye.jpg)

- 小行星

![planet](http://mojing1999.github.io/media/pano_little_planet.jpg)






## 5. TODO list
- [x] 处理键盘输入
- [x] 处理鼠标输入
- [ ] 处理触摸屏输入
- [x] 全景模式变换
    - [x] 全景浏览
    - [x] 鱼眼模式
    - [x] 水晶球模式
    - [x] 小行星模式
- [x] 优化OpenGL渲染
- [x] 媒体文件硬解码支持
- [ ] 贴图编码
