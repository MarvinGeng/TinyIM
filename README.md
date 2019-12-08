# TinyIM

## 0. 项目简介

一个基于C++的IM实现     
最初的客户端设计来自flamingo https://github.com/balloonwj/flamingo      
服务器是我完全重写的,个人blog https://www.dennisthink.com/      
有问题可以联系 dennismi1024@gmail.com            
或者加入QQ群：106350930                    
文档链接:https://dennisthink.github.io/AboutAuther.html             

## 1.编译介绍
TinyIM采用C++语言进行开发,CMake工具进行编译,目前已经在Windows7的64位上使用VS2017编译通过了。
因为代码中使用了C++11的标准,所以需要使用支持C++11标准的编译器,也就是VS2013以后的版本。
[Windows平台配置CMake](https://www.dennisthink.com/?p=380)
TinyIM主要包括3个部分的程序,分别为Server、ClientCore、ClientUI。
以下分别介绍各个部分的功能的编译步骤。

## 1. Windows平台编译
### 1.1 Server编译
ChatServer的代码在 ```\TinyIM\Code\Server\ChatServer\```目录下。
![ChatServerDir](./Doc/TinyIM的Server目录.jpg)
在该目录下双击build32.bat即可开始执行编译。程序编译完成后,生成的可执行文件位于 ```\TinyIM\Code\Server\ChatServer\bin\Debug\```目录下。ChatServer即为可执行文件。

### 1.2 ClientCore编译

ClientCore的代码位于```\TinyIM\Code\Client\ClientCore_TinyIM```目录下,
![ClientCoreDir](./Doc/TinyIM的ClientCore目录.jpg)
双击该目录下的build32.bat即可开始执行编译。
编译完成以后,生成的可执行文件位于```\TinyIM\Code\Client\ClientCore_TinyIM\bin\Debug```目录下,*ClientCore.exe* 即为可执行文件。

### 1.3 ClientUI编译
ClientUI的代码在```\TinyIM\Code\Client\ClientUI```目录下,
![ClientUIDir](./Doc/TinyIM的ClientUI的目录.jpg)
双击该目录下的Build.bat即可开始执行编译。编译完成以后,生成的可执行文件位于```\TinyIM\Code\Client\ClientUI\Bin\Debug\```目录下,*TinyIM.exe*即为可执行文件。


## 4.其他
