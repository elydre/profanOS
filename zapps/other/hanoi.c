#include "syscall.h"

void afficher(int **plateau, int n);
void solve(int n, int from_rod, int to_rod, int aux_rod);
void move(int **plateau, int from, int to);

int main(int arg) {
    // ask for input
    char inp[3];
    int n;
    c_fskprint("Enter number of disks: ");
    c_input(inp, 3, 0x09);
    c_fskprint("\n");
    n = c_ascii_to_int(inp);
    int **plateau = c_calloc(3 * sizeof(int*));
    for (int i=0; i<3; i++) {
        plateau[i] = c_calloc(n * sizeof(int));
    }
    for (int i=0; i<n; i++) {
        plateau[0][i] = i+1;
    }
    solve(n, 0, 1, 2);
    for (int i=0; i<3; i++) {
        c_free(plateau[i]);
    }
    c_free(plateau);
    c_fskprint("\n");
    return arg;
}

void solve(int n, int from_rod, int to_rod, int aux_rod) {
    if (n == 1) {
        c_fskprint("\nMove disk 1 from rod %d to rod %d", from_rod, to_rod);
        return;
    }
    solve(n-1, from_rod, aux_rod, to_rod);
    c_fskprint("\nMove disk %d from rod %d to rod %d", n, from_rod, to_rod);
    solve(n-1, aux_rod, to_rod, from_rod);
}
