/*******************************************************************
*  Copyright(c) 2016 Chen Gonghao
*  All rights reserved.
*
*  chengonghao@yeah.net
******************************************************************/

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "condition.h"

/* 线程控制块（task control block），以单向链表的形式组织 TCB */
typedef struct task
{
   void *( *run ) ( void* arg ) ; // 线程的执行函数
   void *arg ; // 执行函数的参数
   struct tast* next ; // 指向下一个 TCB
} task_t ;

/* 线程池结构体 */
typedef struct threadpool
{
   condition_t ready ; // 条件变量结构体
   task_t* first ; // TCB 链表的头指针
   task_t* last ; // TCB 链表的尾指针
   int counter ; // TCB 的总数量
   int idle ; // 空闲 TCB 的个数
   int max_threads ; // 最大线程数量
   int quit ; // 线程池销毁标志
} threadpool_t ;

/* 初始化线程池 */
void threadpool_init( threadpool_t* pool, int threads ) ;

/* 向线程池中添加线程 */ 
void threadpool_add_task( threadpool_t* pool, void* ( *run )( void* arg ), void* arg ) ;

/* 销毁线程池 */
void threadpool_destroy( threadpool_t* pool ) ;

#endif
