rtthread移植平台说明
====================

.. _rtthreaduart:

rtthread_uart
-------------

rtthread平台下通用uart接口实现。

Ubuntu平台下利用qemu实现蓝牙功能
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

搭建 RT-Thread QEMU 环境
^^^^^^^^^^^^^^^^^^^^^^^^

参考 `文档：在 Ubuntu 平台开发
RT-Thread <https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/application-note/setup/qemu/ubuntu/an0005-qemu-ubuntu.md>`__
完成 RT-Thread qemu
环境的搭建。目前我们使用\ **qemu-vexpress-a9**\ 的bsp环境。

配置 NimBLE 软件包
''''''''''''''''''

1、在 **qemu-vexpress-a9** BSP 根目录输入：

.. code:: bash

   scons --menuconfig

2、进入 RT-Thread online packages → IoT - internet of things
目录即可看到 zephyr_polling软件包，勾选软件包。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324162232170.png
   :alt: 

3、配置
zephyr_polling项目的参数，如下述配置chipset选common，example选beacon，platform选rtthread_uart。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324162304667.png
   :alt: 

4、选择HCI接口参数

这里 The uart for HCI Transport 默认输入 “\ **uart1**\ ” 就好

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324162430691.png
   :alt: 

保存后退出。

下载软件包
''''''''''

使用 scons --menuconfig 命令后会安装及初始化 Env 工具，并在 home
目录下面生成 “.env” 文件夹，此文件夹为隐藏文件夹，切换到 home 目录，使用
ls 命令可查看所有目录和文件。

.. code:: bash

   $ ls ~/.env
   env.sh  local_pkgs  packages  tools

运行 **env.sh** 会配置好环境变量，让我们可以使用 **pkgs**
命令来更新软件包，执行

.. code:: bash

   $ source ~/.env/env.sh

使用 **pkgs --update** 命令下载 NimBLE 软件包到 BSP 目录下的 packages
文件夹里。

.. code:: bash

   $ pkgs --update

btproxy方式
^^^^^^^^^^^

本节是参考：\ `QEMU BLE
(rt-thread.org) <https://www.rt-thread.org/document/site/#/rt-thread-version/rt-thread-standard/application-note/setup/qemu/qemu-ble/qemu_ble?id=qemu-环境运行-ble>`__\ 。

如果你手头有一个USB蓝牙dongle。只要Ubuntu能识别的都可以使用。

.. _环境介绍-1:

环境介绍
''''''''

本次搭建环境所使用的资源和版本为：

-  Ubuntu 18.04

-  Bluez - v5.55

-  ell - v0.35

-  qemu - v2.11.1

-  RT-Thread - v5.0.0

-  一个蓝牙适配器

.. _环境搭建-1:

环境搭建
''''''''

环境搭建主要分为两部分，

1. bluez 的编译

2. Ubuntu里 RT-Thread QEMU 环境的搭建

首先需要更新源：

.. code:: bash

   sudo apt update
   sudo apt upgrade

安装所需要的依赖：

.. code:: bash

   sudo apt install git automake libtool libelf-dev elfutils libdw-dev libjson-c-dev libical-dev libreadline-dev libglib2.0-dev libdbus-1-dev libudev-dev libncurses5-dev python3 python3-pip qemu

bluez 编译安装
''''''''''''''

bluez 里有许多实用的工具，例如 ``btproxy``\ ，但是 Ubuntu 自带的 bluez
并未将这些工具都编译出来，因此需要我们重新编译 bluez。

在相同目录下克隆获取 0.35 版本的 ell，5.55 版本的 bluez 。

.. code:: bash

   git clone -b 0.35 git://git.kernel.org/pub/scm/libs/ell/ell.git
   git clone -b 5.55 https://github.com/bluez/bluez.git

最终 ell 和 bluez 应该在同一个目录级别下，目录结构为：

.. code:: bash

   .
       |--- ell
       |    |--- ell
       |    |--- unit
       |--- bluez
       |    |--- src
       |    |--- tools

然后输入下述命令编译 bluez：

.. code:: bash

   cd bluez
   ./bootstrap-configure --disable-android --disable-midi --disable-mesh
   make

注意，这个时候会报错，但是不用管，tools目录下已经有所需的\ ``btproxy``\ 。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324161225919.png
   :alt: 

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324161324577.png
   :alt: 

QEMU 运行 BLE
'''''''''''''

qemu 运行 BLE 时，本质上还是在和物理机的蓝牙设备通信，下面介绍如何在
qemu 环境中使用蓝牙设备。

首先输入 ``hciconfig`` 查看本机的蓝牙设备，

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324161621296.png
   :alt: 

我这里是 hci0 蓝牙设备，需要保证上述红框框住的部分为
``DOWN``\ ，也就是该蓝牙设备未打开，否则需要输入下述命令关闭该蓝牙设备：

.. code:: bash

   sudo hciconfig hci0 down

然后进入 bluez 的 tools
目录，输入命令为该蓝牙设备创建一个代理（proxy）。

.. code:: bash

   sudo ./btproxy -u -i 0

其中 ``-u`` 代表使用 Unix Server，默认路径为
``/tmp/bt-server-bredr``\ ，\ ``-i 0`` 指使用 hci0 蓝牙设备，如果是 hci1
蓝牙设备，则需更改为 ``-i 1`` 。运行成功后会有下述显示：

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324161708541.png
   :alt: 

然后在 RT-Thread 的 bsp/qemu-vexpress-a9 目录下编译运行：

.. code:: bash

   scons
   qemu-system-arm -M vexpress-a9 -kernel rtthread.bin -nographic -serial mon:stdio -serial unix:/tmp/bt-server-bredr

即可成功在 qemu 环境里与蓝牙卡片交互，下面稍微介绍下原理和注意事项。

qemu 命令参数里与串口相关的有
``-serial mon:stdio -serial unix:/tmp/bt-server-bredr``\ ，\ ``-serial``
参数的介绍为：

   -serial dev redirect the serial port to char device 'dev'

这个参数将 qemu 程序的串口重定向到物理机的设备文件，

第一个 ``-serial mon:stdio`` 将 qemu 里的 uart0 重定向到了物理机的 stdio
设备文件，而且在 qemu 程序里 uart0 用作 msh 命令行工具，类似
stdio，因此我们可以在物理机上看到 qemu 程序的打印数据，也可以输入命令。

第二个 ``-serial unix:/tmp/bt-server-bredr`` 是将 qemu 里的 uart1
重定向到物理机的 /tmp/bt-server-bredr 文件，而这个文件又是 hci0
设备的代理，因此从 qemu 程序的角度来看，就是使用 H4 协议，通过 uart1
去访问蓝牙设备。uart1 的波特率使用 115200
即可（未找到说明，但一般都是这样使用）。

**注意：上述两个 -serial 的顺序不能颠倒，否则 qemu 程序无法正常运行。**

若成功运行，则在 btproxy 界面会有如下显示：

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324161832389.png
   :alt: 

若蓝牙设备未关闭，则会显示设备或资源忙：

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324162718229.png
   :alt: 

建议每次运行前都关闭一次蓝牙设备：\ ``sudo hciconfig hci0 down`` 。

.. _测试运行-1:

测试运行
''''''''

而后输入\ **zephyr_polling_init**\ 既可以运行demo工程了。可以看到蓝牙地址是\ **11:22:33:44:55:66**\ 。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324162928956.png
   :alt: 

用nordic的app搜索，就可以看到发送的beacon数据包了。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/ca9bcd0d5bf31ddaed5fcc9ef7f9053.jpg
   :alt: 

串口方式
^^^^^^^^

本节是参考：\ `RT-Thread-QEMU下运行NimBLERT-Thread问答社区 -
RT-Thread <https://club.rt-thread.org/ask/article/47e1aad061e7a53c.html>`__\ 。

如果你手头就是一个uart设备，系统中还没驱动的情况下。

.. _环境介绍-2:

环境介绍
''''''''

本次搭建环境所使用的资源和版本为：

-  Ubuntu 18.04

-  qemu - v2.11.1

-  RT-Thread - v5.0.0

-  一个蓝牙UART模组

.. _环境搭建-2:

环境搭建
''''''''

连接蓝牙 Control 芯片（使用 pts_dongle 进行演示，将其直接连接电脑）。

需要注意，下面的\ **/dev/ttyACM0**\ 需要根据具体情况调整，看你实际的串口是哪个。Linux串口是默认普通用户没有读写权限的，每次开机后对串口读写都要附加权限。

.. code:: bash

   sudo chmod 666 /dev/ttyACM0
   qemu-system-arm -M vexpress-a9 -smp cpus=2 -kernel rtthread.bin -nographic -sd sd.bin -serial mon:stdio -serial /dev/ttyACM0

.. _测试运行-2:

测试运行
''''''''

而后输入\ **zephyr_polling_init**\ 既可以运行demo工程了。可以看到蓝牙地址是\ **c0:07:e8:e1:2d:e8**\ 。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/1679647130430.png
   :alt: 

用nordic的app搜索，就可以看到发送的beacon数据包了。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/98692c1fb1c5df8fc5bb681531b37d7.jpg
   :alt: 

.. _rtthreadartpi:

rtthread_artpi
--------------

rtthread在ArtPi硬件平台下和AP6212蓝牙实现。

RT-Thread Studio下利用ArtPi实现蓝牙功能
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

搭建 RT-Thread Studio 环境
^^^^^^^^^^^^^^^^^^^^^^^^^^

参考完成\ `快速开始
(rt-thread.org) <https://www.rt-thread.org/document/site/#/development-tools/rtthread-studio/um/studio-user-begin>`__
RT-Thread studio 环境的搭建。

本文是参考： `RT-Thread-如何在 ART-Pi 的 Studio 工程中使用 NimBLE
蓝牙协议栈RT-Thread问答社区 -
RT-Thread <https://club.rt-thread.org/ask/article/ed1e170fb2a30f0a.html>`__\ 来实现的。

新建 ART-Pi 示例工程
^^^^^^^^^^^^^^^^^^^^

按照下图新建一个 art_pi_blink_led 示例工程，等待创建完成。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324165633967.png
   :alt: 

.. _配置使用-zephyrpolling:

配置使用 zephyr_polling
^^^^^^^^^^^^^^^^^^^^^^^

进入工程 RT-Thread Settings 界面，
进入到软件包页面，找到zephyr_polling，并按照下面的参数配置好。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324165839041.png
   :alt: 

配置完成后保存，studio 将自动更新下载软件包。

配置相关串口
^^^^^^^^^^^^

在 RT-Thread Settings
下硬件选项页中使能对应串口，如下图，按照实际需求开启。AP6212内部接的是UART3，所以UART3必须打开。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324170135031.png
   :alt: 

串口接收buffer记得调整一下。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324170041183.png
   :alt: 

保存退出。

在 borad.h 头文件中添加对应串口的引脚定义。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324170356709.png
   :alt: 

编译运行
^^^^^^^^

直接烧录进去即可，这里直接使用 ART-Pi 板载的 AP6216
芯片。而后输入\ **zephyr_polling_init**\ 指令，启动beacon程序。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324170549579.png
   :alt: 

使用 nRF Connect 手机 APP
即可成功观察到蓝牙设备地址为\ **70:4a:0e:53:16:bf**\ 的beacon设备。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/ea1a5a883262416ad58fb6ad9c5a5cb.jpg
   :alt: 
