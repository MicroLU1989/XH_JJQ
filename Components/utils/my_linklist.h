/**************************************************************************
 * @FilePath     : /nc-link-v2-adapter/components/utils/linklist.h
 * @Description  :  
 * @Author       : MicroLU1989 916739459@qq.com
 * @Version      : 0.0.1
 * @LastEditors  : MicroLU1989 916739459@qq.com
 * @LastEditTime : 2024-12-19 11:55:52
 * @Copyright    : 
***************************************************************************/
#ifndef _MY_ILINKLIST_H_
#define _MY_ILINKLIST_H_


typedef struct linklist {
    struct linklist *next;  // 后继节点指针
    struct linklist *prev;  // 新增前驱节点指针
    void *data;
} linklist_t;

void linklist_insert_before(linklist_t *target, linklist_t *new_node);
void linklist_insert_after(linklist_t *target, linklist_t *new_node);

linklist_t *linklist_create(void);
void linklist_insert(linklist_t *header, linklist_t *node);
void linklist_destroy(linklist_t *header);
void linklist_delete(linklist_t *header, linklist_t *node);

#endif