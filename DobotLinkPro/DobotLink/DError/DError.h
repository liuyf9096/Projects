#ifndef DERROR_H
#define DERROR_H

#include <QObject>

/**
    | 1~99    | DobotLink 错误码                        |
    | 100~199 | MagicDevice 错误码                      |
    | 200~299 | Arduino/AIStarter/MobilePlatform 错误码 |
    | 300~399 | Microbit 错误码                         |
*/

enum ErrorType {
    Parse_Error = -32700,
    Invalid_Request = -32600,
    Method_Not_Found = -32601,
    Invalid_Params = -32602,
    Internal_Error = -32603,

    NoError = 0,

    /* 1~99 DobotLink 错误码 */
    ERROR_INVALID_REQUEST = 1,
    ERROR_INVALID_PLUGIN,
    ERROR_INVALID_METHOD,
    ERROR_API_BUSY,

    /* 100~199 MagicDevicePlugin 错误码 */
    ERROR_INVALID_PORTNAME = 100,
    ERROR_INVALID_PARAMS,
    ERROR_INVALID_COMMAND,
    ERROR_INVALID_DEVICE,

    ERROR_DEVICE_NOT_FOUND = 110,
    ERROR_DEVICE_OCCUPIED,
    ERROR_DEVICE_ALREADY_CONNECTED,
    ERROR_DEVICE_NOT_CONNECTED,
    ERROR_DEVICE_UNKNOWN_ERROR,

    //![MagicDevice.dll 错误码]
    MAGIC_DEVICE_DLL_BASE = 120,

    /* 1~13 SerialPortError */
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

    ERROR_DEVICE_LOST_CONNECTION = MAGIC_DEVICE_DLL_BASE + 20,

    ERROR_COMMUNICATION_TIMEOUT = MAGIC_DEVICE_DLL_BASE + 30,
    ERROR_COMMUNICATION_BUFFER_FULL,

    ERROR_DEVICE_ACTION_TIMEOUT = MAGIC_DEVICE_DLL_BASE + 40,
    ERROR_DEVICE_ACTION_CANCELED = MAGIC_DEVICE_DLL_BASE + 41,

    ERROR_INVALID_ARDUINO_PROGRAM = 200,
    ERROR_INVALID_ARDUINO_TYPE,
    ERROR_INVALID_ARDUINO_TOOLCHAIN,
    ERROR_NUKNOWN_ERROR
};

class DError
{
public:
    static QString getErrorMessage(ErrorType code);
};

/*

|  180  | caution: Magician device disconnected. |

|  182  | Magician Mathod is invalid.            |

|  201  | ArduinoProgram's data is empty.        |
|  202  | Arduino Type error. type:xxx           |
|  203  | serial port is empty.                  |
|  204  | Open code file Failed. filePath:xxx    |
|  205  | ArduinoProgram is buzy.                |
|  206  | error, timeout.                        |
|  207  | error, please read log.                |

|  210  | Arduino-tools not found.               |
|  211  | code.cpp file is not exist.            |
|  212  | compile process crash.                 |
|  213  | avr-g++ compile Error.                 |
|  214  | avr-gcc compile Error.                 |
|  215  | avr-objcopy compile Error.             |

|  220  | out.hex file is not exist.             |
|  221  | no available device.                   |
|  222  | Do not specify Arduino Type.           |
|  223  | upload process crash.                  |
|  224  | upload process timeout.                |
|  225  | unknown error, please read log.        |


*/

#endif // DERROR_H
