#include "base\byteorder.h"

#include <stdio.h>

#include <logging/log_impl.h>
#include <pthread.h>
#include <windows.h>

static pthread_mutex_t irq_lock;
unsigned int arch_irq_lock(void)
{
    unsigned int key = 0;

    printk("arch_irq_lock()\n");

    pthread_mutex_lock(&irq_lock);

    return key;
}

void arch_irq_unlock(unsigned int key)
{
    printk("arch_irq_unlock()\n");

    pthread_mutex_unlock(&irq_lock);
}

void irq_lock_impl_windows_init(void)
{
    pthread_mutex_init(&irq_lock, NULL);
}
