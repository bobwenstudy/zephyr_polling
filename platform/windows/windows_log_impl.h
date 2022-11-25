
#ifndef _WINDOWS_LOG_IMPL_H_
#define _WINDOWS_LOG_IMPL_H_

#include "logging\log_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

const log_impl_t *log_impl_windows_instance(void);

#ifdef __cplusplus
}
#endif

#endif //_WINDOWS_LOG_IMPL_H_