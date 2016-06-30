/*******************************************************************
*  Copyright(c) 2016 Chen Gonghao
*  All rights reserved.
*
*  chengonghao@yeah.net
******************************************************************/

#include "threadpool.h"
#include <errno.h>
#include <time.h>

/* 线程入口函数 */
void* thread_runtime( void* arg ) 
{
   struct timespec abstime ;
   int timeout ;
   
   // 拿到线程池对象
   threadpool_t* pool = ( threadpool_t* ) arg ;

   while ( 1 ) 
   {
      timeout = 0 ;

      /************************** 进入临界区 ***********************/

      condition_lock( &pool->ready ) ;
      
      // 空闲线程数加 1
      ++ pool->idle ;

      // 如果线程链表为空，而且线程池处于运行状态，那么线程就该等待任务的到来
      while ( pool->first == NULL && pool->quit == 0 )
      {
         printf( "thread 0x%x is waiting\n", (int)pthread_self() ) ;
         clock_gettime( CLOCK_REALTIME, &abstime ) ;
         abstime.tv_sec += 2 ;
         int status = condition_timedwait( &pool->ready, &abstime ) ;
         if ( status == ETIMEDOUT )
         {
            printf( "thread 0x%x is wait timed out\n", (int)pthread_self() ) ;
            timeout = 1 ;
            break ;
         }
      }

      // 如果线程等待超时
      if ( timeout && pool->first == NULL )
      {
         -- pool->counter ; // 那么线程数量减 1
         condition_unlock( &pool->ready ) ; // 释放互斥锁
         break ; // 跳出 while，注意，break 之后，线程入口函数执行完毕，线程将不复存在
      }

      // 线程获得任务
      -- pool->idle ; // 线程池空闲数减 1
      if ( pool->first != NULL )
      {
         task_t* t = pool->first ; // 从链表头部取出 TCB
         pool->first = t->next ; // 指向下一个 TCB

         // 执行任务需要一定时间，所以要先解锁
         // 以便生产者可以往链表中加入任务
         // 以及其他消费者可以等待任务
         condition_unlock( &pool->ready ) ;

         t->run( t->arg ) ; // 执行任务的回调函数

         free( t ) ; // 任务执行完毕，销毁 TCB

         condition_lock( &pool->ready ) ;
      } 

      // quit == 1 说明要销毁线程池
      if ( pool->quit && pool->first == NULL )
      {
         -- pool->counter ;
         if ( pool->counter == 0 ) 
         {
            condition_signal( &pool->ready ) ; // 唤醒等待在条件变量上的主线程
         }
         condition_unlock( &pool->ready ) ;
         break ;
      }

      condition_unlock( &pool->ready ) ;

      /************************** 退出临界区 ***********************/
   }

   return NULL ;
}

/* 初始化线程池 */
void threadpool_init( threadpool_t* pool, int threads ) 
{
   condition_init( &pool->ready ) ; // 初始化条件变量结构体
   pool->first = NULL ; // 设置线程链表头指针
   pool->last = NULL ; // 设置线程链表尾指针
   pool->counter = 0 ; // 设置线程池当前线程数
   pool->idle = 0 ; // 设置线程池当前空闲线程数
   pool->max_threads = threads ; // 设置线程池最大线程数
   pool->quit = 0 ; // quit = 0，线程池运行状态；quit = 1，线程池销毁状态
}

/* 向线程池中添加线程 */
void threadpool_add_task( threadpool_t* pool, void* (*run)( void* arg ), void* arg ) 
{
   task_t* newtask = ( task_t* ) malloc ( sizeof( task_t ) ) ; // 创建线程控制块
   newtask->run = run ; // 设置线程的回调函数
   newtask->arg = arg ; // 设置回调函数的参数
   newtask->next = NULL ; // 新加入的线程会被添加到链表尾部

   /************************** 进入临界区 ***********************/

   condition_lock( &pool->ready ) ; // 拿到互斥锁

   // 把新创建的 TCB 添加到线程链表中
   if ( pool->first == NULL ) 
   {
   	  // 如果线程链表为空，则 TCB 作为链表头部
      pool->first = newtask ;
   }
   else
   {
   	  // 如果线程链表不为空，加入到链表尾部
      pool->last->next = newtask ;
   }
   pool->last = newtask ; // 修改链表尾指针
   
   // 如果有空闲线程，那么就唤醒空闲线程
   if ( pool->idle > 0 ) 
   {
      condition_signal( &pool->ready ) ; // 通知等待在条件变量上的空闲线程
   }
   else if ( pool->counter < pool->max_threads ) 
   {
   	  // 如果没有空闲线程可用，而且当前线程数量小于线程池的容量，我们就创建一个线程
      pthread_t tid ;
      pthread_create( &tid, NULL, thread_runtime, pool ) ; // 指定新线程的起始函数为 thread_runtime，把线程池传递给 thread_runtime
      ++ pool->counter ;
   }

   condition_unlock( &pool->ready ) ; // 释放互斥锁

   /************************** 退出临界区 ***********************/
}

/* 销毁线程池 */
void threadpool_destroy( threadpool_t* pool ) 
{
   if ( pool->quit ) 
   {
      return ;
   } 

   /************************** 进入临界区 ***********************/

   condition_lock( &pool->ready ) ;

   // 设置退出标志为真
   pool->quit = 1 ;

   // 如果线程池中正在运行着线程，那么我们需要等待线程执行完毕再销毁
   if ( pool->counter > 0 ) 
   {
      if ( pool->idle > 0 )
      {
         condition_broadcast( &pool->ready ) ;
      }
      while ( pool->counter > 0 ) 
      {
         condition_wait( &pool->ready ) ; // 主线程（main 函数所在线程）将等待在条件变量上
      }
   }

   condition_unlock( &pool->ready ) ;

   /************************** 退出临界区 ***********************/
   
   // 销毁条件变量
   condition_destroy( &pool->ready ) ;
}
