#include <stdio.h>
#include <pthread.h>
#include <windows.h>

#include "windows_driver_serial.h"
#include "drivers/hci_driver.h"
#include "drivers/hci_h4.h"

#include "logging/bt_log_impl.h"

#include "common/bt_buf.h"

// https://blog.csdn.net/u010835747/article/details/117357403

BOOL synchronizeflag;
PORT OpenPort(int idx, BOOL sync)
{
    HANDLE hComm;
    TCHAR comname[100];
    synchronizeflag = sync;
    wsprintf(comname, TEXT("\\\\.\\COM%d"), idx);
    if (synchronizeflag)
    {
        hComm = CreateFile(comname,                      // port name
                           GENERIC_READ | GENERIC_WRITE, // Read/Write
                           0,                            // No Sharing
                           NULL,                         // No Security
                           OPEN_EXISTING,                // Open existing port only
                           0,                            // Non Overlapped I/O
                           NULL);                        // Null for Comm Devices
    }
    else
    {
        hComm = CreateFileA(comname,                      //串口名
                            GENERIC_READ | GENERIC_WRITE, //支持读写
                            0,                            //独占方式，串口不支持共享
                            NULL,                         //安全属性指针，默认值为NULL
                            OPEN_EXISTING,                //打开现有的串口文件
                            FILE_FLAG_OVERLAPPED, // 0：同步方式，FILE_FLAG_OVERLAPPED：异步方式
                            NULL); //用于复制文件句柄，默认值为NULL，对串口而言该参数必须置为NULL
    }

    if (hComm == INVALID_HANDLE_VALUE)
        return NULL;
    // unit ms
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetupComm(hComm, 1024, 1024))
    {
        return false;
    }
    if (SetCommTimeouts(hComm, &timeouts) == FALSE)
        return NULL;

    if (SetCommMask(hComm, EV_RXCHAR) == FALSE)
        return NULL;

    PurgeComm(hComm, PURGE_TXCLEAR | PURGE_RXCLEAR);

    // printk("open%d ok\n", idx);

    return hComm;
}

void ClosePort(PORT com_port)
{
    CloseHandle(com_port);
}

int SetPortBoudRate(PORT com_port, int rate)
{
    DCB dcbSerialParams = {0};
    BOOL Status;
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    Status = GetCommState(com_port, &dcbSerialParams);
    if (Status == FALSE)
        return FALSE;
    dcbSerialParams.BaudRate = rate;
    Status = SetCommState(com_port, &dcbSerialParams);
    return Status;
}

int SetPortDataBits(PORT com_port, int bits)
{
    DCB dcbSerialParams = {0};
    BOOL Status;
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    Status = GetCommState(com_port, &dcbSerialParams);
    if (Status == FALSE)
        return FALSE;
    dcbSerialParams.ByteSize = bits;
    Status = SetCommState(com_port, &dcbSerialParams);
    return Status;
}

int SetPortStopBits(PORT com_port, int bits)
{
    DCB dcbSerialParams = {0};
    BOOL Status;
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    Status = GetCommState(com_port, &dcbSerialParams);
    if (Status == FALSE)
        return FALSE;
    dcbSerialParams.StopBits = bits;
    Status = SetCommState(com_port, &dcbSerialParams);
    return Status;
}

// default: NOPARITY 0； ODDPARITY 1；EVENPARITY 2；MARKPARITY 3；SPACEPARITY 4
int SetPortParity(PORT com_port, int parity)
{
    DCB dcbSerialParams = {0};
    BOOL Status;
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    Status = GetCommState(com_port, &dcbSerialParams);
    if (Status == FALSE)
        return FALSE;
    dcbSerialParams.Parity = parity;
    Status = SetCommState(com_port, &dcbSerialParams);
    return Status;
}
enum FlowControl
{
    NoFlowControl,
    CtsRtsFlowControl,
    CtsDtrFlowControl,
    DsrRtsFlowControl,
    DsrDtrFlowControl,
    XonXoffFlowControl
};

