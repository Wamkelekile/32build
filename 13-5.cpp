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
#include <vector>
#include <algorithm>
using namespace std;

struct Stab {
  	uint32_t n_strx;
  	uint8_t n_type;
  	uint8_t n_other;
  	uint16_t n_desc;
  	uintptr_t n_value;

};

struct myStab {
	string name;
	int index;
	uintptr_t start;
	uintptr_t end;
	string source;

};

bool comp(const myStab& stab1, const myStab& stab2) {
	int res = strcmp(stab1.name.c_str(), stab2.name.c_str());
	if (res < 0) {
		return 1;
	} else if (res == 0) {
		if (stab1.start < stab2.start) return 1;
		else return 0;
	} else return 0;
}

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
		return 0;
	}
	Elf32_Ehdr header_stat;
	//int *kmap;
	char *current_char = (char *) mmap(NULL, f.size(), PROT_READ, MAP_SHARED, f.get_fd(), 0);
	//int filedesk = (int) mmap(NULL, f.size(), PROT_READ, MAP_SHARED, f.get_fd(), 0);
	read(f.get_fd(), &header_stat, 52);

	// START pRINT

	if(header_stat.e_type != 2) {
		return 0;
	}
	uint8_t is_debug_info = 0;
	Elf32_Shdr names_section_header;
	pread(f.get_fd(), &names_section_header, 40,
		 header_stat.e_shoff + 40 * header_stat.e_shstrndx);
	Elf32_Shdr stab_header, stabstr_header;
	stabstr_header.n_strx = 0;
  	stabstr_header.n_type = 0;
  	stabstr_header.n_other = 0;
  	stabstr_header.n_desc = 0;
  	stabstr_header.n_value = 0;

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
			is_debug_info++;
		}
		if (strcmp(name.c_str(), stabstr_str.c_str()) == 0) {
		  	stabstr_header = cur_sec_header;
			is_debug_info++;
		}
	}
	if (is_debug_info != 2) {
		printf("No debug info\n");
		return 0;
	}
	vector<myStab> all_func;
	Stab curr_stab;
	int count_of_blocks = stab_header.sh_size / sizeof(curr_stab);
	bool in_comp = false;
	bool is_func = false;
	bool sline_was = false;
	string source, true_f_name;
	int f_index;
	uintptr_t f_start;
	for (int j = 0; j < count_of_blocks; ++j) {
		if (!j) continue;
	  	pread(f.get_fd(), &curr_stab, sizeof(curr_stab),
			stab_header.sh_offset + sizeof(curr_stab) * j);
		//if (in_comp) cout << "WELL" << endl;
		if (curr_stab.n_type == N_SO) {
			if (in_comp) { //push func_desc we have f_end and source
				//printf("%s %d 0x%08x 0x%08x %s\n", true_f_name.c_str(), f_index, f_start, curr_stab.n_value, source.c_str());
				myStab tmp_f;
				tmp_f.name = true_f_name;
				tmp_f.index = f_index;
				tmp_f.start = f_start;
				tmp_f.end = curr_stab.n_value;
				tmp_f.source = source;
				all_func.push_back(tmp_f);
				sline_was = false;
				is_func = false;
				source = "";
			} else {
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
				source = source_name;
			}
			in_comp  = !in_comp;
			//is_func = false;
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
			source = source_name;
		}
		if ((curr_stab.n_type == N_SLINE) && is_func) sline_was = true;
	  	if (curr_stab.n_type == N_FUN) {
			f_start = curr_stab.n_value;
			f_index = j;
			if (is_func) { // push f_sescription
				//
				myStab tmp_f;
				tmp_f.name = true_f_name;
				tmp_f.index = f_index;
				tmp_f.start = f_start;
				tmp_f.end = curr_stab.n_value;
				tmp_f.source = source;
				all_func.push_back(tmp_f);
				sline_was = false;
			}
			is_func = true; // expect N_So or N_func
			string fun_name;
			fun_name.resize(50);
			char * name = (char *) fun_name.c_str();
			char * read_fun_name = current_char + stabstr_header.sh_offset + curr_stab.n_strx;
			while(*read_fun_name != ':') {
				*name = *read_fun_name;
				read_fun_name++;
				name++;
			}
			*name = '\0';
			true_f_name = fun_name;

			//cout << st_index << ' '<< curr_stab.n_strx << endl;
	    		//cout << "J:"  << fun_name << endl;
			//printf("%s %d 0x%08x 0x%08x %s\n", fun_name.c_str(), j, curr_start.n_value, 1, "SOURCE");
	  	}
		//SORT

	  // name index start end source
	}
	sort(all_func.begin(), all_func.end(), comp);
	for (size_t k = 0; k < all_func.size(); ++k) {
		printf("%s %d 0x%08x 0x%08x %s\n", all_func[k].name.c_str(), all_func[k].index, all_func[k].start, all_func[k].end, all_func[k].source.c_str());
	}


	return 0;
}