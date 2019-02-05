//
// Created by Haifa Bogdan Adnan on 04/08/2018.
//

#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>
#include <vector>
#include <queue>
#include <list>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <random>

#include <thread>
#include <mutex>
#include <chrono>

#include <cmath>
#include <signal.h>

#include <unistd.h>
#include <sys/time.h>

#include<sys/socket.h>
#include<netdb.h>
#include<arpa/inet.h>
#include <fcntl.h>

#include <getopt.h>

#include <microhttpd.h>
#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

struct rk_sema {
#ifdef __APPLE__
    dispatch_semaphore_t    sem;
#else
    sem_t                   sem;
#endif
};


static inline void
rk_sema_init(struct rk_sema *s, uint32_t value)
{
#ifdef __APPLE__
    dispatch_semaphore_t *sem = &s->sem;

    *sem = dispatch_semaphore_create(value);
#else
    sem_init(&s->sem, 0, value);
#endif
}

static inline bool
rk_sema_wait(struct rk_sema *s, int timeout_sec)
{
#ifdef __APPLE__
    return dispatch_semaphore_wait(s->sem, dispatch_time(DISPATCH_TIME_NOW, timeout_sec * 1000000000)) == 0; //timeout after 5 sec
#else
    int r;
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) return false;
    ts.tv_sec += timeout_sec;
    while ((r = sem_timedwait(&s->sem, &ts)) == -1 && errno == EINTR)
        continue;

    return s != -1;
#endif
}

static inline void
rk_sema_post(struct rk_sema *s)
{

#ifdef __APPLE__
    dispatch_semaphore_signal(s->sem);
#else
    sem_post(&s->sem);
#endif
}

static inline void
rk_sema_destroy(struct rk_sema *s)
{

#ifdef __APPLE__
    dispatch_release(s->sem);
#else
    sem_destroy(&s->sem);
#endif
}

using namespace std;

#define LOG(msg) cout<<msg<<endl<<flush

#include <config.h>

#endif
