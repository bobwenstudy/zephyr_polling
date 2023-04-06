zephyr代码同步说明
==================

隔段时间需要和zephyr项目同步代码。

代码获取
--------

主要同步release分支的代码。\ `Releases · zephyrproject-rtos/zephyr
(github.com) <https://github.com/zephyrproject-rtos/zephyr/releases>`__

代码格式化
----------

代码风格两边并不相同，需要统一格式化。

拷贝：scripts\code_format_zephyr.py到根目录。

拷贝：.clang-format到根目录。

运行code_format_zephyr.py脚本。

代码比对并同步
--------------

利用比较工具同步代码。

zephyr中的subsys\bluetooth目录和zephyr_polling中的src目录进行比较。

zephyr中的include\zephyr\bluetooth目录和zephyr_polling中的src\bluetooth目录进行比较。

代码修改说明
------------

部分代码的编译开关zephyr是放在cmake中，而我们是直接通过在代码用#ifdef来开关。

经典蓝牙部分的代码不合入。

.h文件必须加入#ifdef声明

bt_hci_cmd_send_sync都切换为bt_hci_cmd_send

net_buf_pool切换为spool

irq_lock相关处理移除

STRUCT_SECTION_FOREACH展开

log模块统一管理。

net_buf_pool_get换成s_pool

CONFIG_BT_SETTINGS相关的settings_load处理不需要在enable之后。

keys.c的管理换成kv结构。

gatt sc相关先关闭，后续再开启。

gatt初始化行为调整。

gatt ccc相关先关闭。

att的eatt相关先关闭。

buffer管理相关先关闭。
