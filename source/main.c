// https://www.coranac.com/tonc/text/first.htm

int main(void) {
    *(unsigned int*)0x04000000 = 0x0403;

    // 5,5,5 BGR colors
    ((unsigned short*)0x06000000)[120 + 80 * 240] = 0x001F; // red pixel - 0000 0000 0001 1111
    ((unsigned short*)0x06000000)[136 + 80 * 240] = 0x03E0; // green pixel - 0000 0011 1110 0000
    ((unsigned short*)0x06000000)[120 + 96 * 240] = 0x7C00; // blue pixel - 0111 1100 0000 0000

    while(1);

    return 0;
}