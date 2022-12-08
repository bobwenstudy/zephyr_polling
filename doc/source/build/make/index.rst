.. _make:

构建系统(make)
==============

编译架构
--------

为了方便移植，本项目是用makefile组织编译的。

根路径的\ ``<home>/Makefile``\ 是总入口，考虑到协议栈、应用层、chipset以及platform之类的有其独特的编译配置，每个模块下面都有\ ``build.mk``\ ，用于配置所需参与编译的代码文件、头文件和所需的库。

如下所示，Home目录的Makefile会依次加载：

-  ``src/build.mk``\ ：蓝牙协议栈编译配置。

-  ``example/${APP}/build.mk``\ ：根据输入的\ **APP**\ 参数选择example中例程进行编译。

-  ``chipset/${CHIPSET}/build.mk``\ ：根据输入的\ **CHIPSET**\ 参数选择chipset中芯片进行编译。

-  ``porting/${PORT}/build.mk``\ ：根据输入的\ **PORT**\ 参数选择porting中部署进行编译。

.. code:: 

   zephyr_polling
    ├── Makefile
    ├── chipset
    │   └── ${CHIPSET}
    │       ├── ...
    │       └── build.mk
    ├── example
    │   └── ${APP}
    │       ├── ...
    │       └── build.mk
    ├── platform
    │   └── windows
    │       ├── ...
    │       └── build.mk
    ├── porting
    │   └── ${PORT}
    │       ├── build.mk
    │       └── main.c
    └── src
        ├── ...
        └── build.mk

编译参数
--------

由于蓝牙Dongle的接口、运行平台和例程不相同，为了区分不同的场景，特定应用只编译应用相关的东西，需要用户编译时配置不同的参数。

核心参数
~~~~~~~~

-  **APP**\ ：实际就是去找example/${APP}路径。如配置为\ ``APP=beacon``\ 。

-  **CHIPSET**\ ：实际就是去找chipset/${CHIPSET}路径。如配置为\ ``CHIPSET=csr8510``\ 。

-  **PORT**\ ：实际就是去找porting/${PORT}路径。如配置为\ ``PORT=windows_libusb_win32``\ 。

GCC参数

**SRC**\ ：代码文件路径，需要在各个分模块配置路径。

**INCLUDE**\ ：头文件路径，需要在各个分模块配置路径。

**LIB**\ ：库文件路径，需要在各个分模块配置路径。

**LFLAGS**\ ：库文件名称，需要在各个分模块按需配置。

**CFLAGS**\ ：gcc编译的c flags，如优化配置\ ``-O0``\ 。

**LDFLAGS**\ ：gcc编译的link
flags，如\ ``-Wl,--gc-sections -Wl,--check-sections``\ 。

**CPU_ARCH**\ ：cpu架构配置，由于windows平台的cpu架构不同，需要应用不同的lib，所以需要特别配置。

其他参数
~~~~~~~~

**V**\ ：调试开关，配置\ ``V=1``\ 可以显示所有make过程的日志，便于调整编译流程。

**NOGC**\ ：开启或关闭gcc编译的\ ``gc sections``\ ，配置\ ``NOGC=1``\ 就会关闭\ ``gc sections``\ 。

编译目标
--------

**all**\ ：编译生成执行文件。

**clean**\ ：清空中间文件和生成文件。

**help**\ ：编译帮助，显示help文档。

**info**\ ：打印当前的配置参数。

**run**\ ：编译&执行程序。

**code_format**\ ：对项目代码进行格式化，格式化配置为：\ ``<home>/.clang-format``

**menuconfig**\ ：kconfig的配置界面。

**guiconfig**\ ：kconfig的配置界面-另外一个显示方式。

**ram_report**\ ：对生成的main.elf进行ram分析，并生成\ ``output/ram.json``\ 。

**rom_report**\ ：对生成的main.elf进行rom分析，并生成\ ``output/rom.json``\ 。

**all_report**\ ：执行ram_report和rom_report。
