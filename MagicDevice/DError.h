#ifndef DERROR_H
#define DERROR_H

enum DeviceErrorType {
    NoError = 0,

    /* 1~13 为 SerialPortError 错误预留 */
    DeviceNotFoundError,
    PermissionError,
    OpenError,
    ParityError,
    FramingError,
    BreakConditionError,
    WriteError,
    ReadError,
    ResourceError,
    UnsupportedOperationError,
    UnknownError,
    TimeoutError,
    NotOpenError,

    /* 连接意外断开 心跳检测报错 */
    DeviceLostConnectionError = 20,

    /* 通讯超时 */
    CommunicationTimeoutError = 30,
    CommunicationBufferFullError,

    /* 运动超时 */
    DeviceActionTimeoutError = 40,

    /* 紧急停止 */
    DeviceActionCanceled = 41
};

#endif // DERROR_H
