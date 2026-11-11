#include<stdio.h>
int sum_to(int n)
{
    int s=0;
    for(int i=0;i<=n;i++)
    {
        s+=i;
    }
    return s;
}
int main(void)
{
    printf("sum_to(3)=%d\n",sum_to(3));
return 0;
}