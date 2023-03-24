Windows移植平台说明
===================

.. _windowslibusbwin32:

windows_libusb_win32
--------------------

windows平台下USB H2接口实现。USB使用libusb驱动。

USB设备使用
~~~~~~~~~~~

为了能操作这些USB蓝牙dongle，默认使用的驱动是蓝牙的驱动。所以需要更改设备所使用的驱动。

-  Step1：下载\ `Zadig <https://zadig.akeo.ie/>`__\ 。

-  Step2：菜单栏点击Options -> List All Devices。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221125133827682.png
   :alt: 

-  Step3：通过下拉选中当前连接的蓝牙dongle，更换设备driver为\ ``libusb-win32``\ ，如下图所示，过一会就换好驱动了。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221125133953130.png
   :alt: 

.. _安装软件环境-1:

安装软件环境
~~~~~~~~~~~~

参考\ `快速开始 — Zephyr_polling项目文档
(zephyr-polling.readthedocs.io) <https://zephyr-polling.readthedocs.io/en/latest/develop/getting_started/index.html>`__\ 安装Windows环境。

安装GCC环境，\ `Win7下msys64安装mingw工具链 - Milton - 博客园
(cnblogs.com) <https://www.cnblogs.com/milton/p/11808091.html>`__\ 。

安装Python环境，\ `Python安装教程-史上最全壬杰的博客-CSDN博客python安装教程 <https://blog.csdn.net/weixin_49237144/article/details/122915089>`__\ 。

安装Python依赖，运行\ ``python_require_env.py``\ 。

.. _编译运行-1:

编译运行
~~~~~~~~

本项目都由makefile组织编译，命令行运行\ ``make all``\ 指定\ ``PORT``\ 为\ ``windows_libusb_win32``\ 即可，最终会生成\ ``output/main.exe``\ 。而后直接运行即可。

.. code:: bash

   PS D:\worksplace\github\zephyr_polling> make all PORT=windows_libusb_win32 CHIPSET=csr8510 APP=beacon
   Current Configuration: APP=beacon PORT=windows_libusb_win32 CHIPSET=csr8510
   Linking    : "main.exe"
   Start Build Image.
   objcopy -v -O binary output/main.exe output/main.bin
   copy from `output/main.exe' [pei-i386] to `output/main.bin' [binary]
   objdump --source --all-headers --demangle --line-numbers --wide output/main.exe > output/main.lst
   Print Size
      text    data     bss     dec     hex filename
    129080    7016    2676  138772   21e14 output/main.exe
   PS D:\worksplace\github\zephyr_polling> .\output\main.exe

.. _windowsserial:

windows_serial
--------------

windows UART H4接口实现。PC上有串口就行，注意观察串口号。

.. _安装软件环境-2:

安装软件环境
~~~~~~~~~~~~

参考\ `快速开始 — Zephyr_polling项目文档
(zephyr-polling.readthedocs.io) <https://zephyr-polling.readthedocs.io/en/latest/develop/getting_started/index.html>`__\ 安装Windows环境。

安装GCC环境，\ `Win7下msys64安装mingw工具链 - Milton - 博客园
(cnblogs.com) <https://www.cnblogs.com/milton/p/11808091.html>`__\ 。

安装Python环境，\ `Python安装教程-史上最全壬杰的博客-CSDN博客python安装教程 <https://blog.csdn.net/weixin_49237144/article/details/122915089>`__\ 。

安装Python依赖，运行\ ``python_require_env.py``\ 。

.. _编译运行-2:

编译运行
~~~~~~~~

本项目都由makefile组织编译，命令行运行\ ``make all``\ 指定\ ``PORT``\ 为\ ``windows_serial``\ 即可，最终会生成\ ``output/main.exe``\ 。由于不同PC的串口号不同，需要在执行时传入COM口号，如\ ``COM1``\ 需要传入\ ``1``\ 。

.. code:: bash

   PS D:\worksplace\github\zephyr_polling> make all PORT=windows_serial CHIPSET=csr8510 APP=beacon      
   Current Configuration: APP=beacon PORT=windows_serial CHIPSET=csr8510
   Compiling  : "porting/windows_serial/main.c"
   Linking    : "main.exe"
   Start Build Image.
   objcopy -v -O binary output/main.exe output/main.bin
   copy from `output/main.exe' [pei-i386] to `output/main.bin' [binary]
   objdump --source --all-headers --demangle --line-numbers --wide output/main.exe > output/main.lst
   Print Size
      text    data     bss     dec     hex filename
    129388    7016    2676  139080   21f48 output/main.exe
   PS D:\worksplace\github\zephyr_polling> .\output\main.exe 1
