.. _footprint:

优化占用空间
============

整个照搬zephyr项目的Kconfig配置系统，可以灵活的实现对各种场景的功能开关和参数管理。

优化Code Size
-------------

整个蓝牙协议栈基本整个照搬zephyr项目的Kconfig系统，所有功能都可以根据需要进行开关，不需要的代码可以随时关闭，细节可以看 :ref:`kconfig`。

如需要开启BT central相关功能只要在应用的prj.conf中配置如下即可。

.. code:: 

   CONFIG_BT_CENTRAL=y

如需要开启日志打印功能，只需要配置如下就行。

.. code:: 

   CONFIG_BT_DEBUG_LOG=y

优化RAM Size
------------

蓝牙最大的buffer就是HCI的交互缓存池，zephyr本身提供了很多的参数来配置，所以可以用如下方式去调整POOL的大小，并且会根据选择的link的个数动态调整pool大小。

.. code:: 

   CONFIG_BT_BUF_ACL_RX_SIZE=255
   CONFIG_BT_BUF_ACL_TX_SIZE=251
   CONFIG_BT_BUF_CMD_TX_SIZE=255
   CONFIG_BT_BUF_EVT_DISCARDABLE_SIZE=255

   CONFIG_BT_L2CAP_TX_MTU=247

当然如果相关代码关闭了，相应的RAM也不需要了。
