# 简介

[![Compile Check](https://github.com/bobwenstudy/zephyr_polling/actions/workflows/github-actions-demo.yml/badge.svg)](https://github.com/bobwenstudy/zephyr_polling/actions/workflows/github-actions-demo.yml) [![Documentation Status](https://readthedocs.org/projects/zephyr-polling/badge/?version=latest)](https://zephyr-polling.readthedocs.io/en/latest/?badge=latest)

本项目是基于[Zephyr Project](https://www.zephyrproject.org/)进行二次开发的，去除了OS调度部分，只保留了Bluetooth的Host协议栈。项目地址：[bobwenstudy/zephyr_polling (github.com)](https://github.com/bobwenstudy/zephyr_polling)。文档地址：[Welcome to Zephyr_polling’s documentation!](https://zephyr-polling.readthedocs.io/en/latest/)。

一般蓝牙协议栈分为Host和Controller两个部分，根据是否包含BR/EDR还是LE，分为如下形式。

![image-20221124185342589](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221124185342589.png)

作为一个蓝牙芯片协议栈开发者，既做过Controller也做过Host，同时在多家公司任职过，接触过包括rwip\ceva\bluedroid\bluez\btstack\nimble\softdevice以及各种自定义协议栈（擅长的不多，哈哈），总体就是各个协议栈都有各自的写法，不管是代码风格还是环境都各不相同。

**Controller侧**，虽然Controller要实现RF操作，和硬件行为强相关，不可避免代码差异大。但是Spec有明确规定HCI接口，大家都有一个标准可以遵循，API相对统一，从A切换B其实只要有HCI接口就能方便的使用。

如下图所示，Host和Controller之间通过HCI连接，对于BLE而言只有C/E、ACL、ISO这4个通道。

![image-20221124185611408](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221124185611408.png)

**Host侧**，Spec只是规定了行为并没有规定接口，那就是天马行空，各家方案的API差异很大，学习成本较高，并且迁移成本也高，而且文档基本上都是靠代码、例程来学习，有些API还不是很友好。

现行的开源Host协议栈还是比较多的，下面做一个简单的介绍：

| 名称                                                         | 开源协议 | 双模支持             | 持续维护 | OS/轮询 | 备注                                                         |
| ------------------------------------------------------------ | -------- | -------------------- | -------- | ------- | ------------------------------------------------------------ |
| [bluedroid](https://android.googlesource.com/platform/external/bluetooth/bluedroid/+/refs/heads/master) | 开源免费 | 是                   | 是       | OS      | Android所使用的蓝牙协议栈，功能比较全，但是Profile层是在Java实现的，所以有个JNI层，相应的行为就复杂了一些。 |
| [BlueZ](http://www.bluez.org/)                               | 开源免费 | 是                   | 是       | OS      | Linux系统所使用的蓝牙协议栈，没深入理解，但是bluedroid的是从blueZ演变过来的，记得Android4.0之前用的还是blueZ，后面改成了bluedroid。 |
| [Zephyr](https://www.zephyrproject.org/)                     | 开源免费 | 是(经典蓝牙实现较少) | 是       | OS      | 本文所基于的协议栈，蓝牙之前其中的一个部分，包含Controller的实现，API很不错，目前很火的开源协议栈，没事大家可以上去学点东西，后面详细来讲。 |
| [btstack](https://github.com/bluekitchen/btstack)            | 商业收费 | 是                   | 是       | 轮询    | 整体就是为了满足低成本方案设计来做的，没有OS需求，协议栈的调度就是轮询架构，可以轻易的porting到任何平台中，功能也全，本项目的架构也是参考这一项目来实现的。里面有很多芯片的Porting实现。 |
| [nimble](https://github.com/apache/mynewt-nimble)            | 开源免费 | BLE                  | 是       | OS      | 没深入了解，不过很久没维护了，最近突然又开始维护了，好像是乐鑫开始用他们的方案。 |

从上述可以看出，除了btstack在设计之处就是为了方便Porting存在的，其他方案都是捆绑了自己的OS。

这块其实做的比较好的还是btstack，不过其商业是收费的，文档这块其实也不太多，同时其是极简实现的方案，所以使用体验上并不是很舒服，数据缓存之类都需要在应用层做。



# 改动说明

作为一个芯片开发人员，国内大多数芯片方案还是资源受限的平台，不管对于Code Size，RAM Size都非常敏感；同时对于低功耗设计，Retention RAM的大小也至关重要。上述协议栈在各自的领域都做的不错，但是对于国内市场需要开源免费、API友好、Code/RAM/Retention Size精简的需要来讲，多少还是有一些缺陷的。

基于这一现状，基于Zephyr的蓝牙子系统，保留其API和蓝牙协议实现，做出如下调整：

- 调度方式，由原本基于Zephyr OS调度，调整为轮询调度方式，以便于后续移植到其他平台，并减少对CPU mips的占用。
- buffer管理，移除关于多thread的操作，保留pool和net_buf设计。
- 配置系统，保留kconfig配置架构，支持.h单独配置方式，简化配置复杂度。
- 存储系统，由原本的setting架构调整为简易的kv结构。
- 系统架构，调整为btstack架构。
- hci操作，原本有很多同步操作，调整为异步操作。
- Retention RAM分离，Zephyr本身需要的RAM还是比较大的，将pool等大块memory调整为非retention设计，以便低成本方案移植需要。
- API，移除同步操作接口，其他接口全部保留。
- 代码风格，依然基于.clang-format来格式化代码，但是zephyr蓝牙部分的代码风格和他OS说明好像并不相同，所以重新定义了一个我自己舒服的格式。
- 编译系统，全部基于makefile来组织编译，方便大家移植。



# 系统架构

蓝牙协议栈为Zephyr的结构，系统总体架构参考Btstack的实现，总体结构如下图所示。

![image-20221125111329111](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221125111329111.png)

如上图所示，系统主要分为5个部分，代码结构如下所示：

- **chipset**：各家厂商在使用之前需要进行一些配置，有些是因为芯片是rom化版本，需要加载patch，有些要配置RF参数，有些要配置蓝牙地址等。
- **example**：各种蓝牙例程，基本是照搬zephyr的来，当然会加入一些新的case。
- **platform**：移植时重点关注的部分，蓝牙协议栈运行需要用到一些平台资源，不同平台有不同实现方式，主要包括log、timer、storage_kv和HCI接口的实现。
- **porting**：程序的主入口，这些会将platform/chipset和协议栈接口进行绑定，并启动example，最后对协议栈进行调度。
- **src**：zephyr的蓝牙协议栈部分，具体实现蓝牙协议栈的具体细节。

```shell
zephyr_polling
 ├── chipset
 │   ├── csr8510_usb
 │   └── csr8910_uart
 ├── example
 │   ├── beacon
 │   ├── broadcaster
 │   ├── central
 │   ├── observer
 │   ├── peripheral
 │   ...
 ├── platform
 │   └── windows
 │   ...
 ├── porting
 │   ├── windows_libusb_win32
 │   └── windows_serial
 │   ...
 └── src
     ...
```





# 使用说明

## 环境搭建

目前暂时只支持Windows编译，最终生成exe，可以直接在PC上跑。

目前需要安装如下环境：

- [Python3](http://www.python.org/getit/)，用于Kconfig，代码格式化，RAM&ROM分析等，编译工具类都用这个。
- GCC环境，笔者用的msys64+mingw，用于编译生成exe，参考这个文章安装即可。[Win7下msys64安装mingw工具链 - Milton - 博客园 (cnblogs.com)](https://www.cnblogs.com/milton/p/11808091.html)。



### Python环境准备

python装好后，还需要安装一些环境，运行`python_require_env.py`脚本就会安装所有所需的python环境。

```shell
python python_require_env.py
```





## 编译说明

本项目都是由makefile组织编译的，编译整个项目只需要执行`make all`即可，调用`make help`可以查看帮助。

根据具体需要可以调整一些参数，目前Makefile支持如下参数配置。

- **APP**：选择example中的例程，默认选择为`example/beacon`。
- **PORT**：选择porting中的环境，也就是当前平台和HCI接口类型，默认选择为`porting/windows_libusb_win32`。
- **CHIPSET**：选择chipset中的芯片种类，默认选择为`chipset/csr8510_usb`。

也就是可以通过如下指令来编译工程：

```shell
make all APP=example/beacon PORT=porting/windows_libusb_win32 CHIPSET=chipset/csr8510_usb
```





## HCI Dongle部署

在PC环境下，常见的设备有USB设备和UART设备，下面分别对这两个设备进行部署。

### USB设备使用

为了能操作这些USB蓝牙dongle，默认使用的驱动是蓝牙的驱动。所以需要更改设备所使用的驱动。

- Step1：下载[Zadig](https://zadig.akeo.ie/)。
- Step2：菜单栏点击Options -> List All Devices。

![image-20221125133827682](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221125133827682.png)

- Step3：通过下拉选中当前连接的蓝牙dongle，更换设备driver为`libusb-win32`，如下图所示，过一会就换好驱动了。

![image-20221125133953130](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221125133953130.png)



### UART设备使用

这个直接看以下设备的串口号，在代码里面配置好后就可以正常使用了。（这块还没做，准备放在Makefile中操作）





## 支持芯片列表

makefile中配置**CHIPSET**，后续不断完善。

| chipset                                                      | 接口 | 蓝牙版本 |
| ------------------------------------------------------------ | ---- | -------- |
| [csr8510_usb](https://detail.tmall.com/item.htm?abbucket=2&id=534662513906&ns=1&spm=a230r.1.14.1.2f6811a37qFFQU&skuId=4910946697067) | USB  | 4.0      |
|                                                              |      |          |
|                                                              |      |          |





# 蓝牙配置

用的就是zephyr的Kconfig配置，使用办法就是输入如下指令：

```shell
make menuconfig
```

和zephyr一样，支持持久化配置和临时配置，各个example里面有一个`prj.conf`作为特定应用的永久化配置，当然使用过程中可以随时通过menuconfig调整配置，但是注意需要保存时，将差异保存在`prj.conf`中，不然下次clean后会失效。Kconfig的使用说明可以参考：[从零到一搭建Kconfig配置系统](https://blog.csdn.net/wenbo13579/article/details/127464764)。

kconfig生成的`autoconfig.h`保存在output目录下。

![image-20221125141858546](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221125141858546.png)

**注意**：考虑到部分项目不想使用kconfig的配置方案，生成`autoconfig.h`的使用在`src\bt_config.h`中通过include导入，也就是说可以在其他环境下生成好`autoconfig.h`，而后将生成的`autoconfig.h`保存在`src`目录下即可。



# 调试

## 日志系统

参考btstack，系统支持多种debug方式，生成的日志文件保存在output/log目录下：

| 名称                 | 分析软件          | 备注                                                   |
| -------------------- | ----------------- | ------------------------------------------------------ |
| btsnoop(.cfa)        | frontline/eclipse | 分析蓝牙协议最好的一个协议存储格式                     |
| packet_logger(.pklg) | wireshark         | 暂不支持，苹果推出的一个日志分析格式，支持log+协议分析 |
| 日志(.log)           | 文本编辑器        | 将终端的日志保存成文件，方便离线分析                   |

![image-20221125112900757](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221125112900757.png)

## RAM_Report&ROM_Report

参考zephyr，对生成的elf进行分析，最终会生成ram.json和rom.json。这两个文件也可以导入到nordic的vscode环境下，可以借助其图形化工具进行分析。

**注意**：windows环境暂时不可用，只能对elf进行分析。

![image-20221125112930355](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221125112930355.png)

![image-20221125113047913](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20221125113047913.png)





