快速开始
========

简介
----

通过本文可以实现：

-  配置基于Windows/Ubuntu系统的命令行开发环境。

-  获取源代码

-  运行Zephyr_polling项目

安装软件环境
------------

为了方便大家移植，本项目都是用makefile来组织编译的，一些脚本通过python来实现，所以系统主要安装好make环境和python环境即可运行本项目。

GCC环境
~~~~~~~

参考这个文章安装即可。\ `Win7下msys64安装mingw工具链 - Milton - 博客园
(cnblogs.com) <https://www.cnblogs.com/milton/p/11808091.html>`__

安装完以后，键入\ ``gcc -v``\ ，得到如下提示说明环境安装好了，目前笔者用的是mingw32的gcc。

.. figure:: https://img-blog.csdnimg.cn/img_convert/e0f445cdb5fc3174219e763cfdc71194.png
   :alt: 

`Python环境 <https://www.python.org/>`__
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

脚本/Kconfig都是python运行环境，为了方便，本项目是用python3环境。

网上有很多配置python环境教程了，\ `Python安装教程-史上最全\ 壬杰的博客-CSDN博客\ python安装教程 <https://blog.csdn.net/weixin_49237144/article/details/122915089>`__\ 安装可以参考这个，当然也可以直接到\ `Welcome
to Python.org <https://www.python.org/>`__\ 下载。

最终安装结束后，键入\ ``python -V``\ ，能看到python版本，代表环境安装好了。

.. figure:: https://img-blog.csdnimg.cn/img_convert/02f37e912230aa160c7310234e82579e.png
   :alt: 

Python依赖环境
~~~~~~~~~~~~~~

Python运行过程中，需要安装一些module，所需的环境都在\ ``python_require_env.py``\ 配置好了，只要运行下该脚本，即可安装好python依赖环境。

.. code:: bash


   D:\worksplace\github\zephyr_polling>python python_require_env.py
   Requirement already satisfied: pyelftools~=0.29 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from -r scripts/footprint/requirements.txt (line 1)) (0.29)
   Requirement already satisfied: anytree~=2.8.0 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from -r scripts/footprint/requirements.txt (line 2)) (2.8.0)
   Requirement already satisfied: six>=1.9.0 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from anytree~=2.8.0->-r scripts/footprint/requirements.txt (line 2)) (1.16.0)

   [notice] A new release of pip available: 22.2.2 -> 22.3.1
   [notice] To update, run: python.exe -m pip install --upgrade pip
   Requirement already satisfied: kconfiglib~=14.1.0 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from -r scripts/kconfig/requirements.txt (line 1)) (14.1.0)
   Requirement already satisfied: windows-curses~=2.3.0 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from -r scripts/kconfig/requirements.txt (line 2)) (2.3.0)

   [notice] A new release of pip available: 22.2.2 -> 22.3.1
   [notice] To update, run: python.exe -m pip install --upgrade pip
   Requirement already satisfied: Sphinx==5.1.1 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from -r doc/requirements.txt (line 1)) (5.1.1)
   Requirement already satisfied: recommonmark==0.7.1 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from -r doc/requirements.txt (line 2)) (0.7.1)
   Requirement already satisfied: sphinx-markdown-tables==0.0.17 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from -r doc/requirements.txt (line 3)) (0.0.17)
   Requirement already satisfied: sphinx-rtd-theme==1.0.0 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from -r doc/requirements.txt (line 4)) (1.0.0)
   Requirement already satisfied: sphinxcontrib-jsmath in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (1.0.1)
   Requirement already satisfied: Pygments>=2.0 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (2.12.0)
   Requirement already satisfied: colorama>=0.3.5 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (0.4.5)
   Requirement already satisfied: requests>=2.5.0 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (2.28.1)
   Requirement already satisfied: imagesize in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (1.4.1)
   Requirement already satisfied: sphinxcontrib-applehelp in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (1.0.2)
   Requirement already satisfied: alabaster<0.8,>=0.7 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (0.7.12)
   Requirement already satisfied: sphinxcontrib-htmlhelp>=2.0.0 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (2.0.0)
   Requirement already satisfied: sphinxcontrib-qthelp in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (1.0.3)
   Requirement already satisfied: Jinja2>=2.3 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (3.1.2)
   Requirement already satisfied: babel>=1.3 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (2.10.3)
   Requirement already satisfied: sphinxcontrib-devhelp in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (1.0.2)
   Requirement already satisfied: snowballstemmer>=1.1 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (2.2.0)
   Requirement already satisfied: packaging in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (21.3)
   Requirement already satisfied: docutils<0.20,>=0.14 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (0.17.1)
   Requirement already satisfied: sphinxcontrib-serializinghtml>=1.1.5 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (1.1.5)
   Requirement already satisfied: commonmark>=0.8.1 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from recommonmark==0.7.1->-r doc/requirements.txt (line 2)) (0.9.1)
   Requirement already satisfied: markdown>=3.4 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from sphinx-markdown-tables==0.0.17->-r doc/requirements.txt (line 3)) (3.4.1)
   Requirement already satisfied: pytz>=2015.7 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from babel>=1.3->Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (2022.1)
   Requirement already satisfied: MarkupSafe>=2.0 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from Jinja2>=2.3->Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (2.1.1)
   Requirement already satisfied: idna<4,>=2.5 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from requests>=2.5.0->Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (3.3)
   Requirement already satisfied: urllib3<1.27,>=1.21.1 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from requests>=2.5.0->Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (1.26.11)
   Requirement already satisfied: charset-normalizer<3,>=2 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from requests>=2.5.0->Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (2.1.0)
   Requirement already satisfied: certifi>=2017.4.17 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from requests>=2.5.0->Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (2022.6.15)
   Requirement already satisfied: pyparsing!=3.0.5,>=2.0.2 in c:\users\wenbo\appdata\local\programs\python\python310\lib\site-packages (from packaging->Sphinx==5.1.1->-r doc/requirements.txt (line 1)) (3.0.9)

   [notice] A new release of pip available: 22.2.2 -> 22.3.1
   [notice] To update, run: python.exe -m pip install --upgrade pip

   D:\worksplace\github\zephyr_polling>

