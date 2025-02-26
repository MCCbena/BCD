//
// Created by shuta on 25/02/15.
//

#ifndef BCD_BCD_H
#define BCD_BCD_H

#include <stdio.h>

#define getAddr(n, addr) ((n.decimal[(addr)/2] >> !((addr)%2)*4) & (0x0F))
#define setAddr(n, addr, value) (n.decimal[(addr)/2] |= ((int)(value)) << (int)(!((addr)%2))*4)

typedef struct{
    char sign; //符号
    unsigned int point; //整数の最後のバイト
    unsigned char decimal[128]; //データ
}BCD;

BCD makeBCD(double value);
BCD quickMakeBCD(unsigned char sign, const char* value, unsigned int digits);
void display(BCD n);
double toDouble(BCD n);
long toLong(BCD n);

BCD addB(BCD n1, BCD n2);
BCD subB(BCD n1, BCD n2);
BCD mulB(BCD n1, BCD n2);
BCD divB(BCD n1, BCD n2);

BCD pow10B(BCD n, int pow);

char eqB(BCD n1, BCD n2);
char gtB(BCD n1, BCD n2);
char gteB(BCD n1, BCD n2);
char ltB(BCD n1, BCD n2);
char lteB(BCD n1, BCD n2);

const unsigned int integer_byte = 10;

BCD makeBCD(double value){
    BCD bcd = {0};

    //マイナスをプラスにする
    if(value < 0){
        value*=-1;
        bcd.sign = 1;
    }

    unsigned long addr = 0;
    //整数の代入
    int int_digits = 0; //整数の桁数を代入
    unsigned int threshold = 9;
    unsigned int maximum_multiple = 1; //10^桁数乗
    unsigned int int_value = (unsigned int)value; //引数から整数を抜き出し

    //桁数を計算
    while (int_value > threshold){
        int_digits++;
        maximum_multiple *= 10;
        threshold *= 10;
        threshold += 9;
    }
    int_digits++;
    //BCDに変換
    addr = (integer_byte*2)-int_digits;
    unsigned int temp = int_value;
    for (int i = int_digits; i > 0; i--) {
        unsigned digit = temp/maximum_multiple;
        temp = int_value%maximum_multiple;
        maximum_multiple /= 10;

        setAddr(bcd, addr, digit);
        addr++;
    }

    bcd.point = addr;

    //小数点
    double decimal_value = value-int_value;
    while (addr/2 < sizeof(bcd.decimal)){
        decimal_value*=10;
        if(decimal_value == 0){
            break;
        }
        setAddr(bcd, addr, (unsigned int)decimal_value);

        decimal_value-=(int)decimal_value;
        addr++;
    }
    return bcd;
}

/**
 * 桁数の計算を省略することで高速な変換をします。
 * 小数点は使えません。
 * @param value 値
 * @param digits 桁数
 * @return
 */
BCD quickMakeBCD(unsigned char sign, const char* value, unsigned int digits){
    BCD bcd = {0};
    bcd.sign = sign;
    bcd.point=integer_byte*2;

    for (int i = 0; i < digits; ++i) {
        setAddr(bcd, (integer_byte*2-1)-i, value[digits-i-1]-48);
    }

    return bcd;
}

BCD addB(BCD n1, BCD n2){
    BCD bcd = {0};
    bcd.point = n1.point;
    char of = 0;
    char sign = (char)(n1.sign ^ n2.sign);
    bcd.sign = (char)(n1.sign && n2.sign);

    //必ずn1のほうが大きくなるようにする
    if(sign) {
        for (int i = 0; i < sizeof(bcd.decimal); i++) {
            if (n1.decimal[i] == n2.decimal[i]) continue;

            if (n1.decimal[i] < n2.decimal[i]) {
                BCD temp = n1;
                n1 = n2;
                n2 = temp;
                bcd.sign = n1.sign;
                n1.sign = 0;
            } else bcd.sign = n1.sign;
            break;
        }
        n2.sign = sign;
    } else {
        n1.sign = 0;
        n2.sign = 0;
    }

    for(int addr = (sizeof(bcd.decimal)*2-1); addr >= 0; addr--){
        unsigned int decimal1_bit = getAddr(n1, addr);
        unsigned int decimal2_bit = getAddr(n2, addr)+of;
        if(decimal1_bit == 0 && decimal2_bit == 0) continue;
        unsigned int sum;
        //マイナスが含まれているかいないかで別の動きをする
        if(sign==0) {
            sum = decimal1_bit + decimal2_bit;
            if (sum >= 0X0A) {
                of = 1;
                //sumから0x0Aを引くことで4ビットが絶対に9を越さないようにする
                sum -= 0x0A;
            } else of = 0;
        }else {
            if (decimal1_bit < decimal2_bit) {
                //計算結果がマイナスになってしまう場合、繰り下げを行う
                sum = (decimal1_bit + 0x0A) - decimal2_bit;
                of = 1;
            } else {
                //decimal2_bitに繰り下げが発生した場合引くようになってるから通常計算
                sum = decimal1_bit - decimal2_bit;
                of = 0;
            }
        }
        setAddr(bcd, addr, sum);
    }

    return bcd;
}

