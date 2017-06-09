const char* SimpleFragShader = STRINGIFY(
varying lowp vec4 DestinationColor;
void main(void)
{
    gl_FragColor = DestinationColor;
}
);
//此处，varying 参数定义为一个连接点。片元着色器仅对既定的颜色值执行相应的传输操作。
