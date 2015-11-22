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
#include <string>
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
	bool sline_was;
	bool so_was;

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
	stabstr_header.sh_name = 0;
  	stabstr_header.sh_type = 0;
  	stabstr_header.sh_flags = 0;
  	stabstr_header.sh_addr = 0;
  	stabstr_header.sh_offset = 0;
  	stabstr_header.sh_size = 0;
  	stabstr_header.sh_link = 0;
  	stabstr_header.sh_info = 0;
  	stabstr_header.sh_addralign = 0;
  	stabstr_header.sh_entsize = 0;
  	
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
	myStab stab_to_add;
	string source_file;
	for (int j = 1; j < count_of_blocks; ++j) {
		pread(f.get_fd(), &curr_stab, sizeof(curr_stab),
			stab_header.sh_offset + sizeof(curr_stab) * j);
		if (curr_stab.n_type == N_SO || curr_stab.n_type == N_SOL) {
			char * read_source_name = current_char + stabstr_header.sh_offset + curr_stab.n_strx;
			string nanana(read_source_name);
			source_file = nanana;
			if ((strcmp(source_file.c_str(), "") == 0) && curr_stab.n_type == N_SO) {
				if ((all_func.size() != 0) && (!all_func[all_func.size() - 1].so_was)) {
		  			all_func[all_func.size() - 1].end = curr_stab.n_value;
		  			all_func[all_func.size() - 1].so_was = true;
		  		}

			}
	  	}
	  	if (curr_stab.n_type == N_SLINE) {
	  		if (all_func.size() != 0) {
	  		  if (!all_func[all_func.size() - 1].sline_was) {
	  		      all_func[all_func.size() - 1].source = source_file;
		              //cout << "NAME IN SLINE:" << source_file << endl;
	  		      all_func[all_func.size() - 1].sline_was = true;
	  		  }
	  		}
	  	}
	  	if (curr_stab.n_type == N_FUN) {
	  		myStab new_function;
			char * read_fun_name = current_char + stabstr_header.sh_offset + curr_stab.n_strx;
			string nananame(read_fun_name);
			new_function.index = j;
			new_function.start = curr_stab.n_value;
			new_function.sline_was = false;
			new_function.so_was = false;
			new_function.name = nananame.substr(0, nananame.find(':'));

			new_function.source = source_file;

			if (all_func.size() != 0) {
				if (!all_func[all_func.size() - 1].so_was) {
	  				all_func[all_func.size() - 1].end = curr_stab.n_value;
	  			}
	  		}

			all_func.push_back(new_function);


	  	}
		//SORT

	  // name index start end source
	}
	// sort(all_func.begin(), all_func.end(), comp);
	// for (size_t k = 0; k < all_func.size(); ++k) {
	// 	printf("%s %d 0x%08x 0x%08x %s\n", all_func[k].name.c_str(), all_func[k].index, all_func[k].start, all_func[k].end, all_func[k].source.c_str());
	// }
	uint32_t num;
	while(scanf("%x", &num) == 1) {
		bool found = false;
		uint32_t smoff;
		uint16_t line_num;
		string addr_name;
		string addr_source;
		for (size_t k = 0; k < all_func.size(); ++k) {
			if ((num >= all_func[k].start) && (num < all_func[k].end)) { // we have source, func, shoff
				found = true;
				addr_name = all_func[k].name;
				addr_source = all_func[k].source;
				smoff = num - all_func[k].start;
				std::vector<Stab> stab_2(2);
				Stab curr_stab;
				for (int i = all_func[k].index; i < count_of_blocks; ++i) {
					/* code */
				  	pread(f.get_fd(), &curr_stab, sizeof(curr_stab),
						stab_header.sh_offset + sizeof(curr_stab) * i);
				  	if (curr_stab.n_type == N_SLINE) {
				  		if (curr_stab.n_value < smoff) {
				  			stab_2[0] = stab_2[1];
				  			stab_2[1] = curr_stab;
				  		} else if (curr_stab.n_value == smoff) {// это наша
				  			line_num = curr_stab.n_desc;
				  			break;
				  		} else { // наша предыдущая
				  			line_num = stab_2[1].n_desc;
				  			break;
				  		}

				  	}
				}
				break;
			}
		}
		if (found) printf("0x%08x:%s:%s:%x:%d\n", num, addr_source.c_str(),addr_name.c_str(), smoff, line_num ); 
		else printf("0x%08x::::\n", num);
		found = false;
	}


	return 0;
}