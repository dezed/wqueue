/*
   wqueue.h

   Worker thread queue based on the Standard C++ library list
   template class.

   ------------------------------------------

   Copyright @ 2013 [Vic Hargrave - http://vichargrave.com]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __wqueue_h__
#define __wqueue_h__

#include <pthread.h>
#include <list>
#include <iostream>

using namespace std;

template <typename T> class wqueue
{
    list<T>          m_queue;
    pthread_mutex_t  m_mutex;
    pthread_cond_t   m_condv; 
    
    pthread_cond_t   m_cond_all_done; 

    int _unfinished_tasks;


  public:
    wqueue() {
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_condv, NULL);

        pthread_cond_init(&m_cond_all_done, NULL);
	_unfinished_tasks=0;

    }
    ~wqueue() {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_condv);

        pthread_cond_destroy(&m_cond_all_done);
    }
    void add(T item) {
        pthread_mutex_lock(&m_mutex);
        m_queue.push_back(item);
	++_unfinished_tasks;

        pthread_cond_signal(&m_condv);
        pthread_mutex_unlock(&m_mutex);
    }
    T remove() {
        pthread_mutex_lock(&m_mutex);
        while (m_queue.size() == 0) {
            pthread_cond_wait(&m_condv, &m_mutex);
        }
        T item = m_queue.front();
        m_queue.pop_front();
        pthread_mutex_unlock(&m_mutex);
        return item;
    }
    int size() {
        pthread_mutex_lock(&m_mutex);
        int size = m_queue.size();
        pthread_mutex_unlock(&m_mutex);
        return size;
    }
    
    void task_done(void)
    {
      pthread_mutex_lock(&m_mutex);
      
      int unfinished = _unfinished_tasks - 1;

      if (unfinished <= 0) {
	if (unfinished < 0) {
	  std::cout << "ValueError task_done() called too many times.";
	}
	pthread_cond_broadcast(&m_cond_all_done);
      }

      _unfinished_tasks = unfinished;
 
      pthread_mutex_unlock(&m_mutex );
    }

    void join()
    {
      pthread_mutex_lock(&m_mutex);
      
      while (_unfinished_tasks > 0) {
	pthread_cond_wait(&m_cond_all_done, &m_mutex );
      }
      pthread_mutex_unlock(&m_mutex);
    }
    
};

#endif
