//
//  VCCRenderingEngine1.cpp
//  opengles1
//
//  Created by qiu on 05/06/2017.
//  Copyright © 2017 qiu. All rights reserved.
//

//VCCRenderingEngine1类和工厂方法

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include "VCCRenderingEngine.hpp"
#define STRINGIFY(A) #A
#include "fs.glsl"
#include "vs.glsl"

//浮点常量以定义对应的角速度；
static const float RevolutionsPerSecond = 1;

class VCCRenderingEngine2 : public VCCRenderingEngine{
public:
    VCCRenderingEngine2();
    void Initialize(int width, int height);
    void Render() const;
    void UpdateAnimation(float timeStep);
    void OnRotate(VCCDeviceOrientation newOrientation);
private:
    float RotationDirection() const;
    GLuint BuildShader(const GLchar* source, GLenum shaderType) const;
    GLuint BuildProgram(const GLchar* vertexPath, const GLchar* fragmentPath) const;
    void ApplyOrtho(float maxX, float maxY) const;
    void ApplyRotation(float degrees) const;
    float m_desiredAngle;
    float m_currentAngle;
    GLuint m_simpleProgram;
    GLuint m_framebuffer;
    GLuint m_renderbuffer;
};

//其中， UpdateAnimation() 和 OnRotate()通过桩函数（存根函数）实现，且需要进一步完善以支持旋转操作

struct Vertex{
    float Position[2];
    float Color[4];
};

const struct Vertex Vertices[] ={
    {{-0.5, -0.866}, {1, 1, 0.5f, 1}},
    {{0.5, -0.866}, {1, 1, 0.5f, 1}},
    {{0, 1}, {1, 1, 0.5f, 1}},
    {{-0.5, -0.866}, {0.5f, 0.5f, 0.5f, 1}},
    {{0.5, -0.866}, {0.5f, 0.5f, 0.5f, 1}},
    {{0, -0.4}, {0.5f, 0.5f, 0.5f, 1}},
};
VCCRenderingEngine* CreateRenderer2()
{
    return new VCCRenderingEngine2();
}
VCCRenderingEngine2::VCCRenderingEngine2()
{
    //生成渲染缓冲区操作符并将其绑定至管线上。
    //去掉 ES1 中的 OES
    glGenRenderbuffers(1, &m_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);
}

//Initialize()方法将构建视口变换居正以及投影矩阵。其中，投影矩阵定义了一个当前可见场景的 3D 空间
void VCCRenderingEngine2::Initialize(int width, int height)
{
    //Create the framebuffer object and attach the color buffer
    //创建帧缓冲对象并将渲染缓冲区绑定至该对象之上。
    //通过 glViewport 和 glOrthof 确定顶点的变换状态。
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderbuffer);
    //光栅化之前的最后一步从标准化设备空间通过视口转换到窗口空间
    glViewport(0, 0, width, height);
    //glViewport() 函数用于控制窗口空间内的 x、y 值转换操作，而窗口空间内的 z 值转换则采用了不同的 OpenGL 函数如下
    //glDepthRangef(near, far)
    //在实际操作过程中，该函数并不常见，且参数 near 和 far 的默认值分别为 0 和 1.
    m_simpleProgram = BuildProgram(SimpleVertxShader, SimpleFragShader);
    //从 OpenGL 的角度来看，程序（program）可视为由多个链接在一起的着色器程序所构成的模块。
    glUseProgram(m_simpleProgram);
    //Initialize the projection matrix
    ApplyOrtho(2,3);
    
    // Initialize the rotation animation state
    OnRotate(VCCDeviceOrientationPortrait);
    m_currentAngle = m_desiredAngle;
}

//针对平滑旋转操作，Apple 通过 UIViewController 类提供了相应的底层实现方案，但这并非 OpenGL ES 所推荐的方法，其原因如下
//处于性能考虑，Apple 建议应避免 Core Animation 和 OpenGL 之间的交互操作。
//理想情况下，在应用程序的生命周期内，渲染缓冲区应保持相同的尺寸和高宽比，这将有利于提高程序的运行性能并降低代码的复杂度
//在图形应用程序中，开发人员需要全面地对动画和渲染进行操控
//
//
//
GLuint VCCRenderingEngine2::BuildShader(const GLchar* source, GLenum shaderType) const
{
    
    GLuint shaderHandle = glCreateShader(shaderType);
    glShaderSource(shaderHandle, 1, &source, 0);
    glCompileShader(shaderHandle);
    GLint compileSuccess;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
    if (compileSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetShaderInfoLog(shaderHandle, sizeof(messages), 0, &messages[0]);
        std::cout << messages;
        exit(1);
    }
    return shaderHandle;
}

GLuint VCCRenderingEngine2::BuildProgram(const GLchar* vertexPath, const GLchar* fragmentPath) const
{
    GLuint vertexShader = BuildShader(vertexPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = BuildShader(fragmentPath, GL_FRAGMENT_SHADER);
    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vertexShader);
    glAttachShader(programHandle, fragmentShader);
    glLinkProgram(programHandle);
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    if (linkSuccess == GL_FALSE) {
        GLchar messages[256];
        glGetProgramInfoLog(programHandle, sizeof(messages), 0, &messages[0]);
        std::cout << messages;
        exit(1);
    }
    return programHandle;
}

