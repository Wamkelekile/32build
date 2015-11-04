#include <sys/stat.h>
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <vector>
// #include </home/wamkelekile/Dropbox/Progs/rsync/contrib/gtest/gtest.h>

using namespace std;

struct file_desc {
	int uid;
	int gid;
	int rights;
	char f_name[1020];
};

struct process_desc {
	int uid_p;
	std::vector<int> group_v;
	int expected_rights;
};

void first_space(file_desc &cur){
//    std::cout << 'H' << cur.f_name << std::endl;
    int i = 0;
    while(isspace(cur.f_name[i])){
        i++;
    }
    strcpy(cur.f_name, cur.f_name + i);
}

void last_space(file_desc &cur) {
  //  cout << 'h' << cur.f_name << endl;
    string s(cur.f_name);
    int size = s.length() - 1;
    while(isspace(cur.f_name[size])){
        size--;
    }
    cur.f_name[size + 1] = '\0';
    cout << cur.f_name << endl;
}


bool check_rights(int a, int b) {
	// cout << "a = " << a << "b = " << b << endl;
	if(a >= 4) {
		a -= 4;
		b -= 4;
		if (b < 0) return 0;
	} else if (b >= 4) {
		b -= 4;
	}
	if(a >= 2) {
		a -= 2;
		b -= 2;
		if (b < 0) return 0;
	} else if (b >= 2) {
		b -= 2;
	}
	if(a >= 1) {
		a -= 1;
		b -= 1;
		if (b < 0) return 0;
	} else if (b >= 1) {
		b -= 1;
	}
	return 1;

}

bool check_group_member(file_desc &f_inf, process_desc &proc_inf) {
	for (auto i = proc_inf.group_v.begin(); i !=  proc_inf.group_v.end(); ++i) {
		if (*i == f_inf.gid) return 1;
	}
	return 0;

}


void check_file(file_desc &f_inf, process_desc &proc_inf) {
	// cout << 'H' << endl;
	if (f_inf.uid == proc_inf.uid_p) {
		// cout << "root" << ((f_inf.rights & 0700) / 64) << endl;
		if (check_rights(proc_inf.expected_rights & 0007, (f_inf.rights & 0700) / 64)) {
			first_space(f_inf);
			last_space(f_inf);
		}
	} else if (check_group_member(f_inf, proc_inf)) {
		// cout << "group" << ((f_inf.rights & 0070) / 8) << endl;

		if (check_rights(proc_inf.expected_rights & 0007, (f_inf.rights & 0070) / 8)) {
			first_space(f_inf);
			last_space(f_inf);
		}
	} else {
		// cout << "other" << ((f_inf.rights & 0007)) << endl;
		if (check_rights(proc_inf.expected_rights & 0007, (f_inf.rights & 0007))) {
			first_space(f_inf);
			last_space(f_inf);
		}
	}

}

// TEST(check_rights, corrrect_work) {
 //    std::vector<int> v1(8);
 //    std::vector<int> v2(8);
 //    std::vector<std::vector<int>> v3;
 //   //  v3 =  { {1, 0, 0, 0, 0, 0, 0, 0},
	// 		// {1, 1, 0, 0, 0, 0, 0, 0},
	// 		// {1, 0, 1, 0, 0, 0, 0, 0},
	// 		// {1, 1, 1, 1, 0, 0, 0, 0},
	// 		// {1, 0, 0, 0, 1, 0, 0, 0},
	// 		// {1, 1, 0, 0, 1, 1, 0, 0},
	// 		// {1, 0, 1, 0, 1, 0, 1, 0},
	// 		// {1, 1, 1, 1, 1, 1, 1, 1}

 // 		// 	};
 //    std::vector<int> ver = {1, 0, 0, 0, 0, 0, 0, 0};
 //    // v3.push_back({1, 0, 0, 0, 0, 0, 0, 0});
	// for (int i = 0; i < 8; ++i) {
	// 	v1[i] = i;
	// 	v2[i] = i;
	// }
	// for (int i = 0; i < 8; ++i) {
	// 	v1[i] = i;
	// 	v2[i] = i;
	// }
// }



int main(int argc, char const *argv[]) {
	process_desc proc_inf;
	// int uid_p;
	string s;
	std::getline (std::cin, s);
	char buf[s.size()];
	strcpy(buf, s.c_str());
	sscanf(buf, "%d", &proc_inf.uid_p);
	std::vector<int> v;
	std::getline (std::cin, s);
	char buf2[s.size()];
	strcpy(buf2, s.c_str());
	int gid_p; 
	int count;
	while(sscanf(buf2, "%d%n", &gid_p, &count) > 0)  {
		v.push_back(gid_p);

		// cout <<gid_p<< 'H' << count<< 'S' << buf2 <<  endl;
		// usleep(10000);
		string st(buf2);
		st.erase(0, count);
		strcpy(buf2, st.c_str());
	}
	proc_inf.group_v = v;
	
	// cout << endl;
	// int expected_rights;
	string expected_rights_str;
	std::getline (std::cin, expected_rights_str);
	char buf_expected_r_str[expected_rights_str.size()];
	strcpy(buf_expected_r_str, expected_rights_str.c_str());

	sscanf(buf_expected_r_str,"%o", &proc_inf.expected_rights);

	// cout <<proc_inf.uid_p << endl;
	// for (auto i = v.begin(); i != v.end(); ++i) {
	// 	cout << *i << ' ';
	// }
	// cout << endl;
	// cout << proc_inf.expected_rights << endl;


	string str_file_desc;
	std::vector<file_desc> file_inf_arr;

	while(getline(cin, str_file_desc)) {
		file_desc f_inf;
		int count;
		char buf_with_str_inf[str_file_desc.size()];
		strcpy(buf_with_str_inf, str_file_desc.c_str());
		sscanf(buf_with_str_inf, "%d%d%o%n", &(f_inf.uid),
											 &(f_inf.gid),
											 &(f_inf.rights),
											 &count);
		strcpy(f_inf.f_name, buf_with_str_inf + count);
		// cout << f_inf.f_name << endl;
		file_inf_arr.push_back(f_inf);
	}
	
	for (auto i = file_inf_arr.begin(); i != file_inf_arr.end(); ++i) {
		check_file(*i, proc_inf);
	}
	return 0;
}
