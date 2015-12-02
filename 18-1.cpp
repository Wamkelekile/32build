#include <pthread.h>
#include <iostream>
#include <cstdint>
#include <unistd.h>
using namespace std;

//pthread_t tid[10];

void *fun(void *i) {
	int g = (int)i;
	cout << g << endl;
	return NULL;
}

int main() {
	pthread_t tid[10];
	for (int i = 0; i < 10; ++i) {
		pthread_create(&tid[i], NULL, fun, (void *)i);
	}
	pthread_join(tid[9], NULL);
	return 0;
}
