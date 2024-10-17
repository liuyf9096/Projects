#include "DError.h"

#include <QSerialPort>

QString DError::getErrorMessage(ErrorType code)
{
#if 0
    if (code == Parse_Error) {
        return "语法解析错误,服务端接收到无效的json。该错误发送于服务器尝试解析json文本";
    } else if (code == Invalid_Request) {
        return "找不到方法,该方法不存在或无效";
    } else if (code == Method_Not_Found) {
        return "无效的参数,无效的方法参数";
    } else if (code == Invalid_Params) {
        return "内部错误,JSON-RPC内部错误";
    } else if (code == Internal_Error) {
        return "服务端错误,预留用于自定义的服务器错误";
    }
#endif

    if (code == ERROR_INVALID_PORTNAME) {
        return "no portname, please specify an available portname.";
    }
    if (code == ERROR_INVALID_DEVICE) {
        return "invalid device, please check your port.";
    }
    if (code == ERROR_INVALID_COMMAND) {
        return "invalid command api, for more information in help document.";
    }
    if (code == ERROR_INVALID_PARAMS) {
        return "invalid params, for more information in help document.";
    }

    //![SerialPortError]
    if (code == DeviceNotFoundError) {
        return "An error occurred while attempting to open an non-existing device.";
    } else if (code == PermissionError) {
        return "An error occurred while attempting to open an already opened device by another process " \
               "or a user not having enough permission and credentials to open.";
    } else if (code == OpenError) {
        return "An error occurred while attempting to open an already opened device in this object.";
    } else if (code == ParityError) {
        return "Parity error detected by the hardware while reading data. This value is obsolete. " \
               "We strongly advise against using it in new code.";
    } else if (code == FramingError) {
        return "Framing error detected by the hardware while reading data. This value is obsolete. " \
               "We strongly advise against using it in new code.";
    } else if (code == BreakConditionError) {
        return "Break condition detected by the hardware on the input line. This value is obsolete. " \
               "We strongly advise against using it in new code.";
    } else if (code == WriteError) {
        return "An I/O error occurred while writing the data.";
    } else if (code == ReadError) {
        return "An I/O error occurred while reading the data.";
    } else if (code == ResourceError) {
        return "An I/O error occurred when a resource becomes unavailable, e.g. " \
               "when the device is unexpectedly removed from the system.";
    } else if (code == UnsupportedOperationError) {
        return "The requested device operation is not supported or prohibited by the running operating system.";
    } else if (code == UnknownError) {
        return "An unidentified error occurred.";
    } else if (code == TimeoutError) {
        return "A timeout error occurred.";
    } else if (code == NotOpenError) {
        return "his error occurs when an operation is executed that can only be successfully performed if the device is open.";
    }


    if (code == ERROR_INVALID_REQUEST) {
        return "invalid request.";
    } else if (code == ERROR_INVALID_PLUGIN) {
        return "invalid plugin.";
    } else if (code == ERROR_INVALID_METHOD) {
        return "invalid method.";
    } else if (code == ERROR_API_BUSY) {
        return "Dobotlink api is buzy.";
    }

    if (code == ERROR_DEVICE_LOST_CONNECTION) {
        return "device lost its connection.";
    } else if (code == ERROR_COMMUNICATION_TIMEOUT) {
        return "communication timeout.";
    } else if (code == ERROR_COMMUNICATION_BUFFER_FULL) {
        return "command buffer full.";
    }

    if (code == ERROR_DEVICE_ACTION_TIMEOUT) {
        return "action timeout.";
    } else if (code == ERROR_DEVICE_ACTION_CANCELED) {
        return "action canceled.";
    }

    return "";
}
