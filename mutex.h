//*****************************************************************************
//
// Copyright (C) 2001 Steve Connet.  All rights reserved.
//
// Source File Name : mutex.h
// Author           : Steve Connet
//
// Version          : $Id: $
//
// File Overview    : class for single locking mechanism and condition
//
// Revision History :
//
// $Log: $
//
//*****************************************************************************

#ifndef __MUTEX_H_
#define __MUTEX_H_

#include <sys/time.h>
#include <pthread.h>

class Mutex
{
public:
    Mutex() : event(false)
    {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~Mutex()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void lock()
    {
        pthread_mutex_lock(&mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
    bool trylock()
    {
        return pthread_mutex_trylock(&mutex) == 0;
    }

    void signal()
    {
        lock();
        event = true;
        unlock();
        pthread_cond_signal(&cond);
    }

    void broadcast()
    {
        lock();
        event = true;
        unlock();
        pthread_cond_broadcast(&cond);
    }

    void waitEvent()
    {
        lock();
        while(!event)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        unlock();
        event = false;
    }

    bool waitEvent(int timeout /* ms */)
    {
        const long billion = 1000000000L;

        lock();
        while(!event)
        {
            struct timespec deadline;
            struct timespec interval;
            interval.tv_sec = timeout / 1000;
            interval.tv_nsec = (timeout % 1000) * 1000000L;

            struct timeval now;
            gettimeofday(&now, NULL);
            deadline.tv_sec = now.tv_sec;
            deadline.tv_nsec = now.tv_usec * 1000;

            if((deadline.tv_nsec += interval.tv_nsec) >= billion)
            {
                deadline.tv_nsec -= billion;
                deadline.tv_sec += 1;
            }

            deadline.tv_sec += interval.tv_sec;
            if(pthread_cond_timedwait(&cond, &mutex, &deadline) == ETIMEDOUT)
            {
                break;
            }
        }
        unlock();
        return event;
    }

    void reset()
    {
        event = false;
    }

private:
    bool event;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

#endif // __MUTEX_H_
