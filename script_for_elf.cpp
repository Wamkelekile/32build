
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdint>
using namespace std;

int main() {
	uint32_t a, b;
	scanf("%x %x\n", &a, &b);
	//std::ofstream file;
	FILE* f = fopen("MATHERFUCKER", "w");
	for(uint32_t i = a; i <= b; ++i) {
//		i += 10;
		fprintf(f, "%08x\n", i);
	}
	fclose(f);
	return 1;
}
