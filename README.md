<!--
 * @Author: your name
 * @Date: 2020-06-08 17:27:23
 * @LastEditTime: 2020-06-08 17:30:02
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /wjgl/README.md
--> 
反复重构后第五次写软件render。仅供学习分享。

需要make、gnu gcc。

顶点着色演示程序：
```bash
make test                   # output: test.png
```

纹理演示程序：
```bash
make demo_texture           # output: demo_texture.png
```

光照演示程序：
```bash
make demo_light             # output: demo_light.png
```

如何使用？请移步`demo`文件夹下的程序。

# 技术特性

1. 只使用了标准库。不依赖第三方库。

1. 目前为止支持顶点着色模式、纹理、光照（单光源）。

1. 支持齐次裁剪空间中z=0平面的裁剪。可扩展成6面裁剪（利用现有代码修改后即可）。

1. 用户实现的片段着色器：`fshader(wg_render_t* render, wg_gbuff_t* gbuff)`，注册完成后可以使用。

1. 光照：由于片段着色器可以自定义，实现光照着色就很容易了。详见`demo/demo_light.c`。光照采用了GBuffer+延迟光照的技术，可以轻松扩展到多光源。

1. 纹理：实现了纹理的创建、存储，以及最近邻采样、双线性插值的支持。

1. 阴影：等我摸完鱼。

1. OBJ格式支持：等上面都摸完。

1. Gamma变换：等上面都摸完。
