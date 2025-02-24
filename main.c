#include "BCD.h"
#include "mathBCD.h"
#include <time.h>
#include <math.h>


int main(void){
    BCD n1 = makeBCD(0.5235987755982988);
    printf("sin=");
    display(sinB(n1));
    printf("sin=%f\n", toDouble(sinB(n1)));
    BCD n2 = makeBCD(-13);
    display(sqrtB(n2));
    //計測開始
    clock_t cpu_time_start = clock();
    double sec_start = (double)cpu_time_start/CLOCKS_PER_SEC;
    BCD n = sqrtB(n2);
    //計測終了
    clock_t cpu_time_end = clock();
    double sec_end = (double)cpu_time_end/CLOCKS_PER_SEC;

    //処理時間
    double result_time = sec_end - sec_start;
    printf("処理時間 : %f\n",result_time);
    //display(ans);
    display(n);

    return 0;
}