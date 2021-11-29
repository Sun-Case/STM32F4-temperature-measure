#include "main.h"

/*
 需要按需调整时钟频率
 注意编译器优化代码，空循环可能会错误优化
 */

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
int main() {
    while (1) {}
}
#pragma clang diagnostic pop
