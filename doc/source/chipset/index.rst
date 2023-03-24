Chipsets
========

本项目主要实现了蓝牙Host协议栈，并没有包含Controller部分，如果需要实现蓝牙交互，按照Core
Spec，需要通过HCI接口连接其他蓝牙芯片实现蓝牙功能。

由于HCI接口层是Spec有规定的，所以只要符合HCI接口的设备理论上都可以使用本项目的协议栈实现蓝牙功能。但是各家方案不同，存在一些特殊的配置行为，所以需要根据具体情况具体调整。

如下图所示，通过HCI接口，可以对接CSR/RTK/ATS/Nordic等厂商的蓝牙芯片，从而实现蓝牙功能。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208193337208.png
   :alt: 

HCI接口
-------

| 在Sig定义中，Host和Controller通过HCI接口进行通信。HCI协议本身只有5种接口。包括Command、Event、ACL、SCO以及ISO接口。
| 如下图所示，BLE只支持Command、Event、ACL和ISO数据通道，由于目前项目只完成BLE，并不涉及BLE
  Audio业务，所以只使用上述的3种接口，分别是：Command、Event和ACL接口。

-  **Command**\ ：Host到Controller的单向通道，主要传输Host发送给Controller的控制指令；

-  **Event**\ ：Controller到Host的单向通道，主要传输Controller上报给Host的事件；

-  **ACL**\ ：双向通道，承载了两个设备之间的数据交互；

-  **ISO**\ ：双向通道，承载了两个设备之间的音频数据交互；

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208193922536.png
   :alt: 

HCI包格式
---------

在Sig提供的\ `Core
Spec5.3 <https://www.bluetooth.com/specifications/specs/core-specification-5-3/>`__\ 中的Vol4中对HCI层有详细说明，重点说明了各个通道的包格式以及HCI
command和events。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208194858460.png
   :alt: 

HCI Command包格式
~~~~~~~~~~~~~~~~~

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208194723153.png
   :alt: 

HCI Event包格式
~~~~~~~~~~~~~~~

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208194705967.png
   :alt: 

HCI ACL包格式
~~~~~~~~~~~~~

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208194743750.png
   :alt: 

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208194755353.png
   :alt: 

HCI ISO包格式
~~~~~~~~~~~~~

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208195240829.png
   :alt: 

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208195250336.png
   :alt: 

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208195258443.png
   :alt: 

HCI接口类型
-----------

由于物理传输接口的不同，在HCI包格式的基础上，还会加入接口层协议，去固定不同接口类型下HCI数据包如何进行数据收发。

.. _uart接口h4）:

UART接口（H4）
~~~~~~~~~~~~~~

通常也被称之为H4接口，其结构如下所示。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208195528187.png
   :alt: 

在原本HCI交互的基础上，需要加入1个字节的Packet Type。

也就是说，HCI Reset
Command，按照Sig来说是\ ``03 0C 00``\ ，通过UART发送时就变成为\ ``01 03 0C 00``\ ，在开始的时候加入了1个字节的Packet
type。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208195537948.png
   :alt: 

注意：由于UART接口的不确定性，一般UART接口都会开启流控，不然在大数据吞吐时容易丢包。但是这样的话需要连接的IO就需要5根了，\ ``TX/RX/CTS/RTS/GND``\ 。

注意，由于UART本身没有同步接口，对于SCO等同步数据交互，该接口的体验并不是很好。

注意：UART接口一般需要确认串口号，波特率，流控等参数，这些参数根据不同厂商有不同的定义。在不开启流控的芯片中可以不接CTS/RTS接口。

.. _3线uart接口h5）:

3线UART接口（H5）
~~~~~~~~~~~~~~~~~

一般的UART接口加上流控就有\ ``TX/RX/CTS/RTS/GND``\ 5个IO了，对于资源有限的场景，IO很多时候并不充裕，并且就算加上流控，当UART接口收到错误数据时，依然无法实现重传，进而整个协议栈就崩溃了。

考虑到这个情况，Sig提出了既省IO，又有流控、重传和唤醒功能的\ **H5**\ 接口。通过在UART接口上加上一层协议来实现这一功能需要。该方案只需要\ ``TX/RX/GND``\ 三个IO，所以又称之为3线UART。

其架构如下，通过协议来保证数据交互的可靠性。具体可以看Core Spec。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221208200444004.png
   :alt: 

