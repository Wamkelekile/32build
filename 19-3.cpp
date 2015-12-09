#include <vector>
#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

std::vector<long long> vec;


void *thread_func(void *arg){
	sleep(3);
	for (int j = 0; j < 1000; ++j) {
			// cout << i * 1000 + j << endl;
		vec.push_back((intptr_t) arg * 1000 + j);
	}
	sched_yield();
	return NULL;
}



int main() {
	pthread_t * tid = new pthread_t [100];
	// for (intptr_t i = 0; i < 100; ++i) {
	for (intptr_t i = 0; i < 100; ++i) {
		pthread_create(&tid[i], NULL, thread_func, (void*) i); 
		// for (int j = 0; j < 1000; ++j) {
			// cout << i * 1000 + j << endl;
			// vec.push_back(i * 1000 + j);
		// }
	}
	// for (int i = 0; i < 100; ++i) {
	for (int i = 0; i < 100; ++i) {
		pthread_join(tid[i], NULL);
	}
	// std::cout << vec[10] << std::endl;
	delete tid;

}