int SetPortFlowControl(PORT com_port, bool flowcontrol)
{
    DCB dcb = {0};
    BOOL Status;
    dcb.DCBlength = sizeof(dcb);
    Status = GetCommState(com_port, &dcb);
    if (Status == FALSE)
        return FALSE;
    // dcb.fOutxCtsFlow = flowcontrol;
    // dcb.fRtsControl = flowcontrol ? RTS_CONTROL_HANDSHAKE : 0;

    //流控设置
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = FALSE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;

    int fc = flowcontrol ? CtsRtsFlowControl : NoFlowControl;
    switch (fc)
    {
        //不流控
    case NoFlowControl:
    {
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        break;
    }
    //硬件CtsRts流控
    case CtsRtsFlowControl:
    {
        dcb.fOutxCtsFlow = TRUE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        break;
    }
    //硬件 CtsDtr流控
    case CtsDtrFlowControl:
    {
        dcb.fOutxCtsFlow = TRUE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        break;
    }
    //硬件DsrRts流控
    case DsrRtsFlowControl:
    {
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = TRUE;
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        break;
    }
    //硬件DsrDtr流控
    case DsrDtrFlowControl:
    {
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = TRUE;
        dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        break;
    }
    //软件流控
    case XonXoffFlowControl:
    {
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fOutX = TRUE;
        dcb.fInX = TRUE;
        dcb.XonChar = 0x11;
        dcb.XoffChar = 0x13;
        dcb.XoffLim = 100;
        dcb.XonLim = 100;
        break;
    }
    }

    Status = SetCommState(com_port, &dcb);
    return Status;
}

int GetPortBoudRate(PORT com_port)
{
    DCB dcbSerialParams = {0};
    BOOL Status;
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    Status = GetCommState(com_port, &dcbSerialParams);
    if (Status == FALSE)
        return -1;
    return dcbSerialParams.BaudRate;
}

int GetPortDataBits(PORT com_port)
{
    DCB dcbSerialParams = {0};
    BOOL Status;
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    Status = GetCommState(com_port, &dcbSerialParams);
    if (Status == FALSE)
        return -1;
    return dcbSerialParams.ByteSize;
}

int GetPortStopBits(PORT com_port)
{
    DCB dcbSerialParams = {0};
    BOOL Status;
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    Status = GetCommState(com_port, &dcbSerialParams);
    if (Status == FALSE)
        return -1;
    return dcbSerialParams.StopBits;
}

int GetPortParity(PORT com_port)
{
    DCB dcbSerialParams = {0};
    BOOL Status;
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    Status = GetCommState(com_port, &dcbSerialParams);
    if (Status == FALSE)
        return -1;
    return dcbSerialParams.Parity;
}

int SendData(PORT com_port, const char *data)
{
    DWORD dNoOFBytestoWrite = strlen(data);
    DWORD dNoOfBytesWritten;
    BOOL Status = WriteFile(com_port, data, dNoOFBytestoWrite, &dNoOfBytesWritten, NULL);
    if (Status == FALSE)
    {
        return -1;
    }
    else
    {
        printk("%s\n", data);
    }

    return dNoOfBytesWritten;
}

int ReciveData(PORT com_port, char *data, int len)
{
    DWORD dwEventMask;
    DWORD NoBytesRead;

    BOOL Status = WaitCommEvent(com_port, &dwEventMask, NULL);
    if (Status == FALSE)
    {
        return FALSE;
    }
    Status = ReadFile(com_port, data, len, &NoBytesRead, NULL);
    data[NoBytesRead] = 0;

    if (Status == FALSE)
    {
        return FALSE;
    }
    else
    {
        printk("%s\n", data);
    }

    return TRUE;
}

