简单线程池的实现
==========================
[线程池技术实现简要分析](http://blog.csdn.net/chengonghao/article/category/6289128 "CSDN")

[关于作者](https://github.com/Apulus/cghSTL/blob/master/profile.md)

##功能
    线程池内部有两个线程（可配置），依次有序的在屏幕上打印执行信息
    
##编译程序
    本程序在 Ubuntu 15.10 下开发，所有的编译工具来源于软件仓库。
    我用 autotools 编译的程序，第一次编译的过程很繁琐，需要经历以下步骤：
    1.安装 gcc：$ sudo apt-get install gcc
    2.安装 autoconf：$ sudo apt-get install autoconf
    3.安装 libtool：$ sudo apt-get install libtool
    4.安装 make ：$ sudo apt-get install make
    5.在代码根目录依次执行：
        $ autoscan
        $ mv configure.scan configure.ac
        $ vi configure.ac  # 添加项目名、作者等信息
        $ aclocal
        $ autoconf
        $ autoheader
        $ vi Makefile.am    # 创建 Makefile.am 文件，我上传了写好的 Makefile.am
        $ automake --add-missing
        $ ./configure
        $ make
    我上传了一个编译脚本：build.sh，之后的每次编译只需执行该脚本，便能自动进行。