BCD subB(BCD n1, BCD n2){
    n2.sign = 1;
    return addB(n1, n2);
}

BCD mulB(BCD n1, BCD n2){
    BCD bcd = {0};
    bcd.point = n1.point;
    char sign = (char)(n1.sign != n2.sign);
    n1.sign = 0;
    n2.sign = 0;

    for (int i0 = 0; i0 < (sizeof(n2.decimal)*2); ++i0) {
        BCD temp_bcd = {0};
        temp_bcd.point = n1.point;

        unsigned int val = getAddr(n2, i0);
        if(val==0) continue;
        temp_bcd = n1;

        if (val >= 2) {
            temp_bcd = addB(temp_bcd, temp_bcd);
            //display(temp_bcd);
            if (val >= 4) {
                if (val >= 8) {
                    temp_bcd = addB(temp_bcd, temp_bcd);
                    temp_bcd = addB(temp_bcd, temp_bcd);
                    if (val == 9){
                        temp_bcd = addB(temp_bcd, n1);
                    }
                }else{
                    if(val != 4) {
                        if (val >= 6) {
                            temp_bcd = addB(temp_bcd, addB(temp_bcd, temp_bcd));
                        }else temp_bcd = addB(temp_bcd, temp_bcd);
                        if (val != 6) {
                            temp_bcd = addB(temp_bcd, n1);
                        }
                    }else temp_bcd = addB(temp_bcd, temp_bcd);
                }
            } else if(val==3){
                temp_bcd = addB(temp_bcd, n1);
            }
        }

        temp_bcd = pow10B(temp_bcd, (int) (bcd.point-1) - i0);
        bcd = addB(bcd, temp_bcd);
    }

    bcd.sign = sign;
    return bcd;
}

/**
 * 割り算をします。内部的には、n1*1/n2という形で掛け算に変換されます。
 * @param n1
 * @param n2
 * @return
 */
BCD divB(BCD n1, BCD n2){
    BCD bcd = {0};
    bcd.point = n1.point;


    BCD one = {0};
    one.point = bcd.point;
    setAddr(one, n1.point-1, 1);
    n1.sign = (n1.sign ^ n2.sign);
    n2.sign = 0;

    BCD zero = {0};
    zero.point = bcd.point;

    if(eqB(n2, zero)){
        printf("Division by zero occurs\n");
        return bcd;
    }
    // 1/n2を生成
    //控除数を宣言。今回は1。
    BCD dividend = one;
    for (int i = 0; i < sizeof(bcd.decimal)*2; i++) {
        BCD div_num_count = zero; //現在計算中の「商」が代入される
        BCD div_num = n2; //除数*nの一時的な計算に必要な値が代入される
        BCD div_num_back = zero; //除数*nをwhileが更新する前の値が代入される

        while (lteB(div_num, dividend)){//現在の控除数の値を除数が超えていなければ商のカウンターを増やす
            div_num_back = div_num;
            div_num = addB(div_num, n2);
            div_num_count = addB(div_num_count, one);
        }


        //小数点の場所を維持
        BCD temp = pow10B(div_num_count, -i);
        bcd = addB(bcd, temp); //bcdに計算した商を代入
        dividend = pow10B(subB(dividend, div_num_back), 1);//次の位の控除数を計算して代入
    }

    //以上の動作でbcdが1/bcdに変換されるため、割り算を掛け算として計算できる
    return mulB(n1, bcd);
}

