void
strncpy_upper(char *dst, int n, char *src)
{
    unsigned char *a = (unsigned char *)src;
    unsigned char *b = (unsigned char *)dst;

    int c = 0;
    while (a[c] && (c < n-1)) {
        if (a[c] > 0x60 && a[c] < 0x7B) {
            b[c] = a[c] - 0x20;
        } else {
            b[c] = a[c];
        }
        c++;
    }
    b[c] = 0;
}
