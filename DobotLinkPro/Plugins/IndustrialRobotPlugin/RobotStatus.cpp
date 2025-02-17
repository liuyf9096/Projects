#include "RobotStatus.h"

#include <QDebug>

RobotStatus::RobotStatus()
{
    setControlMode(ENABLE);
    setCoordinateMode(JOINT);
    setJogMode(JOG);

    toolCoordinate = 0;
    userCoordinate = 0;

    alarmClear = false;
    hardware = true;
    outputEnable = false;

    for (int i = 0; i < 8; ++i) {
        outputsArray << 0;
    }

    _updateStatusMap();
}

void RobotStatus::_updateStatusMap(QString key)
{
    if (key.isEmpty()) {
        m_dataMap.insert("controlMode", controlMode);
        m_dataMap.insert("coordinate", coordinateMode);
        m_dataMap.insert("jogMode", jogMode);
        m_dataMap.insert("autoManual", autoManualMode);
        m_dataMap.insert("toolCoordinate", toolCoordinate);
        m_dataMap.insert("userCoordinate", userCoordinate);
        m_dataMap.insert("alarms", alarmsArray);
        m_dataMap.insert("outputs", outputsArray);
        m_dataMap.insert("inputs", inputsArray);
        m_dataMap.insert("jointCoordinate", jointCoordinateArray);
        m_dataMap.insert("cartesianCoordinate", cartesianCoordinateArray);
    } else if (key == "controlMode") {
        m_dataMap.insert("controlMode", controlMode);
    } else if (key == "coordinate") {
        m_dataMap.insert("coordinate", coordinateMode);
    } else if (key == "jogMode") {
        m_dataMap.insert("jogMode", jogMode);
    } else if (key == "autoManual") {
        m_dataMap.insert("autoManual", autoManualMode);
    } else if (key == "toolCoordinate") {
        m_dataMap.insert("toolCoordinate", toolCoordinate);
    } else if (key == "userCoordinate") {
        m_dataMap.insert("userCoordinate", userCoordinate);
    } else if (key == "alarms") {
        m_dataMap.insert("alarms", alarmsArray);
    } else if (key == "outputEnable") {
        m_dataMap.insert("outputEnable", outputsArray);
    } else if (key == "inputs") {
        m_dataMap.insert("inputs", inputsArray);
    } else if (key == "jointCoordinate") {
        m_dataMap.insert("jointCoordinate", jointCoordinateArray);
    } else if (key == "cartesianCoordinate") {
        m_dataMap.insert("cartesianCoordinate", cartesianCoordinateArray);
    } else {
        qDebug() << __FUNCTION__ << "error. the key does not mathch:" << key;
    }
}

/**
SEND:
{
    "controlMode": "disable" / "enable" / "drag",
    "coordinate": "joint" / "cartesian",
    "jogMode": "jog" / "step",
    "toolCoordinate": 0~9,
    "userCoordinate": 0~9,
    "alarms": true / false,
    "outputs": {
        "enable": true,
        "value": [11, 45, 36, 30, 11, 45, 36, 30]
    },
    "hardware": true
}
**/
QJsonObject RobotStatus::getObjForSend() const
{
    QJsonObject obj;
    obj.insert("controlMode", controlMode);
    obj.insert("coordinate", coordinateMode);
    obj.insert("jogMode", jogMode);
    obj.insert("toolCoordinate", toolCoordinate);
    obj.insert("userCoordinate", userCoordinate);
    obj.insert("alarms", alarmClear);
    obj.insert("hardware", hardware);

    QJsonObject outputsObj;
    outputsObj.insert("enable", outputEnable);
    outputsObj.insert("value", outputsArray);

    obj.insert("outputs", outputsObj);

    return obj;
}

/**
RECEIVE:
{
    "controlMode": "disable" / "enable" / "drag",
    "coordinate": "joint" / "cartesian",
    "jogMode": "jog" / "step",
    "toolCoordinate": 0~9,
    "userCoordinate": 0~9,
    "autoManual"
    "auto" / "manual",
    "jointCoordinate": [j1, j2, j3, j4, j5, j6],
    "cartesianCoordinate": [x, y, z, a, b, c],
    "alarms": [
        [id, id, id],
        [id],
        [id],
        [id],
        [id],
        [id],
        [id]
    ],
    "outputs": [11, 45, 36, 30, 11, 45, 36, 30],
    "inputs": [11, 45, 36, 30, 11, 45, 127, 255]
}
**/

