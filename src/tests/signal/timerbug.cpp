//cxx: -lrt -O2

#include <stdio.h> 
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <signal.h>
#include <time.h>

int test;

void timer_handler(union sigval sv) {
	auto ok = sv.sival_ptr == &test;
	printf("timer_handler called, ok = %d\n", ok);

	exit( ok ? 0 : -1);
}

int main() {

	timer_t timer;
	sigevent sige;
	itimerspec timerspec;

        memset(&sige, 0, sizeof(sige));

        sige.sigev_notify = SIGEV_THREAD;
        sige.sigev_notify_function = &thread_handler;
        sige.sigev_value.sival_ptr = &test;

        timer_create(CLOCK_REALTIME, &sige, &timer);
        
	memset(&timerspec, 0, sizeof(timerspec));

        timerspec.it_value.tv_sec = 0;
        timerspec.it_value.tv_nsec = 1;

        timer_settime(timer, 0, &timerspec, NULL);

	sleep(1);
        
        puts("should never get here\n");
        return -2;
}
