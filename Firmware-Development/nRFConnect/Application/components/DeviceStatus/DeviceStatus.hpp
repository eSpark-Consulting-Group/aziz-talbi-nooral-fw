#ifndef _DEVICE_STATUS_H_
#define _DEVICE_STATUS_H_

#define ENUM_TO_STRING(ENUM) #ENUM

typedef enum 
{
    #define STATUS_DEFINE(status) status,
    #include "DeviceStatusHelper.hpp"
} DeviceStatus;

typedef struct
{
    DeviceStatus status;
    const char* name;
} DeviceStatusMapping;

#endif // _DEVICE_STATUS_H_