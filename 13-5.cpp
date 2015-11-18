#include <stdint.h>
#include <iostream>
#include <elf.h>
#include <cstring>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stab.h>
using namespace std;

struct Stab {
  uint32_t n_strx;
  uint8_t n_type;
  uint8_t n_other;
  uint16_t n_desc;
  uintptr_t n_value;

};


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

	char *current_char = (char *) mmap(NULL, f.size(), PROT_READ, MAP_SHARED, f.get_fd(), 0);
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
	Elf32_Shdr stab_header, stabstr_header;
	for(int i = 0; i <= header_stat.e_shnum - 1; ++i) {
		Elf32_Shdr cur_sec_header; 
		pread(f.get_fd(), &cur_sec_header,
					  40, header_stat.e_shoff + 40 * i);
		if(!i) continue;
		string name;
		name.resize(20);
	        pread(f.get_fd(),(char*) name.c_str(), 20,
		 names_section_header.sh_offset + cur_sec_header.sh_name);
		string stab_str = ".stab";
		string stabstr_str = ".stabstr";
		if (strcmp(name.c_str(), stab_str.c_str()) == 0) {
		  stab_header = cur_sec_header;
		  cout << "stub" << endl;
		}
		if (strcmp(name.c_str(), stabstr_str.c_str()) == 0) {
		  stabstr_header = cur_sec_header;
		  cout << "stubstr" << endl;
		}
		//printf("%20s 0x%08x %10d %10d 0x%04x\n",
		//	 name.c_str(), cur_sec_header.sh_addr,
		//	 cur_sec_header.sh_offset, cur_sec_header.sh_size,
		//	 cur_sec_header.sh_addralign);
	}
	Stab curr_stab;
	int count_of_blocks = stab_header.sh_size / sizeof(curr_stab);
	cout << count_of_blocks << endl;
	//char * read_fun_name = current_char;

	int in_comp = -1;
	bool is_func = false;
	bool sline_was = false;
	string source, true_f_name;
	int f_index;
	uintptr_t f_start;
	for (int j = 0; j < count_of_blocks; ++j) {
		if (!j) continue;
	  	pread(f.get_fd(), &curr_stab, sizeof(curr_stab),
			stab_header.sh_offset + sizeof(curr_stab) * j);
		if (curr_stab.n_type == N_SO) {
			if (in_comp) { //push func_desc we have f_end and source
				printf("%s %d 0x%08x 0x%08x %s\n", true_f_name.c_str(), f_index, f_start, curr_stab.n_value, source.c_str());
				sline_was = false;
			}
			in_comp *= -1;
		}
		if ((curr_stab.n_type == N_SOL) && (is_func) && (!sline_was)) {
			string source_name;
			source_name.resize(50);
			char * source_ptr = (char *) source_name.c_str();
			char * read_source_name = current_char + stabstr_header.sh_offset + curr_stab.n_strx;
			while(*read_source_name != '\0') {
				*source_ptr = *read_source_name;
				read_source_name++;
				source_ptr++;
			}
			*source_ptr = '\0';
			cout << "SOURCE" << source_name << endl;
			source = source_name;
		}
		if ((curr_stab.n_type == N_SLINE) && is_func) sline_was = true;
	  	if (curr_stab.n_type == N_FUN) {
			if (is_func) { // push f_sescription
				printf("%s %d 0x%08x 0x%08x %s\n", true_f_name.c_str(), f_index, f_start, curr_stab.n_value, source.c_str());
				sline_was = false;
			}
			is_func = true; // expect N_So or N_func
			size_t st_index = 0;
			size_t pos = 0;
			string fun_name;
			fun_name.resize(50);
			char * name = (char *) fun_name.c_str();
			char * read_fun_name = current_char + stabstr_header.sh_offset + curr_stab.n_strx;
			while(*read_fun_name != '\0') {
				*name = *read_fun_name;
				read_fun_name++;
				name++;
			}
			*name = '\0';
			true_f_name = fun_name;
			f_start = curr_stab.n_value;
			f_index = j;

			//cout << st_index << ' '<< curr_stab.n_strx << endl;
	    		//cout << "J:"  << fun_name << endl;
			//printf("%s %d 0x%08x 0x%08x %s\n", fun_name.c_str(), j, curr_start.n_value, 1, "SOURCE");
	  	}

	  // name index start end source
	}


	return 0;
}
