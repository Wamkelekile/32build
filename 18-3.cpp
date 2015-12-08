#include <iostream>
#include <stdint.h>
#include <pthread.h>
#include <sstream>

pthread_t tid[4];

using namespace std;

void* thanler(void *ptr) {
	intptr_t sum = 0;
	int num;
	while (scanf("%d\n", &num) != EOF) {
		sum += num;
		sched_yield();
	}
	int g = (intptr_t) ptr;
	if (g > 0) pthread_join(tid[g - 1], NULL);
	//комментарий тута
	return (void*) sum;
}

int main(int argc, char *argv[]) {
	uint32_t N = 0;
	stringstream ss;
	ss << argv[1];
	ss >> N;
	// pthread_t tid[N];
	for (intptr_t i = 0; i < N; ++i) { 
		pthread_create(&tid[i], NULL, thanler, (void *)i); 
	}
	long long res = 0;
	cout << "d" << endl;
	// pthread_join(tid[3], NULL);
	cout << "d" << endl;
	for (int i = 0; i < N; ++i) {
		cout << "H" << endl;
		res += pthread_join(tid[i], NULL);
	}
	cout << res << endl;
	return 0;
}