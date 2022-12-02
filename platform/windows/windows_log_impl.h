
#ifndef _WINDOWS_LOG_IMPL_H_
#define _WINDOWS_LOG_IMPL_H_

#include "logging\bt_log_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

const bt_log_impl_t *bt_log_impl_local_instance(void);

#ifdef __cplusplus
}
#endif

#endif //_WINDOWS_LOG_IMPL_H_