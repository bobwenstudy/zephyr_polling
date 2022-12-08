应用说明
========

简介
----

本项目的编译环境是基于make来编译的。

蓝牙本身有各种各样的场景的业务，不同业务场景的功能需求各不相同，为了满足不同场景的需要，并确保资源有限的设备在特殊场景下使用足够少的Code
Size和RAM Size，项目提供了丰富的配置参数，来实现相关功能代码的开关。

为了方便用户理解本项目，编译是以\ **example**\ 中的例程为中心的，每个例程都有不同的配置参数和业务代码。通过学习这些例程，便于用户快速掌握如何使用本项目的API和Kconfig配置系统。

本文具体对\ **应用**\ 设计和实现进行说明。

本项目所有的\ **应用**\ 都在\ **example**\ 目录中，通常的目录结构如下：

.. code:: bash

   <home>/example/xxx
    ├── prj.conf（应用层Kconfig配置）
    ├── build.mk（编译系统所需）
    └── app_main.c（程序代码）

**prj.conf**\ ：Kconfig持久化设计，关于Kconfig的设计可以看 :ref:`kconfig`，每个应用可以根据需要单独配置，修改这个会最终影响生成的\ **autoconfig.h**\ ，也就是蓝牙协议栈编译出来的Code和RAM。

**build.mk**\ ：根目录的makefile会引用这个文件，这里需要配置\ **应用**\ 所需编译的文件和头文件路径。

**app_main.c**\ ：应用层代码，文件名并没有特别指定，但是需要在build.mk中引用。

新增应用
--------

按照下面的流程可以创建一个你自己的\ **应用**\ 。

创建应用目录
~~~~~~~~~~~~

到example目录下，创建一个自己感兴趣的目录即可，可以通过GUI创建当然也可以通过命令行创建。

如：创建一个名为\ **app_test**\ 的工程。后续已这个例程为例进行介绍。

.. code:: bash

   mkdir app_test

.. _创建appmainc:

创建app_main.c
~~~~~~~~~~~~~~

到\ **app_test**\ 目录下，创建\ ``app_main.c``\ 即可，由于本项目主要实现蓝牙功能，所以所有例程都会开启蓝牙功能，项目的入口函数也不再是\ ``main``\ 函数，而是\ ``bt_ready``\ 函数。

**bt_ready**\ ：是蓝牙开启结束的回调函数，在porting中的\ ``main.c``\ 调用。

**app_polling_work**\ ：是平台提供给应用层的轮询函数，注意这里不能写\ ``while(1)``\ ，只能写一些轮询业务，在porting中的\ ``main.c``\ 中有个\ ``while(1)``\ 函数周期调用。

**printk**\ ：是平台提供的日志打印函数，所有的日志都需要使用本接口，不能使用\ ``printf``\ ，不然一些日志记录之类的功能可能不好用。

.. code:: c

   #include <logging/bt_log_impl.h>

   void bt_ready(int err)
   {
       if (err)
       {
           printk("Bluetooth init failed (err %d)\n", err);
           return;
       }

       printk("Bluetooth initialized\n");
   }

   void app_polling_work(void)
   {
       return;
   }

.. _创建buildmk:

创建build.mk
~~~~~~~~~~~~

这个比较简单，就是配置\ ``SRC``\ 代码路径、\ ``INCLUDE``\ 头文件路径以及\ ``LIB``\ 库路径（应用层一般不建议使用，由于平台异构，lib使用会比较麻烦。）

需要注意的是\ ``APP_PATH``\ ，这个是编译系统提供的当前应用的路径，也就是\ ``example/app_test``\ ，新加的路径必须加这个根路径参数。

.. code:: makefile

   # define source directory
   SRC		+= $(APP_PATH)

   # define include directory
   INCLUDE	+= $(APP_PATH)

   # define lib directory
   LIB		+=

.. _创建prjconf:

创建prj.conf
~~~~~~~~~~~~