H5协议的功能较为完善，但是其实现较为复杂，占用系统资源不少，对系统可靠性和资源有限场景会使用该协议。

.. _usb接口h2）:

USB接口（H2）
~~~~~~~~~~~~~

目前市面上大多数HCI
Dongle都是使用这一接口协议，借助于USB接口的高速和可靠性，完全可以满足HCI数据/音频传输要求。实际的PC蓝牙Dongle设备很多都是使用USB接口。

在spec中规定了USB接口行为，Host一般作为USB Host端，Controller为USB
Controller。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103160944400.png
   :alt: 

两个USB Dongle设备的交互框图如下所示。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103161054215.png
   :alt: 

USB接口关键就是数据交互的端点配置，针对于蓝牙HCI接口的特点，SPEC定义了多种接口来分别承载不同类型的数据交互。

如下所示，Command通过0x00的Control
Endpoint传输，Event数据包通过0x81的Interrupt
Endpoint传输，ACL数据包通过0x82和0x02的Buck接口进行传输。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103161719636.png
   :alt: 

音频传输通过Isoch端口传输，此外又根据不同的同步传输格式，固定了不同的接口行为。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103161942944.png
   :alt: 

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103161959635.png
   :alt: 

在本项目中，也是按照Spec规定了完成和USB设备的数据交互，详见：\ ``platform/windows/windows_driver_libusb.c``\ 。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103162233842.png
   :alt: 

注意，在嵌入式产品中，由于嵌入式主芯片资源受限，一般而言都是使用UART接口，部分场景下会使用SPI/SDIO接口。

SD接口
~~~~~~

基于SDIO接口来完成HCI数据交互。

其架构如下，由于笔者实际项目也没怎么用，就不在多数，实际使用看芯片手册定义，基本都会按照SPEC来配置接口。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103162513561.png
   :alt: 

H4 over SPI
~~~~~~~~~~~

UART接口需要确定的波特率，对于一些高速需求并不是很好满足，并且可移植性相对较差。SPI接口由master提供时钟，无需预先知道两边的波特率配置即可进行通信。

目前ST主推的是SPI接口的HCI Controller，详见\ `BlueNRG-2 -
蓝牙低功耗无线单芯片 -
STMicroelectronics <https://www.st.com/zh/wireless-connectivity/bluenrg-2.html>`__\ 。

由于笔者基本没有用过这个接口，并且这个接口在Spec并未定义，所以也不具体展开，具体使用到特定芯片时，按照产商的要求使用即可。

芯片启动配置
------------

目前市面上的HCI
Dongle产品大多数并不能拿来直接使用，也就是说在用标准的HCI指令控制设备进行蓝牙操作之前，需要提前通过Vendor
Command对设备进行配置，只有正确配置好的设备才能正常使用。

为什么厂商不将设备配置为上电就能完成HCI蓝牙交互行为呢，反而需要配置这么多参数呢，这样并不利于用户使用。

从产商角度来看。类似于HCI
Dongle之类的产品一般而言是由芯片产商提供固件预烧录在芯片中，而芯片产商只出售芯片，并不关注外围电路和具体的产品形态，这些是具体的ODM厂商来实现的。也就是芯片产商提供带HCI蓝牙功能的芯片，ODM设计电路并设计产品，之后通过HCI和芯片进行交互。实际各家ODM产商的需求各不相同，芯片产商为了满足不同客户的需要，并且为了减少和客户的对接，就必然在同一套代码的基础上，需要提供一系列的配置参数来满足不同ODM厂商的需要。

从使用角度来看。首先一点，像HCI
Dongle这类产品一方面是提供给专业用户使用，如开发人员等，这些人使用之前必然要查看产商的手册进行配置；而像蓝牙适配器这类产品对应的PC都有相应的驱动程序，用户安装产商提供的驱动程序就可以正常使用产品，驱动程序屏蔽了HCI接口，也完成了相关配置。也就是说这样的行为并不影响绝大多数用户的使用。

根据各家芯片产商的情况，所需配置的参数和种类各不相同，下面对一些参数配置进行说明。

注意，由于嵌入式资源有限，现有市面上的产品基本都是通过HCI的Vendor
Cmd/Event接口来实现对设备的参数配置行为。

固件烧录
~~~~~~~~

