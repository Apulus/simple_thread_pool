AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=threadpool
threadpool_SOURCES=\
   main.c \
   threadpool.c condition.c

threadpool_CXXFLAGS=-Ibson/src -Iinclude -D_FILE_OFFSET_BITS=64 -ggdb -Wall -O0
threadpool_LDADD=-lpthread -lm -lrt
threadpool_LDFLAGS=-fPIC -rdynamic -pthread
