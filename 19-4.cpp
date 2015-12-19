#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <cstdio>
#include <vector>
#include <time.h>
#include <unistd.h>
 
using namespace std;
 
int N, MAXV;
int num = 0;
 
class Worker {
    pthread_t th;
    pthread_cond_t c = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    int i;
    volatile bool flag = false;
    public:
        void start(int i);
        void *run(void *vptr);
        void join();
        void wait();
        void notify();
        int get_i() {
            return i;
        }
};
 
vector <Worker> threads;
 
void Worker::join() {
    pthread_join(th, NULL);
}
 
void Worker::wait() {
    flag = false;
    pthread_mutex_lock(&m);
    while (!flag)
        pthread_cond_wait(&c, &m);
    pthread_mutex_unlock(&m);
}
 
void Worker::notify() {
    pthread_mutex_lock(&m);
    flag = true; // устанавливаем флаг на случай, если signal и wait разомнутся по времени
    pthread_cond_signal(&c);
    pthread_mutex_unlock(&m);
}
 
void *thread_func(void *vptr) {
   // Worker *a = (Worker *)vptr;
    int i = (int) vptr;
    threads[i].wait();
   // int i = a->get_i();
    while (num < MAXV) {
        cout << i << ' ' << num << endl;
        num++;
        if ((num * num) % N == i)
            continue;
        threads[(num * num) % N].notify();
        threads[i].wait();
    }
    cout << i << ' ' << '!' << num << endl;
    cout << N << endl;
    return NULL;
}
 
void Worker::start(int _i) {
    i = _i;
    pthread_create(&th, NULL, thread_func, (void *) i);
 //   this->wait();
}
 
 
int main(int argc, char *argv[]) {
    N = atoi(argv[1]);
    MAXV = atoi(argv[2]);
    threads.resize(N + 1);
    for (int i = 0; i <= N; ++i) {
        threads[i].start(i);
    }
    threads[0].notify();
    while (num < MAXV) {
        usleep(1);
    }
    for (int j = 0; j <= N; ++j) {
        cout << j << ' ';
        threads[j].notify();
        threads[j].join();
    }
    return 0;
}