下载源码
--------

项目的源码都在github上维护\ `bobwenstudy/zephyr_polling <https://github.com/bobwenstudy/zephyr_polling>`__\ ，直接clone即可。

.. code:: bash

   git clone https://github.com/bluekitchen/btstack.git

下载项目后的工程目录如下：

.. code:: bash

   zephyr_polling
    ├── chipset（支持芯片列表）
    ├── doc（文档目录）
    ├── example（应用例程）
    ├── platform（各个平台的支持）
    ├── porting（部署，包含platform和HCI driver配置）
    ├── scripts（脚本）
    ├── src（协议栈核心部分）
    ├── code_format.py（代码格式化，可以通过make code_format调用，当然也可以直接运行）
    ├── python_require_env.py（python依赖安装，直接运行即可）
    ├── LICENSE（LICENSE信息）
    ├── Makefile（项目的makefile）
    └── README.md（ReadMe说明）

编译工程
--------

本项目都由makefile组织编译，像很多项目一样，在命令行中键入\ ``make all``\ 即可。编译过程如下，在Windows环境下最终会生成\ ``output/main.exe``\ 。

.. code:: bash

   D:\worksplace\github\zephyr_polling>make all
   Current Configuration: APP=beacon PORT=windows_libusb_win32 CHIPSET=csr8510
   Using user config.
   已复制         1 个文件。
   python scripts/kconfig/kconfig.py --handwritten-input-configs src/Kconfig output/.config output/autoconfig.h output/autoconfig_log.txt example/beacon/prj.conf
   Parsing src/Kconfig
   Loaded configuration 'example/beacon/prj.conf'
   Configuration saved to 'output/.config'
   Kconfig header saved to 'output/autoconfig.h'
   .config updated
   python scripts/kconfig/kconfig.py src/Kconfig output/.config output/autoconfig.h output/autoconfig_log.txt output/.config
   Parsing src/Kconfig
   Loaded configuration 'output/.config'
   No change to configuration in 'output/.config'
   No change to Kconfig header in 'output/autoconfig.h'
   Compiling  : "src/common/aes_soft.c"
   Compiling  : "src/common/bt_buf.c"
   Compiling  : "src/common/bt_storage_kv.c"
   Compiling  : "src/common/hex.c"
   Compiling  : "src/common/net_buf.c"
   Compiling  : "src/common/rpa.c"
   Compiling  : "src/common/timeout.c"
   Compiling  : "src/common/timer.c"
   Compiling  : "src/common/work.c"
   Compiling  : "src/drivers/hci_h4.c"
   Compiling  : "src/host/addr.c"
   Compiling  : "src/host/adv.c"
   Compiling  : "src/host/att.c"
   Compiling  : "src/host/conn.c"
   Compiling  : "src/host/crypto.c"
   Compiling  : "src/host/ecc.c"
   Compiling  : "src/host/gatt.c"
   Compiling  : "src/host/hci_common.c"
   Compiling  : "src/host/hci_core.c"
   Compiling  : "src/host/id.c"
   Compiling  : "src/host/keys.c"
   Compiling  : "src/host/l2cap.c"
   Compiling  : "src/host/scan.c"
   Compiling  : "src/host/smp.c"
   Compiling  : "src/host/uuid.c"
   Compiling  : "src/logging/bt_log.c"
   Compiling  : "src/logging/bt_log_impl.c"
   Compiling  : "src/utils/k_queue.c"
   Compiling  : "src/utils/mem_slab.c"
   Compiling  : "src/utils/spool.c"
   Compiling  : "src/services/bas.c"
   Compiling  : "src/services/dis.c"
   Compiling  : "src/services/hrs.c"
   Compiling  : "src/services/tps.c"
   Compiling  : "example/beacon/app_main.c"
   Compiling  : "porting/windows_libusb_win32/main.c"
   Compiling  : "platform/windows/windows_bt_log_impl.c"
   Compiling  : "platform/windows/windows_bt_storage_kv_impl.c"
   Compiling  : "platform/windows/windows_bt_timer_impl.c"
   Compiling  : "platform/windows/windows_driver_libusb.c"
   Compiling  : "platform/windows/windows_driver_serial.c"
   Compiling  : "chipset/csr8510/chipset_csr8510.c"
   Linking    : "main.exe"
   Start Build Image.
   objcopy -v -O binary output/main.exe output/main.bin
   copy from `output/main.exe' [pei-i386] to `output/main.bin' [binary]
   objdump --source --all-headers --demangle --line-numbers --wide output/main.exe > output/main.lst
   Print Size
      text    data     bss     dec     hex filename
    129224    7016    2676  138916   21ea4 output/main.exe

