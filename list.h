/*
 * list
 */

#ifndef _LIST_H_
#define _LIST_H_

#include <stdio.h>

#define offsetof(TYPE, MEMBER)  ((size_t)&((TYPE *)0)->MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({          \
    void *__mptr = (void *)(ptr);                   \
    ((type *)(__mptr - offsetof(type, member))); })

/**
 * list_entry - get the struct for this entry
 * @ptr:    the &struct list_head pointer.
 * @type:   the type of the struct this is embedded in.
 * @member: the name of the list_head within the struct.
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    list_head name = LIST_HEAD_INIT(name)

typedef struct _list_head {
    struct _list_head *prev;
    struct _list_head *next;
} list_head;

static inline void
INIT_LIST_HEAD(list_head *list)
{
    list->next = list;
    list->prev = list;
}

static inline void
__list_add(list_head *new,
           list_head *prev,
           list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void
list_add(list_head *new,
         list_head *head)
{
    __list_add(new, head, head->next);
}

static inline void
list_add_tail(list_head *new,
              list_head *head)
{
    __list_add(new, head->prev, head);
}

static inline void
__list_del(list_head * prev, list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void
__list_del_entry(list_head *entry)
{
    __list_del(entry->prev, entry->next);
}

static inline void
list_del(list_head *entry)
{
    __list_del_entry(entry);
    entry->next = NULL;
    entry->prev = NULL;
}

static inline int
list_empty(const list_head *head)
{
    return (head->next == head);
}

#endif /* _TRACE_H_ */
