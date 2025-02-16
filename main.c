#include "BCD.h"
#include "mathBCD.h"
#include <time.h>
#include <math.h>


int main(void){
    BCD n1 = makeBCD(10);
    BCD n2 = makeBCD(13);
    //計測開始
    clock_t cpu_time_start = clock();
    double sec_start = (double)cpu_time_start/CLOCKS_PER_SEC;
    double ansD = 10.0/13.0;
    //ans = mulB(ans, n2);
    //計測終了
    clock_t cpu_time_end = clock();
    double sec_end = (double)cpu_time_end/CLOCKS_PER_SEC;

    //処理時間
    double result_time = sec_end - sec_start;
    printf("処理時間 : %f\n",result_time);
    //display(ans);
    printf("%f\n", ansD);

    cpu_time_start = clock();
    sec_start = (double)cpu_time_start/CLOCKS_PER_SEC;
    BCD ans = divB(n1, n2);

    cpu_time_end = clock();
    sec_end = (double)cpu_time_end/CLOCKS_PER_SEC;

    //処理時間
    result_time = sec_end - sec_start;
    printf("処理時間 : %f\n",result_time);
    display(ans);

    return 0;
}