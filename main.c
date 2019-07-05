#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<unistd.h>
#include<time.h>
#include"timer.h"
#include"list.h"
#include"hashmap.h"
//for map
map_t mymap[2];//0 for session
char map_buf[1024*1024*100];  // balloc cache size
int Malloc(void)
{
	int rc;
	rc = bopen(map_buf,sizeof(map_buf),0);
	if( rc < 0 )
	{
	 printf("balloc failed\n");
     return 1;
	}
	return 0;

}
void map_init()
{
	mymap[0] = hashmap_new();
	mymap[1] = hashmap_new();
}
typedef struct session_map{
	list_t *list;
	int len;
}SESSMAP;
void func_test_map()
{
	mymap[0] = hashmap_new();
	mymap[1] = hashmap_new();
	int i = 0;
	for (i; i < 100000; i++) {
		SESSMAP *value = malloc(sizeof(SESSMAP));
		value->list = list_new();
		  list_node_t *a = list_node_new("a");
		  list_node_t *b = list_node_new("b");
		  list_node_t *c = list_node_new("c");
		  // a b c
		  list_rpush(value->list , a);
		  list_rpush(value->list , b);
		  list_rpush(value->list , c);
		value->len = i;
		char *key = (char *) malloc(24);
		memset(key, 0, 24);
		sprintf(key, "session_%d", i);
		hashmap_put(mymap[0], key, value);
	}
	i = 0;
	for (i; i < 100000; i++) {
		SESSMAP *value = NULL;
		char *key = (char *) malloc(24);
		memset(key, 0, 24);
		sprintf(key, "session_%d", i);
		hashmap_get(mymap[0], key, (void**) (&value));
		if (value) {
			printf("Get value:%d\n", value->len);
			list_iterator_t *it = list_iterator_new(value->list, LIST_HEAD);
			list_node_t *a = list_iterator_next(it);
			  for(a;a!=NULL;a = list_iterator_next(it)){
				  printf("List value:%s\n",a->val);
				 // free(a->val);
			  }
			  free(value);
		}
	}
}
typedef struct event_t{
    int id;
}event_t;

void event_cb(void * event)
{
    event_t * _event = (event_t *)event;
    struct timeval now;
    gettimeofday(&now, NULL);
    unsigned long curtime = now.tv_sec * 1000000 + now.tv_usec;
    printf("event %d timeout, current time is %ld\n", _event->id, curtime / 1000);
    return;
}
//

void unit_test_for_queue1()
{
    zx_timer_t * timer = zx_timer_create(0, TM_TYPE_QUEUE);

    struct timeval current_time;
	long timeout = 5;
    long curtime;
    zx_timer_index_t * indexs[10000];
    event_t events[10000];
    int i = 0;
	while (i < 5)
    {
		//微秒 百万分之一秒
        events[i].id = i;
        gettimeofday(&current_time, NULL);
		curtime = current_time.tv_sec * 1000000 + current_time.tv_usec; //当前微妙
		//printf("sec :%d: usec:%d", current_time.tv_sec, current_time.tv_usec);
		//printf("curtime :%d \n", curtime);

        if(zx_timer_add(timer, curtime, timeout, event_cb, &events[i], &indexs[i]) < 0)
        {
            printf("add event %d failed\n", i);
        }
        else
        {
            printf("add event %d success, it will occur at %ld\n", i, (curtime + timeout) / 1000);
        }
		zx_timer_check(timer, curtime, 10000);
        usleep(100);
        i++;
    }
    printf("%ld\n", zx_timer_count(timer));
    printf("%ld\n", zx_timer_memsize(timer));
    zx_timer_destroy(timer, NULL, NULL);
}

