/*******************************************************************
*  Copyright(c) 2016 Chen Gonghao
*  All rights reserved.
*
*  chengonghao@yeah.net
******************************************************************/

#ifndef _CONDITION_H_
#define _CONDITION_H_

#include <pthread.h>

/* 将互斥锁和条件变量封装成一个结构体 */
typedef struct condition
{
   pthread_mutex_t pmutex ;
   pthread_cond_t pcond ;
} condition_t ;

/* 初始化结构体 */
int condition_init( condition_t* cond ) ;

/* 拿到结构体中的互斥锁 */
int condition_lock( condition_t* cond ) ;

/* 释放结构体中的互斥锁 */
int condition_unlock( condition_t* cond ) ;

/* 使消费者线程等待在条件变量上 */
int condition_wait( condition_t* cond ) ;

/* 使消费者线程等待在条件变量上，abstime：等待超时 */
int condition_timedwait( condition_t* cond, const struct timespec* abstime ) ;

/* 生产者线程通知等待在条件变量上的消费者线程 */
int condition_signal( condition_t* cond ) ;

/* 生产者线程向等待在条件变量上的消费者线程广播 */
int condition_broadcast( condition_t* cond ) ;

/* 销毁结构体 */
int condition_destroy( condition_t* cond ) ;

#endif