以Realtek为典型，这类芯片产品的代码基本都存储在ROM中，一个品类的产品ROM在芯片TO阶段就已经固定下来了，之后需要修改BUG或者加新功能时，就需要重新调整ROM，这样芯片产商需要维护的成本过高，并且不同应用场景所需的代码并不相同，所有都放入ROM中会导致芯片成本过大。

那么其芯片结构基本为ROM+Patch形式，Patch如果存储在Flash之类的形态中，对于终端用户其实也无感，使用之前预烧录好特定的Patch即可（Nordic/ST/Actions之类都有Flash）。可是以Realtek/CSR的厂商所提供的芯片并没有Flash，而是有一个较大的Patch
RAM，由于RAM掉电后就会丢失，所以在每次上电时都需要通过HCI将最新的固件下载到Patch
RAM中，不然默认出厂的ROM大多数情况下是工作异常的。

Patch RAM的形式虽然省却了Flash的成本，但是在低功耗产品时，由于Patch
RAM一般都不小，其功耗普遍较高。并且RAM都是芯片成本，在设计之初就定好了，太大成本过高/功耗过高。太小的话能满足的应用场景就较少。

同时由于每次掉电都需要烧录新的固件，要使用蓝牙功能之前准备时间都是百ms级别，对一些特定应用场景需要还是有点繁琐的。

蓝牙地址配置
~~~~~~~~~~~~

由于蓝牙地址都是需要向Sig花钱买的，有一些芯片厂商（ST/CSR）将这部分成本转嫁到ODM和终端用户上（很多时候ODM和终端用户也并不会付费，由于他们体量较小，Sig一般也追不回这些费用）。基于这一实际需求，芯片产商会提供一个Vendor
Command专门来实现蓝牙地址的配置，美曰其名给用户配置权利，实际大家懂的都懂。

硬件接口配置
~~~~~~~~~~~~

如RF接口，晶振类型等，CSR的需要配置这些，如当前的RF接口的配置，系统时钟源，低功耗时钟源等硬件接口的配置，这个大厂灵活性较大，所需配置的东西就较多，大多数产品形态基本不需要管这些。

串口波特率配置
~~~~~~~~~~~~~~

大多数是UART接口，一般默认都是运行在115200的配置下，但是这个速度太慢了，有时需要切换成更高的波特率。

其他参数配置
~~~~~~~~~~~~

按照厂商手册需要配置即可。

无需配置
~~~~~~~~

对于有Flash的芯片，如（Nordic/ST/Actions）等产商的芯片，完全可以将配置参数和Patch
Code预烧录到Flash中，使用时完全不需要配置任何参数，直接通过HCI接口操作使用即可。

芯片BOOT操作接口
----------------

zephyr_polling协议栈需要满足不同芯片的使用需要，而这些不同产商的特殊配置需要放在协议栈内部并不方便，并且也不利于后期用户添加新的chipset。从上述分析已经知道现在市面上芯片的启动配置需求，所以围绕于这些功能需要，项目提供了Boot流程接口和Prepare流程接口。

系统启动流程如下，先进行Boot流程，之后下发HCI Reset
Command，而后进行Prepare流程，最后进行常规的HCI boot流程。

整个协议栈的HCI初始化流程是通过状态机来管理的，通过\ ``hci_state_polling()``\ 轮询进行调度，chipset通过\ ``bt_hci_chipset_driver_register()``\ 注册操作接口。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103191652806.png
   :alt: 

Boot流程接口
~~~~~~~~~~~~

用于完成厂商的初始化流程，如固件下载，蓝牙地址配置等。

chipset注册好接口后，协议栈启动时会通过\ ``boot_start()``\ 回调启动chipset的boot流程，由于操作接口是HCI，所以一般都是下发一个Vendor
Command，然后根据Vendor
Event来进行后续动作，协议栈会通过\ ``event_process()``\ 回调接口将收到的event上报给chipset，当chipset认为操作结束时，通过调用\ ``bt_hci_set_boot_ready()``\ 接口通知协议栈boot流程结束。

Prepare流程接口
~~~~~~~~~~~~~~~

照理说Boot流程已经完成了厂商的参数配置等工作，为什么还需要定义一个Prepare流程呢？这是因为类似于ST厂商的参数要求在HCI_Reset
Command之后进行，也就是说HCI_Reset Command会清空配置的参数。

