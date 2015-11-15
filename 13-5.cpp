#include <stdint.h>
#include <iostream>
#include <elf.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

using namespace std;


class File {
public:
	File(char* file_name) {
		f_ = 0;
		f_ = open(file_name, O_RDONLY, 0);
		if (f_ != -1) {
			fstat (f_, &	s_);
			f_size_ = s_.st_size;
		}
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
  //long long count = 0;
	//if (!(access(argv[1], F_OK) == 0) ) {
	//	std::cout << -1 << std::endl;
	//	return 0;
	//}
	File f(argv[1]);
	if (f.get_fd() == -1) {
		cerr << "Couldn't open input file" << endl;
		return 1;
	}
	Elf32_Ehdr header_stat;
	//int *kmap;

	//char *current_char = (char *) mmap(NULL, f.size(), PROT_READ, MAP_SHARED, f.get_fd(), 0);
	//int filedesk = (int) mmap(NULL, f.size(), PROT_READ, MAP_SHARED, f.get_fd(), 0);
	int well_red = read(f.get_fd(), &header_stat, 52);
	// ERRORS
	if (well_red != 52) {
		cerr << "Error while reading ELF Header" << endl;
		return 1;
	}
	if (!((header_stat.e_ident[0] == '\x7f') && (header_stat.e_ident[1] == 'E')
	 	&& (header_stat.e_ident[2] == 'L') && (header_stat.e_ident[3] = 'F'))) {
		cerr << "Not an ELF file" << endl;
		return 1;
	}
	if ((header_stat.e_ident[4] != 1) ||
	 ((header_stat.e_ident[5] != 1) && (header_stat.e_ident[5] != 2)) ||
	(header_stat.e_ident[6] != 1) ) {
		cerr << "Not supported ELF file" << endl;
		return 1;
	}



	// START pRINT

	if (header_stat.e_type == 0) {
		printf("TYPE: NONE\n");
	} else if(header_stat.e_type == 1) {
		printf("TYPE: REL\n");
	} else if(header_stat.e_type == 2) {
		printf("TYPE: EXEC\n");
	} else if(header_stat.e_type == 3) {
		printf("TYPE: DYN\n");
	} else if(header_stat.e_type == 4) {
		printf("TYPE: CORE\n");
	}
	Elf32_Shdr names_section_header;
	pread(f.get_fd(), &names_section_header, 40,
		 header_stat.e_shoff + 40 * header_stat.e_shstrndx);
	printf("%20s %10s %10s %10s %6s\n", "NAME", "ADDR", "OFFSET", "SIZE", "ALGN");
	for(int i = 0; i <= header_stat.e_shnum - 1; ++i) {
		Elf32_Shdr cur_sec_header;

		pread(f.get_fd(), &cur_sec_header,
					  40, header_stat.e_shoff + 40 * i);
		if(!i) continue;
		string name;
		name.resize(20);
			pread(f.get_fd(),(char*) name.c_str(), 20,
		 names_section_header.sh_offset + cur_sec_header.sh_name);
		printf("%20s 0x%08x %10d %10d 0x%04x\n",
			 name.c_str(), cur_sec_header.sh_addr,
			 cur_sec_header.sh_offset, cur_sec_header.sh_size,
			 cur_sec_header.sh_addralign);
	}


	return 0;
}