这个要理解需要去看懂 :ref:`kconfig`，默认情况下我们只需要开启日志功能和蓝牙功能即可，其他的可以通过\ ``make menuconfig``\ ，修改配置后，用\ ``D``\ 保存差异，生成在根目录的\ ``defconfig``\ 文件就是所需的\ ``prj.conf``\ 。

.. code:: 

   CONFIG_BT=y
   CONFIG_BT_DEBUG_LOG=y

编译应用
~~~~~~~~

上述操作完成后，目录结构如下：

.. code:: 

   <home>/example/app_test
    ├── prj.conf
    ├── build.mk
    └── app_main.c

这时可以通过调整make传入的APP参数来使用本应用程序，键入该指令就可以完成对APP的编译。

.. code:: bash

   make all APP=app_test

运行应用
~~~~~~~~

生成的执行文件在output目录下，直接执行即可，Windows下的执行效果如下：

可以看到启动蓝牙后，就什么都没干了。

.. code:: 

   PS D:\worksplace\github\zephyr_polling> .\output\main.exe
   [2022-12-07 11:06:12.194] [0xb8bc] display_devices(), idVendor: 0xbda, idProduct: 0x8771
   [2022-12-07 11:06:12.196] [0xb8bc] display_devices(), idVendor: 0x10d7, idProduct: 0xb012
   [2022-12-07 11:06:12.198] [0xb8bc] display_devices(), idVendor: 0xa12, idProduct: 0x1
   [2022-12-07 11:06:12.200] [0xb8bc] success: set configuration #1
   [2022-12-07 11:06:12.202] [0xb8bc] success: claim_interface #0
   [2022-12-07 11:06:12.205] [0x77dc] tx_process_loop
   [2022-12-07 11:06:12.205] [0xca8c] rx_evt_process_loop
   [2022-12-07 11:06:12.205] [0xb8bc] hci_driver_open()
   [2022-12-07 11:06:12.209] [0xb8bc] I: (bt_hci_core)hci_init():3220: work start.
   [2022-12-07 11:06:12.211] [0xb8bc] CMD =>  00 FC 13 C2 02 00 09 00 02 00 03 70 00 00 F2 00 01 00 08 00 01 00
   [2022-12-07 11:06:13.241] [0xb8bc] EVT <=  FF 13 C2 01 00 09 00 02 00 03 70 00 00 F2 00 01 00 08 00 01 00
   [2022-12-07 11:06:13.247] [0xb8bc] CMD =>  00 FC 19 C2 02 00 0C 00 08 00 03 70 00 00 01 00 04 00 08 00 44 00 66 55 33 00 22 11
   [2022-12-07 11:06:13.254] [0xb8bc] EVT <=  FF 19 C2 01 00 0C 00 08 00 03 70 00 00 01 00 04 00 08 00 44 00 66 55 33 00 22 11
   [2022-12-07 11:06:13.258] [0xb8bc] CMD =>  00 FC 13 C2 02 00 09 00 09 00 02 40 00 00 00 00 00 00 00 00 00 00
   [2022-12-07 11:06:13.258] [0xc71c] reset_driver_process, wait usb reboot.
   [2022-12-07 11:06:13.264] [0x77dc] error tx:
   libusb0-dll:err [control_msg] sending control message failed, win error: 连到系统上的设备没有发挥作用。


   [2022-12-07 11:06:18.270] [0xc71c] reset_driver_process, usb reboot ready.
   [2022-12-07 11:06:18.272] [0xca8c] error reading:
   libusb0-dll:err [submit_async] submitting request failed, win error: 设备不识别此命令。

   [2022-12-07 11:06:18.381] [0xca8c] rx_evt_process_loop end
   [2022-12-07 11:06:18.384] [0xc71c] display_devices(), idVendor: 0xa12, idProduct: 0x1
   [2022-12-07 11:06:18.386] [0xc71c] display_devices(), idVendor: 0xbda, idProduct: 0x8771
   [2022-12-07 11:06:18.388] [0xc71c] display_devices(), idVendor: 0x10d7, idProduct: 0xb012
   [2022-12-07 11:06:18.390] [0xc71c] success: set configuration #1
   [2022-12-07 11:06:18.392] [0xc71c] success: claim_interface #0
   [2022-12-07 11:06:18.394] [0xb8bc] CMD =>  03 0C 00
   [2022-12-07 11:06:18.394] [0x130c] tx_process_loop
   [2022-12-07 11:06:18.394] [0xbddc] rx_evt_process_loop
   [2022-12-07 11:06:19.559] [0xb8bc] CMD =>  02 10 00
   [2022-12-07 11:06:19.568] [0xb8bc] EVT <=  0E 44 01 02 10 00 FF FF FF 03 FE FF FF FF FF FF FF FF F3 0F E8 FE 3F F7 83 FF 1C 00 00 00 61 F7 FF FF 7F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
   [2022-12-07 11:06:19.578] [0xb8bc] CMD =>  03 20 00
   [2022-12-07 11:06:19.581] [0xb8bc] EVT <=  0E 0C 01 03 20 00 01 00 00 00 00 00 00 00
   [2022-12-07 11:06:19.583] [0xb8bc] CMD =>  6D 0C 02 01 00
   [2022-12-07 11:06:19.587] [0xb8bc] EVT <=  0E 04 01 6D 0C 00[2022-12-07 11:06:19.590] [0xb8bc] CMD =>  01 20 08 02 00 00 00 00 00 00 00
   [2022-12-07 11:06:19.595] [0xb8bc] EVT <=  0E 04 01 01 20 00
   [2022-12-07 11:06:19.596] [0xb8bc] CMD =>  01 0C 08 00 80 00 02 00 00 00 20
   [2022-12-07 11:06:19.600] [0xb8bc] EVT <=  0E 04 01 01 0C 00
   [2022-12-07 11:06:19.602] [0xb8bc] CMD =>  09 10 00
   [2022-12-07 11:06:19.608] [0xb8bc] EVT <=  0E 0A 01 09 10 00 66 55 44 33 22 11
   [2022-12-07 11:06:19.610] [0xb8bc] I: (bt_hci_core)hci_init_end():3195: work end.
   [2022-12-07 11:06:19.612] [0xb8bc] I: (bt_hci_core)bt_dev_show_info():2998: Identity: 11:22:33:44:55:66 (public)
   [2022-12-07 11:06:19.614] [0xb8bc] I: (bt_hci_core)bt_dev_show_info():3030: HCI: version 4.0 (0x06) revision 0x22bb, manufacturer 0x000a
   [2022-12-07 11:06:19.617] [0xb8bc] I: (bt_hci_core)bt_dev_show_info():3033: LMP: version 4.0 (0x06) subver 0x22bb
   [2022-12-07 11:06:19.623] [0xb8bc] Bluetooth initialized

应用调试
--------

编译系统除了生成执行文件外，还会生成反编译文件以及map文件，以便用户对代码进行分析。

生成的目录结构如下所示。

.. code:: 

   <home>/output
    ├── log
    │   ├── log.cfa
    │   └── log.txt
    ├── main.bin
    ├── main.exe
    ├── main.lst
    └── main.map

GDB调试
~~~~~~~

编译的工程都开启了\ **-g**\ 选项，所以可以直接通过\ **GDB**\ 进行调试，也可以自己根据具体需要使用VSCODE或者其他IDE来用GDB调试。

日志调试
~~~~~~~~

在交互终端中，可以会打印核心的日志信息。

此外除了实时打印外，项目还会在\ ``output/log``\ 目录下保存日志文件。其中终端显示的交互日志保存为\ **log.txt**\ ；交互的hci数据包会保存为\ ``btsnoop``\ 格式的文件\ **log.cfa**\ 。

默认打印的日志只是开启了INFO级别以上的日志，如果需要看所有日志，可以在\ ``prj.conf``\ 中加入，初期学习的时候可以通过开启\ ``CONFIG_BT_LOG_LEVEL_DBG=y``\ 来显示所有日志来学习代码。
