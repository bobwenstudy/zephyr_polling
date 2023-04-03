移植说明
========

简介
----

本项目设计之初就考虑到移植到不同平台的需要，毕竟代码最终要部署在实际运行环境中才有真实意义。

移植时主要需要考虑以下几个问题：

-  **代码运行平台**\ ：也就是协议栈运行在哪，目前项目支持运行在\ **windows**\ 环境中，\ **platform**\ 文件夹中定义了\ ``platform_interface.h``\ 声明了平台侧所需实现的接口。

-  **HCI接口实现**\ ：本项目是Host协议栈，要实现蓝牙功能，需要通过HCI接口实现和Controller芯片的通信。目前项目支持windows平台下的USB和UART接口实现，分别对应：\ ``windows_libusb_win32``\ 和\ ``windows_serial``\ 。

-  **协议栈接口实现**\ ：就是协议栈在不同平台和HCI模式下的适配。总体来说包含log模块、chipset模块、storage_kv模块和定时器模块的实现。

对于协议栈而言，部分功能需要根据平台的要求来实现。本项目所需的硬件资源很少，可以非常方便移植到不同应用平台中。

Log模块
-------

本项目有很多日志需要打印，不同平台的log打印方式各不相同，平台需要实现\ ``bt_log_impl_t``\ 所定义的接口，分别是：

-  **init**\ ：初始化接口，通知平台log模块初始化开始，平台可以做一些准备，如在windows平台中，为了方便离线分析问题，会创建log文件和cfa文件，记录过程中的日志。调用\ ``bt_log_impl_register()``\ 函数时调用一次。

-  **packet**\ ：HCI数据包日志接口，每次HCI层的数据包交互会通过该接口通知平台，平台可以根据需要记录该日志。在windows平台会将该数据存在cfa文件中，方便后续借助btsnoop工具debug。

-  **printf**\ ：字符串日志接口，协议栈中\ ``LOG_DBG``\ ，\ ``printk``\ 等字符串打印接口实现。

-  **point**\ ：高性能point点日志接口，暂未实现。

平台需要通过\ ``bt_log_impl_register()``\ 传入Log接口实现。

.. code:: c

   typedef struct
   {
       // init work
       void (*init)(void);
       // log packet
       void (*packet)(uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len);
       // log message
       void (*printf)(uint8_t level, const char *format, va_list argptr);
       // log point
       void (*point)(uint32_t val);
   } bt_log_impl_t;

   void bt_log_impl_register(const bt_log_impl_t *log_impl);

chipset模块
-----------

本项目是Host协议栈，不同Controller的行为除了标准HCI接口实现外，还需要一些特殊功能接口。在\ `Chipsets <https://zephyr-polling.readthedocs.io/en/latest/chipset/index.html#>`__\ 章节中有对现有市面的芯片进行抽象。平台需要根据不同的chipset实现该接口。\ ``struct bt_hci_chipset_driver``\ 包含如下接口：

-  **init_work**\ ：初始化接口，通知平台chipset模块初始化开始，平台可以做一些准备，如状态机初始化之类。调用\ ``bt_hci_chipset_driver_register()``\ 函数时调用一次。

-  **boot_start**\ ：HCI Reset
   Command发送前会回调该流程，完成操作后需调用\ ``bt_hci_set_boot_ready()``\ 通知协议栈启动后续流程。

-  **prepare_start**\ ：HCI Reset
   Command发送后会回调该流程，完成操作后需调用\ ``bt_hci_set_prepare_ready()``\ 通知协议栈启动后续流程。

-  **event_process**\ ：会将协议栈收到的HCI Event通过该接口上报。

平台需要通过\ ``bt_hci_chipset_driver_register()``\ 传入chipset接口实现。


.. note::

    如果Controller无特殊操作，无需实现该接口，也就是传入NULL即可。

.. code:: c

   struct bt_hci_chipset_driver
   {
       /**
        * @brief init work process.
        *
        */
       void (*init_work)(void);
       /**
        * @brief Boot work process.
        *
        */
       void (*boot_start)(void);

       /**
        * @brief Prepare work process.
        *
        * Work after send reset command.
        *
        */
       void (*prepare_start)(void);

       void (*event_process)(uint8_t event, struct net_buf *buf);
   };

   /**
    * @typedef bt_hci_event_process_t
    * @brief Callback for hci event handle.
    *
    * @param err zero on success or (negative) error code otherwise.
    */
   typedef void (*bt_hci_event_process_t)(uint8_t event, struct net_buf *buf);

   void bt_hci_set_boot_ready(void);

   void bt_hci_set_prepare_ready(void);

   int bt_hci_chipset_driver_register(const struct bt_hci_chipset_driver *drv);

.. _storagekv模块:

storage_kv模块
--------------

蓝牙交互中有一些信息需要持久化存储中，确保下次掉电后能正常使用，如Link
Key等信息。不同平台下的持久化存储方式各不相同，考虑到嵌入式芯片实际情况，采用轻量级key-value结构来存储持久化信息，以便掉电后重连。\ ``struct bt_storage_kv_impl``\ 包含如下接口：

-  **init_list**\ ：初始化接口，通知平台当前协议栈所需的key-value
   header信息，暂无用处，后续根据需要再实现相关功能。

-  **get**\ ：获取key对应的data数据。windows平台下直接获取文件对应的内容。

