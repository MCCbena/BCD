//
// Created by shuta on 25/02/16.
//

#ifndef BCD_MATHBCD_H
#define BCD_MATHBCD_H

#include "BCD.h"

BCD factorialB(BCD n);
BCD logB(BCD n);
BCD sqrtB(BCD n);
//三角関数
BCD sinB(BCD n);
BCD cosB(BCD n);
BCD tanB(BCD n);

/**
 * n!を出力します。小数点がある場合の動作は保証されません。
 * @param n
 * @return
 */
BCD factorialB(BCD n){
    BCD one = quickMakeBCD(0, "1", 1);
    BCD bcd = one;
    bcd.point = n.point;
    for (BCD i = one; lteB(i, n); i=addB(i, one)) {
        bcd = mulB(bcd, i);
    }

    return bcd;
}

BCD sqrtB(BCD n){
    /*https://text.tomo.school/extraction-of-square-root/
     * このwebサイトの実装です。
     * */
    int step = 0;
    int offset = 0;

    BCD bcd = {0};
    bcd.point = n.point;

    BCD zero = {0};
    zero.point = sizeof(n.decimal)*2;

    BCD one = {0};
    one.point = sizeof(n.decimal)*2;
    setAddr(one, sizeof(n.decimal)*2-1, 1);

    BCD two = {0};
    two.point = sizeof(n.decimal)*2;
    setAddr(two, sizeof(n.decimal)*2-1, 2);

    BCD selector = {0};
    BCD tenth_place = {0};
    for (int i = 0; (bcd.decimal[sizeof(bcd.decimal)-1] & 0x0f) == 0x00; i++) {
        if(step==0) {
            selector.decimal[sizeof(n.decimal)-1] = n.decimal[i];//1バイトにつき二桁の数が代入されるため、ここでステップ2を再現。selectorは現在選択中の数値の塊という意味。
            selector.point = sizeof(n.decimal)*2;
            if (eqB(selector, zero)) continue;
            BCD square = zero; //二乗した数を代入
            BCD square_num = zero; //二乗する値を代入
            BCD square_back = zero; //square変数のバックアップ

            //このwhileでstep3を再現。
            while (lteB(square, selector)) {
                square_back = subB(square_num, one); //計算コスト削減のため、squareする前の値を保存
                square = mulB(square_num, square_num); //二乗
                square_num = addB(square_num, one); //1を足す
            }

            //((sizeof(n.decimal)-1)*2) - (sizeof(n.decimal)+4)+iという式を使うと何故か小数点の位置が求めれる。
            setAddr(bcd, ((sizeof(n.decimal)-1)*2) - (sizeof(n.decimal)*2-integer_byte-2)+i, square_back.decimal[sizeof(bcd.decimal)-1]); //step3で出てきた値を代入

            tenth_place = pow10B(addB(square_back, square_back), 1); //step7の再現
            BCD temp = subB(selector, mulB(square_back, square_back)); //step4の再現
            selector = pow10B(temp, 2); //step6で桁おろしするために100をかけて位を上げる。桁おろし自体はもっと下のifでやっている。

            offset = ((sizeof(n.decimal)-1)*2) - (sizeof(n.decimal)*2-integer_byte-2);

            step=1;
            continue;
        }

        BCD square = zero; //二乗計算の準備
        square.point = sizeof(n.decimal) * 2;

        BCD square_back = zero;
        square_back.point = sizeof(n.decimal) * 2;

        BCD first_place_counter = one;//ステップ8の□*10□=214と書かれているところの10の部分がtenth_place変数に代入されていて、□の部分を保存するための変数
        first_place_counter.point = sizeof(n.decimal) * 2;
        if (sizeof(n.decimal) > i) { //step6・9の桁おろしを実際におこなる。BCDのdecimal配列のサイズをiが上回ると未定義動作が起こるため、ifで桁おろしが可能か判定
            selector.decimal[sizeof(n.decimal) - 1] = n.decimal[i];
        } else {
            selector.decimal[sizeof(n.decimal) - 1] = 0;
        }
        selector.point = sizeof(n.decimal) * 2;
        while (lteB(square, selector)) { //step8の□の部分を計算する。上にあるwhileと一緒。
            square_back = square;
            tenth_place = addB(tenth_place, one);
            square = mulB(tenth_place, first_place_counter);
            first_place_counter = addB(first_place_counter, one);
            if (getAddr(square, 0) != 0) { //オーバーフローの兆候があれば計算を切り上げる
                printf("overflow\n");
                return bcd;
            }
        }
        first_place_counter = subB(first_place_counter, two); //step8の□に入る数を見つけれたら、そこから2を引いて実際の値にする。

        setAddr(bcd, offset+i, first_place_counter.decimal[sizeof(bcd.decimal)-1]); //step8で出てきた値を代入

        tenth_place = pow10B(addB(subB(tenth_place, one), first_place_counter), 1);
        selector = pow10B(subB(selector, square_back), 2); //step9の桁おろしをするための前段階。ついでにstep10の引き算もやる。
    }
    bcd.sign = n.sign;
    return bcd;
}

BCD powB(BCD x, BCD y){
    BCD one = quickMakeBCD(0, "1", 1);
    BCD bcd = one;

    for(BCD i = quickMakeBCD(0, "0", 1); ltB(i, y); i = addB(i, one)){
        bcd = mulB(bcd, x);
    }

    return bcd;
}


/**
 * マクローリン展開によりsinを求めます。
 * ラジアンです。
 * @param n
 * @return
 */
BCD sinB(BCD n){

    //必須定数の宣言
    BCD zero = {0};
    zero.point = n.point;
    BCD bcd = zero;
    BCD one = quickMakeBCD(0, "1", 1);
    BCD two = quickMakeBCD(0, "2", 1);

    int minus_clock = 0; //cosを微分すると-sin, -sinを微分すると-cosとなるため、この変数で値をマイナスにするべきかどうかを定める
    for (BCD i = zero; ; i = addB(i, one)) {
        BCD x = addB(mulB(i, two), one); //i*2+1 を実行。sin 0=0であるため、sinの計算は省略する。そのためにiを2倍している。

        BCD power = powB(n, x);
        power.sign = (char) minus_clock;
        BCD factorial = factorialB(x);
        if(getAddr(factorial, 0) != 0){
            break;
        }

        BCD temp = divB(power, factorial);
        if(eqB(temp,zero)){
            break;
        }
        bcd = addB(bcd, temp);

        minus_clock = (!minus_clock);
    }

    return bcd;
}

BCD cosB(BCD n){
    //必須定数の宣言
    BCD zero = {0};
    zero.point = n.point;
    BCD bcd = zero;
    BCD one = quickMakeBCD(0, "1", 1);
    BCD two = quickMakeBCD(0, "2", 1);

    int minus_clock = 0; //cosを微分すると-sin, -sinを微分すると-cosとなるため、この変数で値をマイナスにするべきかどうかを定める
    for (BCD i = zero; ; i = addB(i, one)) {

        BCD x = addB(mulB(i, two), zero); //i*2+1 を実行。sin 0=0であるため、sinの計算は省略する。そのためにiを2倍している。

        BCD power = powB(n, x);
        power.sign = (char) minus_clock;
        BCD factorial = factorialB(x);
        if(getAddr(factorial, 0) != 0){
            break;
        }
        bcd = addB(bcd, divB(power, factorial));

        minus_clock = (!minus_clock);
    }

    return bcd;
}

BCD tanB(BCD n){
    return divB(sinB(n), cosB(n));
}

#endif //BCD_MATHBCD_H
