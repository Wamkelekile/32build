#include <pthread.h>
#include <iostream>
#include <cstdint>
#include <unistd.h>
using namespace std;

void *fun(void *i) {
	// cout << (int*)i << endl;
	return;
}

int main(int argc, char const *argv[]) {
	pthread_t tid;
	for (int i = 0; i < 10; i++) {
		pthread_create(&tid, NULL, fun, (void *)i);
	}
	pthread_join(tid, NULL);
	return 0;
}
