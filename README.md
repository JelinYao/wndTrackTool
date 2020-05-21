# wndTrackTool
Windows上捕获窗口进程工具，获取进程加载模块列表

# 用法
代码使用VS2010环境开发，基于微软WTL框架。
使用VS编译后运行即可，如果需要捕获64位程序，得编译64位版本程序再使用。

# 主要用途
(1)查看Windows客户端嵌入的IE控件加载的url链接
最开始主要是这个原因才开发的，因为当时很多客户端内嵌的都是IE浏览器控件。现在基本上都是用的libcef了，所以这个功能没啥用了。

(2)查看Windows上程序运行时候记载的DLL
主要是为了方便远程注入DLL以及发现自己的程序被哪些其他DLL注入了，针对性的进行攻防处理。（API hook或者是驱动层拦截）

# 工具截图

(1)捕获VS2017加载的模块dll列表

![](https://raw.githubusercontent.com/JelinYao/wndTrackTool/master/img/shot1.png)

(2)捕获视频播放器加载的模块dll列表

![](https://raw.githubusercontent.com/JelinYao/wndTrackTool/master/img/shot2.png)
