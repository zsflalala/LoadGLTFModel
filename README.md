# LoadGLTFModel 
Use tiny_gltf to load gltf model file and use Opengl 3.3 to render a dragon gltf model.

## Environment

VisualStidop 2022

OpenGL 3.3

GLFW3

GLAD

## Setup

### nuget环境配置

#### 配置D:/vc_packages

在目录`C:\Users\$NAME\AppData\Roaming\NuGet`下新建`NuGet.Config`文件，内容如下

- `$NAME`表示自己的用户名
- 可能存在该文件，可将原本的文件更名为`NuGet.Config-Default`

![image.png](https://cdn.nlark.com/yuque/0/2024/png/29081253/1718766370597-dbe911b2-36d1-4974-a0d8-ed60378470d4.png?x-oss-process=image%2Fformat%2Cwebp)

```XML
<?xml version="1.0" encoding="utf-8"?>
<configuration>
  <packageSources>
    <add key="nuget.org" value="https://api.nuget.org/v3/index.json" protocolVersion="3" />
    <add key="globalPackagesFolder" value="D:\vc_packages" />
    <add key="Microsoft Visual Studio Offline Packages" value="C:\Program Files (x86)\Microsoft SDKs\NuGetPackages\" />
  </packageSources>

  <solution>
    <!-- 默认不将 packages 提交到源代码管理 -->
    <add key="disableSourceControlIntegration" value="true" />
  </solution>
  <config>
    <!-- 指定这个目录下默认的 packages 目录 -->
    <add key="repositorypath" value="D:\vc_packages" />
  </config>
  <packageRestore>
    <!-- 默认启用 packages 还原 -->
    <add key="enabled" value="True" />
  </packageRestore>

</configuration>
```

## Reference

1. [GLTF的文件结构解读，由浅入深版](https://blog.csdn.net/qq_51802524/article/details/141788273)





