#ifndef _ZEPHYR_POLLING_UTILS_MQUEUE_SLIST_H_
#define _ZEPHYR_POLLING_UTILS_MQUEUE_SLIST_H_

struct _mqueue_slist_node
{
    struct _mqueue_slist_node *next;
};

typedef struct _mqueue_slist_node mqueue_snode_t;

struct _mqueue_slist
{
    mqueue_snode_t *head;
    mqueue_snode_t *tail;
};

typedef struct _mqueue_slist mqueue_slist_t;

/**
 * @brief Initialize a list
 *
 * @param list A pointer on the list to initialize
 */
static inline void mqueue_slist_init(mqueue_slist_t *list)
{
    list->head = NULL;
    list->tail = NULL;
}

static inline uint8_t mqueue_slist_is_empty(mqueue_slist_t *list)
{
    return list->head == NULL;
}

static inline void mqueue_slist_enqueue(mqueue_slist_t *list, mqueue_snode_t *node)
{
    node->next = NULL;
    if (list->tail == NULL)
    {
        list->head = node;
    }
    else
    {
        list->tail->next = node;
    }
    list->tail = node;
}
static inline mqueue_snode_t *mqueue_slist_dequeue(mqueue_slist_t *list)
{
    if (mqueue_slist_is_empty(list))
        return NULL;
    mqueue_snode_t *node = list->head;
    list->head = list->head->next;
    // check tail
    if (list->tail == node)
        list->tail = list->head;

    return node;
}
static inline mqueue_snode_t *mqueue_slist_peek(mqueue_slist_t *list)
{
    if (mqueue_slist_is_empty(list))
        return NULL;

    return list->head;
}

#endif /* _ZEPHYR_POLLING_UTILS_MQUEUE_SLIST_H_ */