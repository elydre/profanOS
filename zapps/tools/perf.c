#include <syscall.h>
#include <stdio.h>

int is_prime(int n);

int main(int argc, char **argv) {
    printf("\e[95mStarting the performance test...\e[0m\n");

    int start_time = c_timer_get_ms();
    int n = 15 * 1000 * 1000;
    int count = 0;
    for (int i = 0; i < n; i++)
        count += is_prime(i);

    int time = c_timer_get_ms() - start_time;
    printf("\e[95mFind \e[92m%d \e[95mprime numbers in \e[92m%d \e[95mms\e[0m\n", count, time);
    return 0;
}

int is_prime(int n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (int i = 5; i * i <= n; i = i + 6)
        if (n % i == 0 || n % (i + 2) == 0)
            return 0;
    return 1;
}
