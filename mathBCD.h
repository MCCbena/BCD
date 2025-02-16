//
// Created by shuta on 25/02/16.
//

#ifndef BCD_MATHBCD_H
#define BCD_MATHBCD_H

#include "BCD.h"

#define ln(x, n) (n * ((x))

BCD factorialB(BCD n);
BCD logB(BCD n);

/**
 * n!を出力します。
 * @param n
 * @return
 */
BCD factorialB(BCD n){
    BCD one = quickMakeBCD(0, "1", 1);
    BCD bcd = one;
    for (BCD i = one; lteB(i, n); i=addB(i, one)) {
        bcd = mulB(bcd, i);
    }

    return bcd;
}



BCD logB(BCD n){

}

#endif //BCD_MATHBCD_H
