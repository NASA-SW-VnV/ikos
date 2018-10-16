extern void __ikos_assert(int);

int main() {
    unsigned long x = 0x200000001;
    unsigned int y = (unsigned int)(x);
    __ikos_assert(y == 1);
    return 0;
}