运行
----

默认选的是\ `CSR8510 <https://detail.tmall.com/item.htm?abbucket=2&id=534662513906&ns=1&skuId=4910946697067&spm=a230r.1.14.1.2f6811a37qFFQU>`__\ 芯片，接口是USB接口，接好dongle后，直接运行即可。

运行过程如下。

.. code:: 

   D:\worksplace\github\zephyr_polling>.\output\main.exe
   [2022-12-06 16:31:16.488] [0xad38] display_devices(), idVendor: 0xbda, idProduct: 0x8771
   [2022-12-06 16:31:16.491] [0xad38] display_devices(), idVendor: 0x10d7, idProduct: 0xb012
   [2022-12-06 16:31:16.493] [0xad38] display_devices(), idVendor: 0xa12, idProduct: 0x1
   [2022-12-06 16:31:16.495] [0xad38] success: set configuration #1
   [2022-12-06 16:31:16.497] [0xad38] success: claim_interface #0
   [2022-12-06 16:31:16.498] [0xa2f0] tx_process_loop
   [2022-12-06 16:31:16.498] [0xa144] rx_evt_process_loop
   [2022-12-06 16:31:16.498] [0xad38] hci_driver_open()
   [2022-12-06 16:31:16.504] [0xad38] I: (bt_hci_core)hci_init():3220: work start.
   [2022-12-06 16:31:16.506] [0xad38] CMD =>  00 FC 13 C2 02 00 09 00 02 00 03 70 00 00 F2 00 01 00 08 00 01 00
   [2022-12-06 16:31:17.530] [0xad38] EVT <=  FF 13 C2 01 00 09 00 02 00 03 70 00 00 F2 00 01 00 08 00 01 00
   [2022-12-06 16:31:17.535] [0xad38] CMD =>  00 FC 19 C2 02 00 0C 00 08 00 03 70 00 00 01 00 04 00 08 00 44 00 66 55 33 00 22 11
   [2022-12-06 16:31:17.543] [0xad38] EVT <=  FF 19 C2 01 00 0C 00 08 00 03 70 00 00 01 00 04 00 08 00 44 00 66 55 33 00 22 11
   [2022-12-06 16:31:17.548] [0xad38] CMD =>  00 FC 13 C2 02 00 09 00 09 00 02 40 00 00 00 00 00 00 00 00 00 00
   [2022-12-06 16:31:17.548] [0xa3a0] reset_driver_process, wait usb reboot.
   [2022-12-06 16:31:17.553] [0xa2f0] error tx:
   libusb0-dll:err [control_msg] sending control message failed, win error: 连到系统上的设备没有发挥作用。


   [2022-12-06 16:31:22.555] [0xa3a0] reset_driver_process, usb reboot ready.
   [2022-12-06 16:31:22.557] [0xa144] error reading:
   libusb0-dll:err [submit_async] submitting request failed, win error: 设备不识别此命令。

   [2022-12-06 16:31:22.666] [0xa144] rx_evt_process_loop end
   [2022-12-06 16:31:22.668] [0xa3a0] display_devices(), idVendor: 0xa12, idProduct: 0x1
   [2022-12-06 16:31:22.670] [0xa3a0] display_devices(), idVendor: 0xbda, idProduct: 0x8771
   [2022-12-06 16:31:22.674] [0xa3a0] display_devices(), idVendor: 0x10d7, idProduct: 0xb012
   [2022-12-06 16:31:22.677] [0xa3a0] success: set configuration #1
   [2022-12-06 16:31:22.679] [0xa3a0] success: claim_interface #0
   [2022-12-06 16:31:22.680] [0xad38] CMD =>  03 0C 00
   [2022-12-06 16:31:22.680] [0xb538] tx_process_loop
   [2022-12-06 16:31:22.680] [0xabc4] rx_evt_process_loop
   [2022-12-06 16:31:23.822] [0xad38] EVT <=  0E 04 01 03 0C 00
   [2022-12-06 16:31:23.825] [0xad38] CMD =>  03 10 00
   [2022-12-06 16:31:23.829] [0xad38] EVT <=  0E 0C 01 03 10 00 FF FF 8F FE DB FF 5B 87
   [2022-12-06 16:31:23.832] [0xad38] CMD =>  01 10 00
   [2022-12-06 16:31:23.835] [0xad38] EVT <=  0E 0C 01 01 10 00 06 BB 22 06 0A 00 BB 22
   [2022-12-06 16:31:23.838] [0xad38] CMD =>  02 10 00
   [2022-12-06 16:31:23.846] [0xad38] EVT <=  0E 44 01 02 10 00 FF FF FF 03 FE FF FF FF FF FF FF FF F3 0F E8 FE 3F F7 83 FF 1C 00 00 00 61 F7 FF FF 7F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
   [2022-12-06 16:31:23.855] [0xad38] CMD =>  03 20 00
   [2022-12-06 16:31:23.859] [0xad38] EVT <=  0E 0C 01 03 20 00 01 00 00 00 00 00 00 00
   [2022-12-06 16:31:23.861] [0xad38] CMD =>  6D 0C 02 01 00
   [2022-12-06 16:31:23.865] [0xad38] EVT <=  0E 04 01 6D 0C 00
   [2022-12-06 16:31:23.866] [0xad38] CMD =>  01 20 08 02 00 00 00 00 00 00 00
   [2022-12-06 16:31:23.873] [0xad38] EVT <=  0E 04 01 01 20 00
   [2022-12-06 16:31:23.875] [0xad38] CMD =>  01 0C 08 00 80 00 02 00 00 00 20
   [2022-12-06 16:31:23.879] [0xad38] EVT <=  0E 04 01 01 0C 00
   [2022-12-06 16:31:23.880] [0xad38] CMD =>  09 10 00
   [2022-12-06 16:31:23.884] [0xad38] EVT <=  0E 0A 01 09 10 00 66 55 44 33 22 11
   [2022-12-06 16:31:23.886] [0xad38] I: (bt_hci_core)hci_init_end():3195: work end.
   [2022-12-06 16:31:23.889] [0xad38] I: (bt_hci_core)bt_dev_show_info():2998: Identity: 11:22:33:44:55:66 (public)
   [2022-12-06 16:31:23.892] [0xad38] I: (bt_hci_core)bt_dev_show_info():3030: HCI: version 4.0 (0x06) revision 0x22bb, manufacturer 0x000a
   [2022-12-06 16:31:23.895] [0xad38] I: (bt_hci_core)bt_dev_show_info():3033: LMP: version 4.0 (0x06) subver 0x22bb
   [2022-12-06 16:31:23.898] [0xad38] Bluetooth initialized
   [2022-12-06 16:31:23.899] [0xad38] Beacon started, advertising as 11:22:33:44:55:66 (public)
   [2022-12-06 16:31:23.904] [0xad38] CMD =>  06 20 0F A0 00 F0 00 02 00 00 00 00 00 00 00 00 07 00
   [2022-12-06 16:31:23.906] [0xad38] CMD =>  08 20 20 1C 02 01 04 03 03 AA FE 14 16 AA FE 10 00 00 7A 65 70 68 79 72 70 72 6F 6A 65 63 74 08 00 00 00
   [2022-12-06 16:31:23.910] [0xad38] EVT <=  0E 04 01 06 20 00
   [2022-12-06 16:31:23.912] [0xad38] CMD =>  09 20 20 0D 0C 09 54 65 73 74 20 62 65 61 63 6F 6E 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
   [2022-12-06 16:31:23.918] [0xad38] EVT <=  0E 04 01 08 20 00
   [2022-12-06 16:31:23.922] [0xad38] CMD =>  0A 20 01 01
   [2022-12-06 16:31:23.924] [0xad38] EVT <=  0E 04 01 09 20 00
   [2022-12-06 16:31:23.927] [0xad38] EVT <=  0E 04 01 0A 20 00

默认工程是beacon项目，这时候手机打开nrf_connect就可以看到我们发的广播了。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221206163742869.png
   :alt: 
