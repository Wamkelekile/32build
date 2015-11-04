#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

bool read_file(const char *path, std::vector<unsigned char> &vec) {
	char bufffer[4096];
	int open_file = open(path, O_RDONLY);
	if (open_file == -1) {
		close(open_file);
		return false;
	}
	while (int count_of_read_bites = read(open_file, bufffer, 4096)) {
		if (count_of_read_bites != -1) {
			vec.insert(vec.end(), bufffer, bufffer + count_of_read_bites);
		} else {
			close(open_file);
			return false;			
		}
	}
	close(open_file);
	return true;
}

// int main(int argc, char const *argv[])
// {
// 	/* code */
// 	return 0;
// }