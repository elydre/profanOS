#include <syscall.h>
#include <iolib.h>

int is_prime(int n);

int main(int argc, char **argv) {
    fskprint("$4Starting the performance test...\n");

    int start_time = c_time_gen_unix();    
    int n = 15 * 1000 * 1000;
    int count = 0;
    for (int i = 0; i < n; i++)
        count += is_prime(i);

    int time = c_time_gen_unix() - start_time;
    fskprint("$4Find $1%d $4prime numbers in $1%d $4seconds\n", count, time);
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
