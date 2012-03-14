#include <stdio.h>
#include <time.h>

int main() {
    printf("#define COMPILE_DATE %d\n", (int) time(NULL));
    return 0;
}