-  **set**\ ：保存key对应的data数据。windows平台按照key命名文件，并保存data到文件中。

-  **delete**\ ：删除key对应的data数据。windows平台删除key对应的文件。

平台需要通过\ ``bt_storage_kv_register()``\ 传入storage_kv接口实现。

.. code:: c


   enum bt_storage_kv_keys
   {
       KEY_INDEX_NULL = 0x0000,

       KEY_INDEX_LE_ID_ADDR_LIST = 0x0001,
       KEY_INDEX_LE_ID_IRK_LIST = 0x0002,
       KEY_INDEX_LE_ID_NAME = 0x0003,
       KEY_INDEX_LE_ID_APPEARANCE = 0x0004,

       KEY_INDEX_LE_KEY_INFO_LIST = 0x0100,
       KEY_INDEX_LE_KEY_INFO_ITEM_BASE = 0x0110,
   };

   #define KEY_INDEX_LE_KEY_INFO_ITEM(__x) (KEY_INDEX_LE_KEY_INFO_ITEM_BASE + (__x))

   struct bt_storage_kv_header
   {
       uint16_t key;
       uint16_t length; // zero length means length not same.
   };

   struct bt_storage_kv_impl
   {
       void (*init_list)(struct bt_storage_kv_header *list, uint16_t list_cnt);

       int (*get)(uint16_t key, uint8_t *data, uint16_t *len);

       void (*set)(uint16_t key, uint8_t *data, uint16_t len);

       void (*delete)(uint16_t key, uint8_t *data, uint16_t len);
   };

   void bt_storage_kv_register(const struct bt_storage_kv_impl *impl);

定时器模块
----------

蓝牙业务中有很多超时业务处理，和定时业务处理，同时实际应用开发也需要一些软件定时器实现。协议栈需要一个精度为1ms的定时器，平台通过周期调用\ ``sys_clock_announce()``\ 传入Elapsed
time。

.. code:: c

   /**
    * @brief Announce time progress to the kernel
    *
    * Informs the kernel that the specified number of ticks have elapsed
    * since the last call to sys_clock_announce() (or system startup for
    * the first call).  The timer driver is expected to delivery these
    * announcements as close as practical (subject to hardware and
    * latency limitations) to tick boundaries.
    *
    * @param ticks Elapsed time, in ticks
    */
   void sys_clock_announce(uint32_t ticks);

HCI模块
-------

协议栈最主要要实现的就是HCI接口，不同平台下，不同芯片类型，其物理接口各不相同。HCI总的来说包含下行和上行通道，从Host层来看下行通道包含HCI
Command和ACL交互。上行通道包含HCI Event和ACL交互。

.. note::

    协议栈的buffer申请和释放是FIFO结构，其支持申请和释放在不同线程进行，无需进入临界区处理。

下行通道
~~~~~~~~

平台需要通过\ ``bt_hci_driver_register()``\ 注册实现\ ``struct bt_hci_driver``\ 相关接口：

-  **open**\ ：HCI开启接口，通知平台打开HCI通道，也就是打开串口或者进行USB枚举。

-  **send**\ ：协议栈所需下发的HCI数据会通过该回调通知平台，平台再将该HCI数据打包发送给Controller。

.. code:: c


   /**
    * @brief Abstraction which represents the HCI transport to the controller.
    *
    * This struct is used to represent the HCI transport to the Bluetooth
    * controller.
    */
   struct bt_hci_driver
   {
       /**
        * @brief Open the HCI transport.
        *
        * Opens the HCI transport for operation. This function must not
        * return until the transport is ready for operation, meaning it
        * is safe to start calling the send() handler.
        *
        * If the driver uses its own RX thread, i.e.
        * CONFIG_BT_RECV_IS_RX_THREAD is set, then this
        * function is expected to start that thread.
        *
        * @return 0 on success or negative error number on failure.
        */
       int (*open)(void);

       /**
        * @brief Send HCI buffer to controller.
        *
        * Send an HCI command or ACL data to the controller. The exact
        * type of the data can be checked with the help of bt_buf_get_type().
        *
        * @note This function must only be called from a cooperative thread.
        *
        * @param buf Buffer containing data to be sent to the controller.
        *
        * @return 0 on success or negative error number on failure.
        */
       int (*send)(struct net_buf *buf);
   };

   /**
    * @brief Register a new HCI driver to the Bluetooth stack.
    *
    * This needs to be called before any application code runs. The bt_enable()
    * API will fail if there is no driver registered.
    *
    * @param drv A bt_hci_driver struct representing the driver.
    *
    * @return 0 on success or negative error number on failure.
    */
   int bt_hci_driver_register(const struct bt_hci_driver *drv);

上行通道
~~~~~~~~

平台收到Controller的数据包发送给协议栈需要先调用\ ``bt_buf_get_controller_tx_evt()``\ 或\ ``bt_buf_get_controller_tx_acl()``\ 获取buffer，将数据填充好后，调用\ ``bt_recv()``\ 接口将数据发送给协议栈处理。

.. code:: c

   // Alloc HCI Event buffer.
   struct net_buf *buf = bt_buf_get_controller_tx_evt();
   // Alloc HCI ACL buffer.
   struct net_buf *buf = bt_buf_get_controller_tx_acl();
   // Receive HCI Event/ACL
   bt_recv(buf);
