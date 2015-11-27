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
// #include <utility>
#include <algorithm>
using namespace std;


//int binary_search(std::vector<myStab> &all_func, uint32_t key, size_t r, size_t l){
//    int m = (l + r) / 2;
//    if ( key >= all_func[m].start && key < all_func[m].end ) return m;
//    if (m == l || m == r) return -1;
//    if (  key >= all_func[m].end ) binary_search(all_func, key, r, m);
//    else if (key < all_func[m].start) binary_search(all_func, key, m, l);
//}

// bool comp(const myStab& stab1, const myStab& stab2) {
// 	int res = strcmp(stab1.name.c_str(), stab2.name.c_str());
// 	if (res < 0) {
// 		return 1;
// 	} else if (res == 0) {
// 		if (stab1.start < stab2.start) return 1;
// 		else return 0;
// 	} else return 0;
// }

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

int binary_search(std::vector<myStab> &all_func, uint32_t key, size_t R, size_t L) {
 	if (!all_func.size()) return -1;
	size_t M;
 	while (1) {
 		M = (L + R) / 2;
 		if (key < all_func[M].start) {
 			R = M - 1;
 		} else if(key >= all_func[M].end) {
 			L = M + 1;
 		} else if (key >= all_func[M].start && key <= all_func[M].end) return M;
 		if (L > R) return -1;
 	}
}

vector<myStab> normalization(std::vector<myStab> &all_func) {
	std::vector<myStab> result;
	if (all_func.size() < 2) {
		return result;
	}

	for (size_t it = 1; it < all_func.size(); ++it) {
		all_func[it - 1].end = all_func[it].start;
		if (all_func[it - 1].index != -77) {
			result.push_back(all_func[it - 1]);
		}

	}
	return result;
}

bool comp2(const myStab& stab1, const myStab& stab2) {
	if (stab1.start < stab2.start) return 1;
	else return 0;
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
	File f(argv[1]);
	if (f.get_fd() == -1) {
		return 0;
	}
	Elf32_Ehdr header_stat;
	char *current_char = (char *) mmap(NULL, f.size(), PROT_READ, MAP_SHARED, f.get_fd(), 0);
	read(f.get_fd(), &header_stat, 52);
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
  	stab_header.sh_offset = 0;
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
	std::vector<myStab> all_SO; // чтоб не сломать функцию поиска для функций
	Stab curr_stab;
	int count_of_blocks = stab_header.sh_size / sizeof(curr_stab);
	myStab stab_to_add;
	string source_file;
	for (int j = 1; j < count_of_blocks; ++j) {
		pread(f.get_fd(), &curr_stab, sizeof(curr_stab),
			stab_header.sh_offset + sizeof(curr_stab) * j);
		if (curr_stab.n_type == N_SO || curr_stab.n_type == N_SOL) {
			// небольшая модификация в массив еще добавим все SO
			//if (curr_stab.n_type == N_SO) {
			//	myStab stab_with_so;
			//	stab_with_so.start = curr_stab.n_value;
			//	stab_with_so.index = -77; // Договоримся, что если стаб индекс =-77 то это SO а не FUN
			//	stab_with_so.end = 0;
			//	all_SO.push_back(stab_with_so);
			//}
			char * read_source_name = current_char + stabstr_header.sh_offset + curr_stab.n_strx;
			string nanana(read_source_name);
			source_file = nanana;
			if ((strcmp(source_file.c_str(), "") == 0) && curr_stab.n_type == N_SO) {
				// my nowa
				myStab stab_with_so;
				stab_with_so.start = curr_stab.n_value;
				stab_with_so.index = -77; // Договоримся, что если стаб индекс =-77 то это SO а не FUN
				stab_with_so.end = 0;
				all_SO.push_back(stab_with_so);
				// my nowa
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
	}
	// Объединяем два вектора
	//printf("all_SO: ");
	//for (size_t k = 0; k < all_SO.size(); ++k) {
	//	printf("0x%08x ", all_SO[k].start);
	//}
	//printf("\n");
	//printf("all_FUN: ");
	//for (size_t k = 0; k < all_func.size(); ++k) {
	//	printf("0x%08x ", all_func[k].start);
	//}
	//printf("\n");
	//printf("merged: ");

	all_func.insert(all_func.end(), all_SO.begin(), all_SO.end());
	// Сортируем все стабы по началу, чтоб потом его нормализвать и привести к виду где у каждой функции нормальное начало и конец
	//for (size_t k = 0; k < all_func.size(); ++k) {
	//	printf("0x%08x ", all_func[k].start);
	//}
	//printf("\n");
	sort(all_func.begin(), all_func.end(), comp2);
	// Нормализуем
	//printf("Sorted:\n");
	//for (size_t k = 0; k < all_func.size(); ++k) {
	//	printf("0x%08x 0x%08x %s\n", all_func[k].start, all_func[k].end, all_func[k].name.c_str());
	//}
	vector<myStab> all_func_res = normalization(all_func);
	//printf("RESULT:\n");

	//for (size_t k = 0; k < all_func_res.size(); ++k) {
	//	printf("0x%08x 0x%08x %s\n", all_func_res[k].start, all_func_res[k].end, all_func_res[k].name.c_str());
	//}
	//
	//printf("\n");
	//for (size_t k = 0; k < all_func.size(); ++k) {
	//	printf("%s %d 0x%08x 0x%08x %s\n", all_func[k].name.c_str(), all_func[k].index, all_func[k].start, all_func[k].end, all_func[k].source.c_str());
	//}
	uint32_t num;
	while(scanf("%x", &num) == 1) {
		int res = binary_search(all_func_res, num, all_func_res.size() - 1, 0);
		if (res == -1) printf("0x%08x::::\n", num);
		else {
			uint16_t line_num = 0;
			string addr_name = all_func_res[res].name;
			string addr_source = all_func_res[res].source;
			uint32_t smoff = num - all_func_res[res].start;
			std::vector<Stab> stab_2(2);
			Stab curr_stab;
			for (int i = all_func_res[res].index; i < count_of_blocks; ++i) {
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

			  	} else if ((curr_stab.n_type == N_SO || curr_stab.n_type == N_FUN) && (i != all_func_res[res].index))  {
					line_num = stab_2[1].n_desc;
					break;
				}
			}
			printf("0x%08x:%s:%s:%x:%d\n", num, addr_source.c_str(),addr_name.c_str(), smoff, line_num );

		}
	}


	return 0;

}
