#include <stdio.h>
#include <string.h>
#include <stdlib.h>
struct test
{
    char (*val)[33];
};
typedef char (*valdef)[33];
struct test t;
int main()
{
    int s = sizeof(char(*)[33]);
    printf("%d\n", s);
    s = sizeof(t.val);
    printf("%d\n", s);
    s = sizeof(*(t.val));
    printf("%d\n", s);
    t.val = (valdef)malloc(sizeof(*(t.val)) * 4);
    strcpy(t.val[0], "123");
    strcpy(t.val[1], "456");
    strcpy(t.val[2], "789");
    printf("%s,%s,%s\n", t.val[0], t.val[1], t.val[2]);
    return 0;
}