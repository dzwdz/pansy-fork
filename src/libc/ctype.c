int isprint(unsigned char c) {
    return c < 127 && c > 31;
}
int isspace(unsigned char c) {
    return (c > 7 && c < 14) || c == ' ';
}
