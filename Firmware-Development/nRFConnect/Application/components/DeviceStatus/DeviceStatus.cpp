#include <zephyr/kernel.h>
#include "DeviceStatus.hpp"

static DeviceStatusMapping map[] = 
{
    #define STATUS_DEFINE(s) {.status = s, .name = ENUM_TO_STRING(s)},
    #include "DeviceStatusHelper.hpp"
};

#define MAX_STATUS (sizeof(map) / sizeof(map[0]))

const char* deviceErrorToName(DeviceStatus status)
{
    if (status < MAX_STATUS)
    {
        return map[status].name;
    }
    
    return NULL;
}