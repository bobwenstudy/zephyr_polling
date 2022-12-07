.. _optimization_tools:

优化工具
========

map文件和反汇编文件
-------------------

系统编译后会在output目录下生成map和lst文件，可以直接对这些文件进行分析。

生成的map文件也可以使用\ `Amap \| Sergey
Sikorskiy <https://www.sikorskiy.net/info/prj/amap/>`__\ 来进行分析。

.. figure:: https://www.sikorskiy.net/info/prj/amap/images/amap.05.03.png
   :alt: 

.. _ramreport和romreport:

ram_report和rom_report
----------------------

实现Zephyr的ram_report和rom_report功能，目前只能对\ **elf**\ 文件进行分析，所以Windows下无法使用，但是嵌入式系统有需要可以使用。

使用方式就是调用\ ``make ram_report``\ 或\ ``make rom_report``\ 或者\ ``make all_report``\ 。

最终会生成\ **ram.json**\ 和\ **rom.json**\ 。这两个文件也可以导入到\ **nordic**\ 的\ **vscode**\ 环境下，可以借助其图形化工具进行分析。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221125112930355.png
   :alt: 

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221207155644150.png
   :alt: 