void RobotStatus::setStatus(QJsonObject obj)
{
    alarmClear = false;

    if (obj.contains("controlMode")) {
        controlMode = obj.value("controlMode").toString();
    }
    if (obj.contains("coordinate")) {
        coordinateMode = obj.value("coordinate").toString();
    }
    if (obj.contains("jogMode")) {
        jogMode = obj.value("jogMode").toString();
    }
    if (obj.contains("toolCoordinate")) {
        toolCoordinate = obj.value("toolCoordinate").toInt();
    }
    if (obj.contains("userCoordinate")) {
        userCoordinate = obj.value("userCoordinate").toInt();
    }
    if (obj.contains("autoManual")) {
        autoManualMode = obj.value("autoManual").toString();
    }
    if (obj.contains("armOrientation")) {
        armOrientation = obj.value("armOrientation").toInt();
    }
    if (obj.contains("jointCoordinate")) {
        jointCoordinateArray = obj.value("jointCoordinate").toArray();
    } 
    if (obj.contains("cartesianCoordinate")) {
        cartesianCoordinateArray = obj.value("cartesianCoordinate").toArray();
    }
    if (obj.contains("alarms")) {
        alarmsArray = obj.value("alarms").toArray();
    }
    if (obj.contains("outputs")) {
        outputsArray = obj.value("outputs").toArray();
    }
    if (obj.contains("inputs")) {
        inputsArray = obj.value("inputs").toArray();
    }

    _updateStatusMap();
}

QVariant RobotStatus::getStatus(QString key)
{
    return m_dataMap.value(key);
}

void RobotStatus::setControlMode(RobotStatus::ControlMode mode)
{
    switch (mode) {
    case DISABLE:
        controlMode = "disable";
        break;
    case ENABLE:
        controlMode = "enable";
        break;
    case DRAG:
        controlMode = "drag";
        break;
    }

    _updateStatusMap("controlMode");
}

void RobotStatus::setControlMode(int mode)
{
    if (mode == 0) {
        setControlMode(DISABLE);
    } else if (mode == 1) {
        setControlMode(ENABLE);
    } else if (mode == 2) {
        setControlMode(DRAG);
    }
}

void RobotStatus::setControlMode(QString mode)
{
    if (mode == "disable") {
        setControlMode(DISABLE);
    } else if (mode == "enable") {
        setControlMode(ENABLE);
    } else if (mode == "drag") {
        setControlMode(DRAG);
    } else {
        qDebug() << __FUNCTION__ << "error mode:" << mode;
    }
}

void RobotStatus::setCoordinateMode(CoordinateType mode)
{
    switch (mode) {
    case JOINT:
        coordinateMode = "joint";
        break;
    case CARTESIAN:
        coordinateMode = "cartesian";
        break;
    }

    _updateStatusMap("coordinate");
}

void RobotStatus::setCoordinateMode(int mode)
{
    if (mode == 0) {
        setCoordinateMode(JOINT);
    } else if (mode == 1) {
        setCoordinateMode(CARTESIAN);
    }
}

void RobotStatus::setCoordinateMode(QString mode)
{
    if (mode == "joint") {
        setCoordinateMode(JOINT);
    } else if (mode == "cartesian") {
        setCoordinateMode(CARTESIAN);
    } else {
        qDebug() << __FUNCTION__ << "error mode:" << mode;
    }
}

void RobotStatus::setJogMode(RobotStatus::JogMode mode)
{
    switch (mode) {
    case JOG:
        jogMode = "jog";
        break;
    case STEP:
        jogMode = "step";
        break;
    }

    _updateStatusMap("jogMode");
}

void RobotStatus::setJogMode(int mode)
{
    if (mode == 0) {
        setJogMode(JOG);
    } else if (mode == 1) {
        setJogMode(STEP);
    }
}

void RobotStatus::setJogMode(QString mode)
{
    if (mode == "jog") {
        setJogMode(JOG);
    } else if (mode == "step") {
        setJogMode(STEP);
    } else {
        qDebug() << __FUNCTION__ << "error mode:" << mode;
    }
}

void RobotStatus::setToolCoordPattern(int pattern)
{
    if (pattern >= 0 and pattern <= 9) {
        toolCoordinate = pattern;

        _updateStatusMap("toolCoordinate");
    }
}

void RobotStatus::setUserCoordPattern(int pattern)
{
    if (pattern >= 0 and pattern <= 9) {
        userCoordinate = pattern;

        _updateStatusMap("userCoordinate");
    }
}

void RobotStatus::setAutoManualMode(AutoManualMode mode)
{
    switch (mode) {
    case AUTO:
        autoManualMode = "auto";
        break;
    case MANUAL:
        autoManualMode = "manual";
        break;
    }

    _updateStatusMap("autoManual");
}

void RobotStatus::setAutoManualMode(int mode)
{
    if (mode == 0) {
        setAutoManualMode(AUTO);
    } else if (mode == 1) {
        setAutoManualMode(MANUAL);
    }
}

void RobotStatus::setAutoManualMode(QString mode)
{
    if (mode == "auto") {
        setAutoManualMode(AUTO);
    } else if (mode == "manual") {
        setAutoManualMode(MANUAL);
    } else {
        qDebug() << __FUNCTION__ << "error mode:" << mode;
    }
}

void RobotStatus::clearAlarms()
{
    alarmClear = true;
}

void RobotStatus::setOutputs(bool en, QJsonArray outputs)
{
    outputEnable = en;
    outputsArray = outputs;
}


