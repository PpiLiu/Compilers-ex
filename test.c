#include<stdio.h>

int main()
{//测试一下
    int *a;
    a = malloc(100);
    printf("%d",sizeof(a));
    free(a);
    return 0;
}