void VCCRenderingEngine2::ApplyOrtho(float maxX, float maxY) const
{
    //顶点的位置属性通常不可或缺，因而该属性在 OpenGL ES 1.1 规范中稍显特殊，他可以定义为 2D、3D 或 4D 坐标值。从本质上讲，OpenGL 一般会将坐标转换为 4D 浮点数。
    //将 3D 坐标转换为其次坐标时，第四项内容（即w）的默认值为1。w = 0 则比较少见，这也意味着对应顶点位于无穷远处（在 OpenGL 中，少数采用 w = 0 的场合通常出现于光源定位情形中。
    //其次坐标最早出现在莫比乌斯的书里，同时莫比乌斯还提出了质心坐标这一概念，当计算三角形内部颜色值时，iPhone 图形芯片要求提供相关位置的质心坐标。
    //w项的除法计算也称作透视转换。并未舍去。
    //正交投影位于原中心位置可视为缩放矩阵的一个特例。
    //ES 1.1 全 3D 渲染需要使用 glfrustumf 函数设置投影矩阵，参数等同于 glOrthof
    
    
    
    float a = 1.0f / maxX;
    float b = 1.0f / maxY;
    float ortho[16] = {
        a, 0, 0, 0,
        0, b, 0, 0,
        0, 0, -1, 0,
        0, 0, 0, 1
    };
    GLint projectUniform = glGetUniformLocation(m_simpleProgram, "Projection");
    glUniformMatrix4fv(projectUniform, 1, 0, &ortho[0]);
    
    
}



void VCCRenderingEngine2::OnRotate(VCCDeviceOrientation newOrientation)
{
    float angle = 0;
    switch (newOrientation) {
        case VCCDeviceOrientationLandscapeLeft:
            angle = 270;
            break;
        case VCCDeviceOrientationPortraitUpsideDown:
            angle = 180;
            break;
        case VCCDeviceOrientationLandscapeRight:
            angle = 90;
            break;
        default:
            break;
    }
    m_desiredAngle = angle;
}

//程序考察箭头的旋转方向问题，即顺时针还是逆时针旋转。此处，仅检测期望值是否大于当前角度值并不充分：若用户将设备方位从 270 改变至 0，则该角度值应增至 360。
//根据箭头的旋转方向，该方法将返回 -1、0 或 +1。这里，假设 m_currentAngle 和 m_desiredAngle 为 0（含）到 360（不含）之间的角度值
float VCCRenderingEngine2::RotationDirection() const
{
    float delta = m_desiredAngle - m_currentAngle;
    if (delta == 0) {
        return 0;
    }
    bool counterclockwise = ((delta > 0 && delta <= 180) || (delta < -180));
    return counterclockwise ? +1 :-1;
}


void VCCRenderingEngine2::ApplyRotation(float degrees) const
{
    float radians = degrees * 3.14159 / 180.0f;
    float s = std::sin(radians);
    float c = std::cos(radians);
    float zRotation[16] = {
        c, s, 0, 0,
        -s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    GLint modelviewUniform = glGetUniformLocation(m_simpleProgram, "Modelview");
    glUniformMatrix4fv(modelviewUniform, 1, 0, &zRotation[0]);
}



void VCCRenderingEngine2::Render() const
{
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    

    //这个旋转有点奇怪，似乎是在视图旋转之后再进行旋转的。
    ApplyRotation(m_currentAngle);
    
    GLuint positionSlot = glGetAttribLocation(m_simpleProgram, "Position");
    GLuint colorSlot = glGetAttribLocation(m_simpleProgram, "SourceColor");
    //ES 1.1 通过 OpenGL 所提供的常量设置相关属性值；
    //ES 2.0 则使用着色器程序中所定义的常量值 （positionSlot 和 colorSlot）。
    
    glEnableVertexAttribArray(positionSlot);
    glEnableVertexAttribArray(colorSlot);
    GLsizei stride = sizeof(Vertex);
    const GLvoid* pCoords = &Vertices[0].Position[0];
    const GLvoid* pColors = &Vertices[0].Color[0];
    
    //另外，两种规范均确定了参数的数量以及顶点属性的类型
    glVertexAttribPointer(positionSlot, 2, GL_FLOAT, GL_FALSE, stride, pCoords);
    glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, pColors);
    

    
    GLsizei vertexCount = sizeof(Vertices) / sizeof(Vertex);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);//该函数调用即可令 OpenGL 从定义于 gl*Pointer 中的指针获取数据，同时三角形数据将渲染至目标表面上。
    
    glDisableVertexAttribArray(positionSlot);
    glDisableVertexAttribArray(colorSlot);
}




void VCCRenderingEngine2::UpdateAnimation(float timeStep)
{
    float direction = RotationDirection();
    if (direction == 0) {
        return;
    }

    float degrees = timeStep * 360 * RevolutionsPerSecond;
    m_currentAngle += degrees * direction;
    //Ensure that the angle stays within [0, 360)
    if (m_currentAngle > 360) {
        m_currentAngle -= 360;
    } else if(m_currentAngle < 0){
        m_currentAngle += 360;
    }
    std::cout<< "1 " <<m_currentAngle << std::endl;
    //If the rotation direction changed, then we overshot the desired angle
    if (RotationDirection() != direction) {
        m_currentAngle = m_desiredAngle;
    }
    std::cout<< "2 " << m_currentAngle << std::endl;
}
