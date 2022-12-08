#include <syscall.h>
#include <string.h>
#include <iolib.h>

void afficher(int **plateau, int n);
void solve(int n, int from_rod, int to_rod, int aux_rod);
void move(int **plateau, int from, int to);

int main(int argc, char **argv) {
    // ask for input
    char inp[3];
    int n;
    fskprint("Enter number of disks: ");
    input(inp, 3, 0x09);
    fskprint("\n");
    n = ascii_to_int(inp);
    int **plateau = c_calloc(3 * sizeof(int *));
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
    fskprint("\n");
    return 0;
}

void solve(int n, int from_rod, int to_rod, int aux_rod) {
    if (n == 1) {
        fskprint("\nMove disk 1 from rod %d to rod %d", from_rod, to_rod);
        return;
    }
    solve(n-1, from_rod, aux_rod, to_rod);
    fskprint("\nMove disk %d from rod %d to rod %d", n, from_rod, to_rod);
    solve(n-1, aux_rod, to_rod, from_rod);
}
