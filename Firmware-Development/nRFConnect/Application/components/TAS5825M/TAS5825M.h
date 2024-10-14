#ifndef _TAS5825M_H_
#define _TAS5825M_H_

#include "DeviceStatus.hpp"
#ifdef __cplusplus
extern "C" {
#endif

// class TAS5825M
// {
//     public:
        DeviceStatus TAS5825M_init(void);
        DeviceStatus TAS5825M_loop(void);

// };
#ifdef __cplusplus
}
#endif

#endif // _TAS5825M_H_