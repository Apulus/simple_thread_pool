/*******************************************************************
*  Copyright(c) 2016 Chen Gonghao
*  All rights reserved.
*
*  chengonghao@yeah.net
******************************************************************/

#include "threadpool.h"

/* 定义线程池最大线程数 */
#define MAX_POOL_SIZE	3

/* 每个任务的回调函数 */
void* mytask( void* arg ) 
{
   printf( " thread 0x%x is working on task %d\n", (int)pthread_self(), *(int*)arg ) ;
   sleep( 1 ) ;
   free( arg ) ;
   return NULL ;
}

int main( void ) 
{
   threadpool_t pool ; // 定义一个线程池变量
   threadpool_init( &pool, MAX_POOL_SIZE ) ; // 初始化线程池

   // 向线程池中添加 10 个任务，每个任务的处理函数都是 mytask
   for ( int i = 0; i < 10; ++ i ) 
   {
      int* arg = (int*)malloc( sizeof( int ) ) ;
      *arg = i ;
      threadpool_add_task( &pool, mytask, arg ) ;
   }

   threadpool_destroy( &pool ) ; // 销毁线程池
   
   return 0 ;
}