/**
 * 10^n上を実行します。
 * @param n
 * @param pow
 * @return
 */
BCD pow10B(BCD n, int pow){
    BCD bcd = {0};
    bcd.sign = n.sign;
    bcd.point = n.point;
    int offset1=0, offset2=0;
    if(pow > 0){
        offset1=pow;
    } else offset2=pow;
    for (int i = offset1; i < (sizeof(n.decimal) * 2)+offset2; ++i) {
        unsigned int val = getAddr(n, i);
        setAddr(bcd, i - pow, val);
    }
    return bcd;
}

/**
 * n1 > n2を判定します。
 * @param n1
 * @param n2
 * @return
 */
char gtB(BCD n1, BCD n2){
    return (char)!lteB(n1, n2);
}

/**
 * n1 >= n2を判定します。
 * @param n1
 * @param n2
 * @return
 */
char gteB(BCD n1, BCD n2){
    return (char)!ltB(n1, n2);
}

/**
 * n1 < n2を判定します。
 * @param n1
 * @param n2
 * @return
 */
char ltB(BCD n1, BCD n2){
    if(n1.sign == 1 && n2.sign == 0) return 1;
    if(n1.sign == 1 || n2.sign == 1) {
        n1.sign = 0;
        n2.sign = 0;
        BCD temp = n1;
        n1 = n2;
        n2 = temp;
    }
    for (int i = 0; i < sizeof(n1.decimal); ++i) {
        if(n1.decimal[i] == 0 && n2.decimal[i] == 0) continue;
        if(n1.decimal[i] < n2.decimal[i]){
            return 1;
        }
        if(n1.decimal[i] > n2.decimal[i]){
            return 0;
        }
    }

    return 0;
}

/**
 * n1 <= n2を判定します。
 * @param n1
 * @param n2
 * @return
 */
char lteB(BCD n1, BCD n2){
    if(n1.sign == 1 && n2.sign == 0) return 1;
    if(n1.sign == 1 || n2.sign == 1) {
        n1.sign = 0;
        n2.sign = 0;
        BCD temp = n1;
        n1 = n2;
        n2 = temp;
    }
    for (int i = 0; i < sizeof(n1.decimal); ++i) {
        if(n1.decimal[i] == 0 && n2.decimal[i] == 0) continue;
        if(n1.decimal[i] < n2.decimal[i]){
            return 1;
        }
        if(n1.decimal[i] != n2.decimal[i]){
            return 0;
        }
    }

    return 1;
}

/**
 * n1==n2を判定します。
 * @param n1
 * @param n2
 * @return
 */
char eqB(BCD n1, BCD n2){
    if(n1.sign != n2.sign) return 0;
    for (int i = 0; i < sizeof(n1.decimal); ++i) {
        if(n1.decimal[i] != n2.decimal[i]) return 0;
    }

    return 1;
}

void display(BCD n){
    if(n.sign == 1){
        printf("-");
    }
    for(int addr = 0; addr < (sizeof(n.decimal)*2); addr++){
        unsigned int decimal_bit = getAddr(n, addr);
        printf("%x", decimal_bit);
        if(addr+1 == n.point){
            printf(".");
        }
    }
    printf("\n");
}

/**
 * BCDをlongに変換します。（小数点丸め込み）
 * toDoubleを用いてLongにキャストするよりも高速に変換することが可能です。
 * @param n
 * @return
 */
long toLong(BCD n){
    long ret = 0;
    char input = 0;
    for (int i = 0; i < integer_byte * 2; ++i) {
        int val = getAddr(n, i);
        if(val!=0) input = 1;
        if(input) {
            ret *= 10;
            ret += val;
        }
    }

    return ret;
}


/**
 *
 * @param n
 * @return
 */
double toDouble(BCD n){
    double ret = 0;
    char input = 0;

    for(int i = ((sizeof(n.decimal)-1)*2); i >= n.point; --i){
        double val = getAddr(n, i);
        if(val!=0) input=1;
        if(input){
            ret/=10;
            ret+=val/10;
        }

    }

    return ret + (double)toLong(n);
}

#endif //BCD_BCD_H