所以为了兼容这类参数形态，HCI_Reset
Command之后还需要进行Prepare流程，协议栈收到HCI_Reset的Command Complete
Event后会通过\ ``prepare_start()``\ 回调启动chipset的Prepare流程，和boot流程一样，协议栈会通过\ ``event_process()``\ 回调接口将收到的event上报给chipset，当chipset认为操作结束时，通过调用\ ``bt_hci_set_prepare_ready()``\ 接口通知协议栈prepare流程结束。

支持芯片列表
------------

协议栈在不断完善中，后续会陆续加入更多支持的芯片，大家需要的话可以购买所需的Dongle来验证本项目协议栈，当然也可以在自家的芯片上，实现chipset接口，来玩本项目。

+-------------------+-------------------+------+----------+-----------+
| 厂商              | chipset           | 接口 | 蓝牙版本 | 类型      |
+===================+===================+======+==========+===========+
| `CSR <https://www | `csr8510 <ht      | USB  | 4.0      | Dual Mode |
| .qualcomm.cn/>`__ | tps://detail.tmal |      |          |           |
|                   | l.com/item.htm?ab |      |          |           |
|                   | bucket=2&id=53466 |      |          |           |
|                   | 2513906&ns=1&spm= |      |          |           |
|                   | a230r.1.14.1.2f68 |      |          |           |
|                   | 11a37qFFQU&skuId= |      |          |           |
|                   | 4910946697067>`__ |      |          |           |
+-------------------+-------------------+------+----------+-----------+
| `CSR <https://www | `csr89            | UART | 4.0      | Dual Mode |
| .qualcomm.cn/>`__ | 10 <https://item. |      |          |           |
|                   | taobao.com/item.h |      |          |           |
|                   | tm?spm=a1z09.2.0. |      |          |           |
|                   | 0.6cd22e8dj2naR0& |      |          |           |
|                   | id=622836061708&_ |      |          |           |
|                   | u=3m1kbkea372>`__ |      |          |           |
+-------------------+-------------------+------+----------+-----------+
| `炬芯-Actio       | `ats2851 <ht      | USB  | 5.3      | Dual Mode |
| ns <http://www.ac | tps://detail.tmal |      |          |           |
| tions.com.cn/>`__ | l.com/item.htm?ab |      |          |           |
|                   | bucket=2&id=53466 |      |          |           |
|                   | 2513906&ns=1&spm= |      |          |           |
|                   | a230r.1.14.1.2f68 |      |          |           |
|                   | 11a37qFFQU&skuId= |      |          |           |
|                   | 5111551883875>`__ |      |          |           |
+-------------------+-------------------+------+----------+-----------+
| `Nordi            | `pts_dong         | UART | 5.3      | LE Only   |
| c <https://www.no | le <https://item. |      |          |           |
| rdicsemi.com/>`__ | taobao.com/item.h |      |          |           |
|                   | tm?spm=a1z09.2.0. |      |          |           |
|                   | 0.6cd22e8dj2naR0& |      |          |           |
|                   | id=622836061708&_ |      |          |           |
|                   | u=3m1kbkea372>`__ |      |          |           |
+-------------------+-------------------+------+----------+-----------+

芯片厂商配置说明
----------------

.. _csr--qualcomm-incorporated:

CSR / Qualcomm Incorporated
~~~~~~~~~~~~~~~~~~~~~~~~~~~

CSR目前已经被高通收购了，但是市面上还有蛮多CSR的Dongle在卖。

市面上卖得比较多，有一套csr得命令，叫做《BCCMD Protocol》，利用里面的PS
Command中《PSKEY_BDADDR》设置蓝牙地址。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103193830348.png
   :alt: 

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103193849093.png
   :alt: 

csr8510
^^^^^^^

市面买到的蓝牙适配器很多是这个。USB接口。

**Boot流程**\ ，主要完成如下操作：关闭NOP；设置蓝牙地址；warn重启。

**Prepare流程**\ ，None。

csr8910
^^^^^^^

市面买到的蓝牙适配器很多是这个。USB接口。

**Boot流程**\ ，主要完成如下操作：模拟参数配置；关闭NOP；启动串口流控；设置蓝牙地址；warn重启。

**Prepare流程**\ ，None。

ST
~~

ST推出了一套叫BlueNRG的ble东西，按照其官方推荐，到淘宝上分别买《X-NUCLEO-BNRG2A1》和《NUCLEO-L476RG》
开发板。

《X-NUCLEO-BNRG2A1》预烧录了BLE+SPI的程序。

