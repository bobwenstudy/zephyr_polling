#ifndef _ZEPHYR_POLLING_DRIVERS_CHIPSET_H_
#define _ZEPHYR_POLLING_DRIVERS_CHIPSET_H_

#include "bt_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    /**
     * chipset driver name
     */
    const char *name;

    void (*polling_work)(void);

    void (*process_start)(void);

} chipset_t;

#ifdef __cplusplus
}
#endif

#endif /* _ZEPHYR_POLLING_DRIVERS_CHIPSET_H_ */