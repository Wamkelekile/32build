#include <iostream>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <unistd.h>
#include <cmath>
#include <sys/types.h>
#include <sys/socket.h>
#include <sstream>
#include <fcntl.h>
#include <cstring>

#include <signal.h>

uint32_t N = 0;
int to[2];
int back[2];
int MOOD = 1;
pid_t p1, p2;

using namespace std;

void hnd(int s) {
    uint32_t ping;

    while(read(back[0], &ping, sizeof(ping))) {
        if (!ping) {
            break;
        }
        if (ping == N) {
            ping = 0;
            write(to[1], &ping , sizeof(ping));
            break;
        }
        ping += 1;
        cout << 1 << ' ' << ping << endl;
        write(to[1], &ping, sizeof(ping));

    }
}

void hndd(int s) {
    uint32_t pong;
    
    while(read(to[0], &pong, sizeof(pong))) {
        if (!pong) {
            break;
        }
        if (pong == N) {
            pong = 0;
            write(back[1], &pong , sizeof(pong));
            break;
        }
        pong += 1;
        cout << 2 << ' ' << pong << endl;
        write(back[1], &pong, sizeof(pong));

    }
}



// pipe 1 write, pipe 0 read
int main(int argc, char const *argv[]) {
    if ((atoi(argv[1]) - 1) == 0) {
        cout << "Done" << endl;
        return 0;
    }
    N = atoi(argv[1]) - 1;
    if (pipe(to) < 0 || pipe(back) < 0) {exit(1);} 
    int pid;
    pid = fork();
    if (pid == 0) { 
        uint32_t ping100 = 1;
        close(to[0]);
        close(back[1]);
        cout << 1 << ' ' << ping100 << endl;
        write(to[1], &ping100, sizeof(ping100));
        signal(SIGUSR1, hnd);
        kill(getppid(), SIGUSR1);
        kill(getpid(), SIGUSR1);
        //hnd(1);
        exit(0);
        close(to[1]);
        close(back[0]);
    } else {
        close(back[0]);
        close(to[1]);
        //hndd(1);
	signal(SIGUSR1, hndd);
        wait(0);
        close(back[1]);
        close(to[0]);
        cout << "Done" << endl;
        exit(0);
    }
    return 0;
}