《NUCLEO-L476RG》
按照下图，烧录一个Virtual_COM_Port的程序，就可以在PC上通过Uart控制蓝牙芯片了。

默认其推荐的是用《BlueNRG GUI》来控制。

ST的《X-NUCLEO-BNRG2A1》中的BLE本身是一个SOC，里面集成了Host的协议栈，ST提供了一套ACI指令来控制芯片行为，包括host的接口。

也就是说默认的情况下，这个芯片的ACL交互都被芯片接管了，所以需要通过ACI命令，关闭Host行为。必须通过《aci_hal_write_config_data》将《LL_WITHOUT_HOST》关闭。

public地址通过必须通过《aci_hal_write_config_data》将《CONFIG_DATA_PUBADDR_OFFSET》配置蓝牙地址。

注意，官方提供的Virtual_COM_Port代码并不支持将ACL转发的功能，此外在压力测试时，由于其代码写的可靠性不高，会有丢包等问题，需要自己把这块串口接收转发的代码逻辑给修改掉。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103194728104.png
   :alt: 

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103194746236.png
   :alt: 

CSR目前已经被高通收购了，但是市面上还有蛮多CSR的Dongle在卖。

市面上卖得比较多，有一套csr得命令，叫做《BCCMD Protocol》，利用里面的PS
Command中《PSKEY_BDADDR》设置蓝牙地址。

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103193830348.png
   :alt: 

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230103193849093.png
   :alt: 

BlueNRG-2
^^^^^^^^^

LE Only的SOC，第二代产品，需要在其原本代码上进行修改进行使用。

**Boot流程**\ ，None。

**Prepare流程**\ ，蓝牙地址配置；关闭Host功能；设置TX
power；GATT配置；GAP配置。

Nordic
~~~~~~

nordic作为行业龙头，其芯片是全Flash的芯片，其并没有出专门的蓝牙适配器，但是市面上还是能购买到一些用他们家产品做的的HCI
Dongle，并且Sig也用NRF52840做了PTS Dongle，用于Host的认证。

nordic的蓝牙地址是每个芯片唯一，厂商预先配置好了（没细的研究）。

PTS Dongle
^^^^^^^^^^

Sig用NRF52840做的一个HCI
Dongle，可以说是最标准蓝牙行为的蓝牙Dongle了。所有的参数都是在Flash配置好的，直接使用即可。每个芯片有独立的蓝牙地址。

**Boot流程**\ ，None。

**Prepare流程**\ ，None。

炬芯-Actions
~~~~~~~~~~~~

通过绿联的5.3 Dongle认识的，。

ats2851
^^^^^^^

绿联在卖的5.3
Dongle，应该也是Flash版本的芯片，每个设备的蓝牙地址都是唯一的，无需配置。

**Boot流程**\ ，None。

**Prepare流程**\ ，None。

AP6212-brcm
~~~~~~~~~~~

通过ArtPi认识的。

板子这4个点是Uart点。

原理图在这：\ `sdk-bsp-stm32h750-realthread-artpi/ART-Pi_SCH_V1.5_Release.pdf
at master · RT-Thread-Studio/sdk-bsp-stm32h750-realthread-artpi
(github.com) <https://github.com/RT-Thread-Studio/sdk-bsp-stm32h750-realthread-artpi/blob/master/documents/board/ART-Pi_HW_V1.5/ART-Pi_SCH_V1.5_Release.pdf>`__

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324172314816.png
   :alt: 

.. figure:: https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20230324171200993.png
   :alt: 

AP6212
^^^^^^

需要注意，有2个地方可能会对方不回数据，需要重传。一个是刚配置完UART口后，发太快可能会导致对方收不到。另一个是烧录完image后，最后reset动作需要等一会才发，当然现在都是按照重传来做的。

这个芯片需要将\ **BT_RST_EN**\ 和\ **BT_WAKEUP**\ 脚拉高，不然芯片就会没上电，或者进入睡眠状态。

**Boot流程**\ ，将image发下去就行。image在这，\ `sdk-bsp-stm32h750-realthread-artpi/ap6212-bt-image-1.0.rbl
at master · RT-Thread-Studio/sdk-bsp-stm32h750-realthread-artpi
(github.com) <https://github.com/RT-Thread-Studio/sdk-bsp-stm32h750-realthread-artpi/blob/master/tools/firmware/ap6212-bt-image-1.0.rbl>`__\ 。为了省事换成了数组来实现。

**Prepare流程**\ ，None。