PORT serial_init(int idx, int rate, int databits, int stopbits, int parity, bool flowcontrol)
{
    int ret = 0;
    PORT com_port;
    com_port = OpenPort(idx, TRUE);
    if (com_port == INVALID_HANDLE_VALUE)
    {
        printk("open COM%d fail\n", idx);
        return NULL;
    }
    // 配置参数
    DCB dcb = {0};

    if (!GetCommState(com_port, &dcb))
    {
        // 获取参数失败
        return false;
    }

    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = rate;     // 波特率
    dcb.ByteSize = databits; // 数据位

    switch (parity) //校验位
    {
    case 0:
        dcb.Parity = NOPARITY; //无校验
        break;
    case 1:
        dcb.Parity = ODDPARITY; //奇校验
        break;
    case 2:
        dcb.Parity = EVENPARITY; //偶校验
        break;
    case 3:
        dcb.Parity = MARKPARITY; //标记校验
        break;
    }

    switch (stopbits) //停止位
    {
    case 1:
        dcb.StopBits = ONESTOPBIT; // 1位停止位
        break;
    case 2:
        dcb.StopBits = TWOSTOPBITS; // 2位停止位
        break;
    case 3:
        dcb.StopBits = ONE5STOPBITS; // 1.5位停止位
        break;
    }

    //流控设置
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = FALSE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;

    int fc = CtsRtsFlowControl;
    switch (fc)
    {
        //不流控
    case NoFlowControl:
    {
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        break;
    }
    //硬件CtsRts流控
    case CtsRtsFlowControl:
    {
        dcb.fOutxCtsFlow = TRUE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        break;
    }
    //硬件 CtsDtr流控
    case CtsDtrFlowControl:
    {
        dcb.fOutxCtsFlow = TRUE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        break;
    }
    //硬件DsrRts流控
    case DsrRtsFlowControl:
    {
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = TRUE;
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        break;
    }
    //硬件DsrDtr流控
    case DsrDtrFlowControl:
    {
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = TRUE;
        dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
        dcb.fOutX = FALSE;
        dcb.fInX = FALSE;
        break;
    }
    //软件流控
    case XonXoffFlowControl:
    {
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fOutX = TRUE;
        dcb.fInX = TRUE;
        dcb.XonChar = 0x11;
        dcb.XoffChar = 0x13;
        dcb.XoffLim = 100;
        dcb.XonLim = 100;
        break;
    }
    }

    if (!SetCommState(com_port, &dcb))
    {
        // 设置参数失败
        return false;
    }

    return com_port;
}

int Serial_SendData(PORT com_port, const char *data, int len)
{
    DWORD dNoOfBytesWritten;
    BOOL Status;

    if (synchronizeflag)
    {
        // sync
        Status = WriteFile(com_port, data, len, &dNoOfBytesWritten, NULL);

        if (Status == FALSE)
        {
            return -1;
        }
        else
        {
            // printk("send ok\n");
        }

        return dNoOfBytesWritten;
    }
    else
    {
        //异步方式
        DWORD dwErrorFlags;   //错误标志
        COMSTAT comStat;      //通讯状态
        OVERLAPPED m_osWrite; //异步输入输出结构体

        //创建一个用于OVERLAPPED的事件处理，不会真正用到，但系统要求这么做
        memset(&m_osWrite, 0, sizeof(m_osWrite));
        m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, "WriteEvent");

        ClearCommError(com_port, &dwErrorFlags, &comStat); //清除通讯错误，获得设备当前状态
        BOOL bWriteStat = WriteFile(com_port,              //串口句柄
                                    data,                  //数据首地址
                                    len,                   //要发送的数据字节数
                                    &dNoOfBytesWritten, // DWORD*，用来接收返回成功发送的数据字节数
                                    &m_osWrite); // NULL为同步发送，OVERLAPPED*为异步发送
        // printk("bWriteStat, %d, dNoOfBytesWritten: %d\n", bWriteStat, dNoOfBytesWritten);
        if (!bWriteStat)
        {
            if (GetLastError() == ERROR_IO_PENDING) //如果串口正在写入
            {
                WaitForSingleObject(m_osWrite.hEvent, 1000); //等待写入事件1秒钟
                dNoOfBytesWritten = len;
            }
            else
            {
                ClearCommError(com_port, &dwErrorFlags, &comStat); //清除通讯错误
                CloseHandle(m_osWrite.hEvent);                     //关闭并释放hEvent内存
                return -1;
            }
        }
        // printk("dNoOfBytesWritten, %d\n", dNoOfBytesWritten);
        return dNoOfBytesWritten;
    }

    return 0;
}

