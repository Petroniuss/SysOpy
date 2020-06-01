#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>


void sighandler(int signal, siginfo_t* info, void* extra) {
    printf("Received signal %d with value %d\n", signal, info -> si_value.sival_int);
}


int main(int argc, char* argv[]) {

    if(argc != 3){
        printf("Not a suitable number of program parameters\n");
        return 1;
    }

    struct sigaction action;
    action.sa_sigaction = &sighandler;

    int child = fork();
    if(child == 0) {
        //zablokuj wszystkie sygnaly za wyjatkiem SIGUSR1
        //zdefiniuj obsluge SIGUSR1 w taki sposob zeby proces potomny wydrukowal
        //na konsole przekazana przez rodzica wraz z sygnalem SIGUSR1 wartosc
        action.sa_flags = SA_SIGINFO;
        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, SIGUSR1);

        // I don't know which one it should be since its ambigiuous so i do both..
        // block signals while handling SIGUSR1
        sigaction(SIGUSR1, &action, NULL);
        // block all other signals
        sigprocmask(SIG_SETMASK, &action.sa_mask, NULL);

        // ----------------------------------------------
        // I am pausing to wait for signal to arrive! !!!
        pause();
    }
    else {
        //wyslij do procesu potomnego sygnal przekazany jako argv[2]
        //wraz z wartoscia przekazana jako argv[1]
        int signalNo = atoi(argv[2]);
        int value    = atoi(argv[1]);

        union sigval s;
        s.sival_int = value;

        printf("Sending signal %d with value: %d\n", signalNo, value);
        sigqueue(child, signalNo, s);
    }

    return 0;
}
