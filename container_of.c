#include <stdio.h>

struct ZZ
{
    int a;
    int b;
};

#define offset_of(TYPE, MEMBER) ((size_t)&((TYPE *)0)->MEMBER)
#define container_of(TYPE, MEMBER_ADDR, MEMBER) (TYPE *)(((void *)&MEMBER_ADDR)-offset_of(TYPE, MEMBER))

int main()
{
    struct ZZ xx = {2,3};
    struct ZZ *y = container_of(struct ZZ, xx.b, b);

    printf("%p\n", offset_of(struct ZZ, b));
    printf("%p %p\n", &xx, y);
    printf("%d\n", y->b);
}