int Serial_ReciveData(PORT com_port, char *data, int len)
{
    DWORD NoBytesRead;
    // BOOL Status;
    if (synchronizeflag)
    {
        //同步方式
        BOOL bReadStat = ReadFile(com_port, //串口句柄
                                  data,     //数据首地址
                                  len,      //要读取的数据最大字节数
                                  &NoBytesRead, // DWORD*,用来接收返回成功读取的数据字节数
                                  NULL);        // NULL为同步发送，OVERLAPPED*为异步发送
    }
    else
    {
        //异步方式
        DWORD dwErrorFlags;  //错误标志
        COMSTAT comStat;     //通讯状态
        OVERLAPPED m_osRead; //异步输入输出结构体

        //创建一个用于OVERLAPPED的事件处理，不会真正用到，但系统要求这么做
        memset(&m_osRead, 0, sizeof(m_osRead));
        m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, "ReadEvent");

        ClearCommError(com_port, &dwErrorFlags, &comStat); //清除通讯错误，获得设备当前状态
        // if (!comStat.cbInQue)
        //  return 0;  //如果输入缓冲区字节数为0，则返回false

        // std::cout << comStat.cbInQue << std::endl;
        BOOL bReadStat = ReadFile(com_port, //串口句柄
                                  data,     //数据首地址
                                  len,      //要读取的数据最大字节数
                                  &NoBytesRead, // DWORD*,用来接收返回成功读取的数据字节数
                                  &m_osRead); // NULL为同步发送，OVERLAPPED*为异步发送
        if (!bReadStat)
        {
            if (GetLastError() == ERROR_IO_PENDING) //如果串口正在读取中
            {
                // GetOverlappedResult函数的最后一个参数设为TRUE
                //函数会一直等待，直到读操作完成或由于错误而返回
                GetOverlappedResult(com_port, &m_osRead, &NoBytesRead, TRUE);
            }
            else
            {
                ClearCommError(com_port, &dwErrorFlags, &comStat); //清除通讯错误
                CloseHandle(m_osRead.hEvent);                      //关闭并释放hEvent的内存
                return -1;
            }
        }
    }

    // printk("rcv ok, %ld\n", NoBytesRead);

    return NoBytesRead;
}

int Serial_AsyncSendData(PORT com_port, const char *data, int len)
{
    DWORD dNoOfBytesWritten;
    BOOL Status = WriteFile(com_port, data, len, &dNoOfBytesWritten, NULL);

    if (Status == FALSE)
        return -1;
    else
        // printk("send ok\n");

        return 0;
}

int Serial_AsyncReciveData(PORT com_port, char *data, int len)
{
    // DWORD dwEventMask;
    DWORD NoBytesRead;
    BOOL Status;

    // BOOL Status = WaitCommEvent(com_port, &dwEventMask, NULL);
    // if (Status == FALSE)
    // {
    // 	return -1;
    // }
    Status = ReadFile(com_port, data, len, &NoBytesRead, NULL);
    data[NoBytesRead] = 0;

    if (Status == FALSE)
        return -1;
    else
        printk("rcv ok\n");

    return NoBytesRead;
}

static bool is_enable;

static struct k_fifo tx_queue;
static pthread_mutex_t tx_lock;
static struct k_fifo rx_queue;
static pthread_mutex_t rx_lock;

static PORT serial;

static struct net_buf *pop_tx_queue(void)
{
    pthread_mutex_lock(&tx_lock);
    struct net_buf *buf = net_buf_get(&tx_queue, Z_FOREVER);
    pthread_mutex_unlock(&tx_lock);

    return buf;
}

static void push_tx_queue(struct net_buf *buf)
{
    pthread_mutex_lock(&tx_lock);
    net_buf_put(&tx_queue, buf);
    pthread_mutex_unlock(&tx_lock);
}

static struct net_buf *pop_rx_queue(void)
{
    pthread_mutex_lock(&rx_lock);
    struct net_buf *buf = net_buf_get(&rx_queue, Z_FOREVER);
    pthread_mutex_unlock(&rx_lock);

    return buf;
}

static void push_rx_queue(struct net_buf *buf)
{
    pthread_mutex_lock(&rx_lock);
    net_buf_put(&rx_queue, buf);
    pthread_mutex_unlock(&rx_lock);
}

