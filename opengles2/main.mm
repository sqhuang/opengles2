//
//  main.m
//  opengles2
//
//  Created by qiu on 06/06/2017.
//  Copyright © 2017 qiu. All rights reserved.
//
//尝试用 OpenGL ES 2.0 规范编写新型的渲染引擎，进而彰显 OpenGL ES 1.1 和 ES 2.0 之间的不同之处。Khronos 并未令 ES 2.0 与 ES 1.0 规范实现前向兼容，同时相关 API 更加简练皆不失优雅性。

//着色器
//着色语言可视为 OpenGL 规范中的一个显著特征。着色器是一类运行于图形处理器上的小型的代码段，通常情况下课分为两类：顶点着色器和片元着色器。顶点着色器负责对 glDrawArrays 中的顶点进行转换操作；而片元着色器则负责计算三角形内各个像素的颜色值。鉴于图形处理器高效的并行计算特征，课同步执行多个着色器实例。
//着色器采用类 C 语言加以编写，称为 OpenGL 着色语言（GLSL）。不同于 C 语言，GLSL 程序无法再 Xcode 内进行编译，但在运行期间， iPhone 自身则可对着色器程序进行编译。另外，应用程序将着色器源程序以 C 语言字符串的形式提交至 OpenGL API处，随后，OpenGL 将其编译为机器码。
//某些 OpenGL ES 还支持离线编译，其中，应用程序在运行期内提交二进制而非字符串。目前（这本书至少是10年的）iPhone 仅支持运行期内的着色器程序编译，其 arm 处理器对着色器程序进行编译并将最终的机器码发送至图形处理器上以供执行。


#import <UIKit/UIKit.h>
#import "AppDelegate.h"

int main(int argc, char * argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
