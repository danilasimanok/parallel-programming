#include <stdio.h>

int main()
{
    int arr[] = { 2 };
    int length = 1;
    while (length > 1) {
        for (int i = 0; i < length / 2; ++i)
            arr[i] = arr[2 * i] + arr[2 * i + 1];
        if (length % 2) {
            arr[length / 2] = arr[length - 1];
            length = length / 2 + 1;
        } else
            length /= 2;
    }
    printf("sum = %d\n", arr[0]);
    return 0;
}