pthread_t serial_tx_thread;
static int tx_process_loop(void *args)
{
    printk("tx_process_loop\n");
    struct net_buf *buf;
    int ret;

    while (1)
    {
        if (!k_fifo_is_empty(&tx_queue))
        {
            buf = pop_tx_queue();

            net_buf_push_u8(buf, bt_get_h4_type_by_buffer(bt_buf_get_type(buf)));
            ret = Serial_SendData(serial, (char *)buf->data, buf->len);

            if (ret < 0)
            {
                printk("error tx\n");
            }
            else
            {
                // printk("success: tx %d bytes\n", ret);
                // printk("data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf->data[0], buf->data[1],
                //        buf->data[2], buf->data[3], buf->data[4], buf->data[5]);
            }

            net_buf_unref(buf);
        }

        if (!is_enable)
        {
            break;
        }
    }
    return 0;
}

pthread_t serial_rx_thread;
static int rx_process_loop(void *args)
{
    printk("rx_process_loop\n");
    char tmp[1024];
    int ret;
    while (1)
    {
        ret = Serial_ReciveData(serial, tmp, sizeof(tmp));
        if (ret <= 0)
        {
            // printk("error reading.\n");
            Sleep(1000);
        }
        else
        {
            // printk("success: serial read %d bytes\n", ret);
            // printk("data: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", tmp[0], tmp[1], tmp[2], tmp[3],
            //        tmp[4], tmp[5]);

            struct net_buf *buf;
            buf = bt_buf_get_controller_tx_evt();

            if (buf)
            {
                net_buf_add_mem(buf, tmp, ret);
                bt_recv(buf);
            }
            else
            {
                printk("hci_driver_polling_work(), no reserve buff\n");
                while (1)
                    ;
            }
        }

        if (!is_enable)
        {
            break;
        }
    }

    return 0;
}

static int hci_driver_open(void)
{
    return 0;
}

static int hci_driver_send(struct net_buf *buf)
{
    push_tx_queue(buf);

    return 0;
}

static const struct bt_hci_driver drv = {
        .open = hci_driver_open,
        .send = hci_driver_send,
};

static void hci_driver_init(void)
{
    bt_hci_driver_register(&drv);
}

static int hci_driver_h4_open(void)
{
    return 0;
}

static int hci_driver_h4_send(uint8_t *buf, uint16_t len)
{
    return Serial_SendData(serial, (char *)buf, len);
}

static int hci_driver_h4_recv(uint8_t *buf, uint16_t len)
{
    return Serial_ReciveData(serial, (char *)buf, len);
}

static const struct bt_hci_h4_driver h4_drv = {
        .open = hci_driver_h4_open,
        .send = hci_driver_h4_send,
        .recv = hci_driver_h4_recv,
};

static void hci_driver_h4_init(void)
{
    hci_h4_init(&h4_drv);
}

int serial_open_process(int idx, int rate, int databits, int stopbits, int parity, bool flowcontrol)
{
    printk("serial_open_process idx: %d, rate: %d, databits: %d, stopbits: %d, parity: %d, "
           "flowcontrol: %d\n",
           idx, rate, databits, stopbits, parity, flowcontrol);
    serial = serial_init(idx, rate, databits, stopbits, parity, flowcontrol);

    is_enable = true;

    // pthread_mutex_init(&tx_lock, NULL);
    // pthread_mutex_init(&rx_lock, NULL);

    // pthread_create(&serial_tx_thread, NULL, (void*)tx_process_loop,NULL);
    // //pthread_join(serial_tx_thread, NULL);

    // pthread_create(&serial_rx_thread, NULL, (void*)rx_process_loop,NULL);
    // //pthread_join(serial_rx_thread, NULL);

    // sys_slist_init(&tx_queue);
    // sys_slist_init(&rx_queue);

    return 0;
}

int bt_hci_init_serial_device(int idx, int rate, int databits, int stopbits, int parity, bool flowcontrol)
{
    int ret = serial_open_process(idx, rate, databits, stopbits, parity, flowcontrol);
    if (ret < 0)
    {
        return ret;
    }

    hci_driver_h4_init();

    return (0);
}