void unit_test_for_queue()
{
    zx_timer_t * timer = zx_timer_create(0, TM_TYPE_QUEUE);

    struct timeval current_time;
	unsigned long timeout = 1000000;
    unsigned long curtime;
    zx_timer_index_t * indexs[10000];
    event_t events[10000];
    int i = 0;
	while (i < 5000)
    {
		//微秒 百万分之一秒
        events[i].id = i;
        gettimeofday(&current_time, NULL);
		curtime = current_time.tv_sec * 1000000 + current_time.tv_usec; //当前微妙
		//printf("sec :%d: usec:%d", current_time.tv_sec, current_time.tv_usec);
		//printf("curtime :%d \n", curtime);

        if(zx_timer_add(timer, curtime, timeout, event_cb, &events[i], &indexs[i]) < 0)
        {
            printf("add event %d failed\n", i);
        }
        else
        {
            printf("add event %d success, it will occur at %ld\n", i, (curtime + timeout) / 1000);
        }
//		zx_timer_check(timer, curtime, 10000);
        i++;
    }
	sleep(1);
	while (i < 5000)
    {
		//微秒 百万分之一秒
        events[i].id = i;
        gettimeofday(&current_time, NULL);
		curtime = current_time.tv_sec * 1000000 + current_time.tv_usec; //当前微妙
		//printf("sec :%d: usec:%d", current_time.tv_sec, current_time.tv_usec);
		//printf("curtime :%d \n", curtime);

        if(zx_timer_add(timer, curtime, timeout, event_cb, &events[i], &indexs[i]) < 0)
        {
            printf("add event %d failed\n", i);
        }
        else
        {
            printf("add event %d success, it will occur at %ld\n", i, (curtime + timeout) / 1000);
        }
//		zx_timer_check(timer, curtime, 10000);
        i++;
    }
	i=0;
	while(1)
	{
		 gettimeofday(&current_time, NULL);
		 curtime = current_time.tv_sec * 1000000 + current_time.tv_usec; //当前微妙
		 int ret=zx_timer_check(timer, curtime, 10000);
		 if(ret!=-1)
		 {
			 i++;
		 }
		 usleep(100);
	}
    printf("%ld\n", zx_timer_count(timer));
    printf("%ld\n", zx_timer_memsize(timer));
    zx_timer_destroy(timer, NULL, NULL);
}


void unit_test_for_wheel()
{
    zx_timer_t * timer = zx_timer_create(3000, TM_TYPE_WHEEL);

    srand((int)time(NULL));

    struct timeval current_time;
    unsigned long curtime;

    zx_timer_index_t * indexs[10000];
    event_t events[10000];
    int i = 0;

    while(i < 10000)
    {
        unsigned long timeout = (rand() % 10 + 1) * 1000;
        events[i].id = i;
        gettimeofday(&current_time, NULL);
        curtime = current_time.tv_sec * 1000000 + current_time.tv_usec;

        zx_timer_check(timer, curtime, 10000);
        if(zx_timer_add(timer, curtime, timeout, event_cb, &events[i], &indexs[i]) < 0)
        {
            printf("add event %d failed\n", i);
        }
        else
        {
            printf("add event %d at time %ld, timeout %ld, it will occur at %ld\n", \
                    i, curtime / 1000, timeout,  (curtime + timeout) / 1000);
        }

       // usleep(100);
        i++;
        if(i==9999)
        	i=0;
    }
    zx_timer_destroy(timer, NULL, NULL);
}


void wheel_test()
{
    zx_timer_t * timer = zx_timer_create(6, TM_TYPE_WHEEL);

    srand((int)time(NULL));

    struct timeval current_time;
    unsigned long curtime;

    zx_timer_index_t * indexs[20];
    event_t events[20];
    int i = 0;

    while(i < 20)
    {
        unsigned long timeout = rand() % 10;
        events[i].id = i;
        curtime = i;
        zx_timer_check(timer, curtime, 10000);
        if(zx_timer_add(timer, curtime, timeout, event_cb, &events[i], &indexs[i]) < 0)
        {
            printf("add event %d failed\n", i - 10);
        }
        else
        {
            printf("add event %d at time %ld, timeout %ld, it will occur at %ld\n", \
                    i, curtime, timeout,  curtime + timeout);
        }

        sleep(1);
        i++;
    }
    zx_timer_destroy(timer, NULL, NULL);
}

void *test(void *p) {
	//unit_test_for_queue1();
	//unit_test_for_wheel();
	//unit_test_for_queue();
	// wheel_test();
	func_test_map();
}
#include<pthread.h>
int main()
{
	pthread_t t;
	pthread_create(&t, NULL, test, NULL);
	pthread_join(t, NULL);
	//sleep(20);
    return 0;
}
