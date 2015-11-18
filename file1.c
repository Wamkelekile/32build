#include "file1.h"
int val;
int func1(void)
{
    scanf("%d", &val);
    return val;
}

int main()
{
    func1();
    func2(val);
    return 0;
}