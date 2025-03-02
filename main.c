#include "BCD.h"
#include "mathBCD.h"
#include <time.h>
#include <math.h>


int main(void){
    //BCD n1 = makeBCD(2.55);
    BCD n1 = divB(quickMakeBCD(0, "1", 1), quickMakeBCD(0, "3", 1));
    //BCD n1 = makeBCD(0.1);
    display(n1);
    /*
    printf("自作:asin=");
    display((asinB((n1))));
    printf("math.hのasin:%f\n", asin(1));
     */
    //double ans = 0;
    //scanf("%lf", &ans);
    //BCD n2 = (asinB((n1)));
    //printf("n2=");
    //display(n2);
    //計測開始
    clock_t cpu_time_start = clock();
    double sec_start = (double)cpu_time_start/CLOCKS_PER_SEC;
    BCD n = mulB(n1, n1);
    //ans = ans*ans;

    //計測終了
    clock_t cpu_time_end = clock();
    double sec_end = (double)cpu_time_end/CLOCKS_PER_SEC;

    //処理時間
    double result_time = sec_end - sec_start;
    printf("処理時間 : %f\n",result_time);
    //printf("%f\n", ans);
    printf("n=");
    display(n);

    return 0;
}