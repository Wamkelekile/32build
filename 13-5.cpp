#include <stdint.h>
#include <iostream>
#include <elf.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

using namespace std;

// typedef struct
// {
//     unsigned char e_ident[16];
//     uint16_t      e_type;
//     uint16_t      e_machine;
//     uint32_t      e_version;
//     uint32_t      e_entry;
//     uint32_t      e_phoff;
//     uint32_t      e_shoff;
//     uint32_t      e_flags;
//     uint16_t      e_ehsize;
//     uint16_t      e_phentsize;
//     uint16_t      e_phnum;
//     uint16_t      e_shentsize;
//     uint16_t      e_shnum;
//     uint16_t      e_shstrndx;
// } Elf32_Ehdr;

class File {
public:
	File(char* file_name) {
		f_ = 0;
		f_ = open(file_name, O_RDONLY, 0);
		fstat (f_, &	s_);
		f_size_ = s_.st_size;
	}
	~File() {
		if(f_ != 0 ) {
			close(f_);
        }
	}
	long long size() {
		return f_size_;
	}
	int get_fd() {
		return f_;
	}
private:
	int f_;
	struct stat s_;
	long long f_size_;
};

int main(int argc, char *argv[]) {
	long long count = 0;
	if (!(access(argv[1], F_OK) == 0) ) {
		std::cout << -1 << std::endl;
		return 0;
	}
	File f(argv[1]);
	Elf32_Ehdr header_stat;
	int *kmap;
	cout << f.size() << endl;

	// char *current_char = (char *) mmap(NULL, f.size(), PROT_READ, MAP_SHARED, f.get_fd(), 0);
	kmap = (int *) mmap(NULL, f.size(), PROT_READ, MAP_SHARED, f.get_fd(), 0);
	if (kmap == MAP_FAILED) {
		cout << "FAIL!" << endl;
	}
	// string s("abcd");
	// char * data = s.c_str();
	// cout << *current_char << endl;
	
	// int n = *(reinterpret_cast<int *>(current_char)); // current_char is my data
	// int n = *(reinterpret_cast<int *>(current_char)); // current_char is my data
	char buf[10];
	
	// cout << *buf << endl;
	cout << "last_point: "<< header_stat.e_shstrndx << endl;

	int well_red = read(f.get_fd(), &header_stat, 52);

	// cout << *buf << endl;
	cout << "red header status: "<< well_red << endl;
	cout << "type:"<<header_stat.e_type << "|" << "shoff:"  << header_stat.e_shoff << endl; 
	cout << "sh_num: "<< header_stat.e_shnum << endl;
	cout << "last_point: "<< header_stat.e_ehsize << endl;



	return 0;
}