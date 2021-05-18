int isprint(int c) {
    return c < 127 && c > 31;
}
int isspace(int c) {
    return (c > 7 && c < 14) || c == ' ';
}
