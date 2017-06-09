//
//  VCCRenderingEngine1.cpp
//  opengles1
//
//  Created by qiu on 05/06/2017.
//  Copyright © 2017 qiu. All rights reserved.
//

//VCCRenderingEngine1类和工厂方法

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include "VCCRenderingEngine.hpp"
//浮点常量以定义对应的角速度；
static const float RevolutionsPerSecond = 1;

class VCCRenderingEngine1 : public VCCRenderingEngine{
public:
    VCCRenderingEngine1();
    void Initialize(int width, int height);
    void Render() const;
    void UpdateAnimation(float timeStep);
    void OnRotate(VCCDeviceOrientation newOrientation);
private:
    float RotationDirection() const;
    
    float m_desiredAngle;
    float m_currentAngle;
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
VCCRenderingEngine* CreateRenderer1()
{
    return new VCCRenderingEngine1();
}
VCCRenderingEngine1::VCCRenderingEngine1()
{
    //生成渲染缓冲区操作符并将其绑定至管线上。
    glGenRenderbuffersOES(1, &m_renderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_renderbuffer);
}

//Initialize()方法将构建视口变换居正以及投影矩阵。其中，投影矩阵定义了一个当前可见场景的 3D 空间
void VCCRenderingEngine1::Initialize(int width, int height)
{
    //Create the framebuffer object and attach the color buffer
    //创建帧缓冲对象并将渲染缓冲区绑定至该对象之上。
    //通过 glViewport 和 glOrthof 确定顶点的变换状态。
    glGenFramebuffersOES(1, &m_framebuffer);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_framebuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, m_renderbuffer);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    //initialize the projection matrix
    const float maxX = 2;
    const float maxY = 3;
    glOrthof(-maxX, maxX, -maxY, maxY, -1, 1);
    glMatrixMode(GL_MODELVIEW);
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
void VCCRenderingEngine1::OnRotate(VCCDeviceOrientation newOrientation)
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

void VCCRenderingEngine1::Render() const
{
    glClearColor(0.5f, 0.5f, 0.5f, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    //为了防止累积效应，代码中还添加了调用 glPushMatrix() 和 glPopMatrix()
    glPushMatrix();
    glRotatef(m_currentAngle, 0, 0, 1);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &Vertices[0].Position[0]);
    glColorPointer(4, GL_FLOAT, sizeof(Vertex), &Vertices[0].Color[0]);
    GLsizei vertexCount = sizeof(Vertices) / sizeof(Vertex);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);//该函数调用即可令 OpenGL 从定义于 gl*Pointer 中的指针获取数据，同时三角形数据将渲染至目标表面上。
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    //关闭两个顶点属性。在执行绘制命令时，需要开启相关的顶点属性，但当后续绘制命令采用完全不同的垫垫属性集时，保留原有的属性并非上次。
}

//程序考察箭头的旋转方向问题，即顺时针还是逆时针旋转。此处，仅检测期望值是否大于当前角度值并不充分：若用户将设备方位从 270 改变至 0，则该角度值应增至 360。
//根据箭头的旋转方向，该方法将返回 -1、0 或 +1。这里，假设 m_currentAngle 和 m_desiredAngle 为 0（含）到 360（不含）之间的角度值

float VCCRenderingEngine1::RotationDirection() const
{
    float delta = m_desiredAngle - m_currentAngle;
    if (delta == 0) {
        return 0;
    }
    bool counterclockwise = ((delta > 0 && delta <= 180) || (delta < -180));
    return counterclockwise ? +1 :-1;
}

void VCCRenderingEngine1::UpdateAnimation(float timeStep)
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
    
    //If the rotation direction changed, then we overshot the desired angle
    if (RotationDirection() != direction) {
        m_currentAngle = m_desiredAngle;
    }
    
}
