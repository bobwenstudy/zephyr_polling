
#ifndef _WINDOWS_STORAGE_KV_IMPL_H_
#define _WINDOWS_STORAGE_KV_IMPL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/storage_kv.h"

const struct storage_kv_impl *storage_kv_impl_windows_instance(void);

#ifdef __cplusplus
}
#endif

#endif //_WINDOWS_STORAGE_KV_IMPL_H_