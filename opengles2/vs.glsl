const char* SimpleVertxShader = STRINGIFY(
attribute vec4 Position;
attribute vec4 SourceColor;
varying vec4 DestinationColor;
uniform mat4 Projection;
uniform mat4 Modelview;
void main(void)
{
    DestinationColor = SourceColor;
    gl_Position = Projection * Modelview * Position;
}
);

//相机的位置和方向应在模式-视图中加以定义。由于投影将影响到视域，因而更倾向于将其视为相机的边角透镜。
//相机的位置和方向通常在模型-视图而非投影中加以处理，OpenGL ES 1.1 据此执行正确的光照计算。


//uniform 变量可视为一类着色器程序无法修改的常量
//首先，顶点着色器声明了一个属性集，可暂且将其视为顶点着色器和外部环境之间的连接点。顶点着色器只是简单地传递颜色值属性并执行标准的位置变换操作。

//基于顶点的图元装配
//对象的 3D 外观通常被称为几何形状。在 OpenGL 中，对象的几何形状一般由一系列的图元构成，例如三角形、点以及直线。 同时，图元往往采用定点数组加以定义，并根据顶点的拓扑关系加以链接。 OpenGL ES 支持7种拓扑关系。
//OpenGL 内置了 GL_QUADS 图元类型以供使用，但 OpenGL ES 并不支持四边形。

//OpenGL 建立了以下3类矩阵
//调整相机的视角，这将使用到投影矩阵
//定位相机并调整观察方向，这将用到视图矩阵
//对各对象进行缩放、旋转以及平移操作，这将使用到模型矩阵

//当采用透视投影后，视域将呈现为某以视见体形状。有时，视见体可形象地描述为一个棱锥且观察点位于该棱锥的顶点处。视见体还可通过锥角加以计算（即视域），与定义视见体的全部6个面相比，该方案更加直观。



