#include <stdio.h>
#include <math.h>
#include <memory.h>
#include "Word.h"

const int g = 64;

Word newWord(int a0, int a1, int a2, int a3, int a4, int a5) {
    Word r;
    r.myBool = a0;
    r.a[0] = a1;
    r.a[1]=a2;
    r.a[2]=a3;
    r.a[3]=a4;
    r.a[4]=a5;
    return r;
}

int getValue_Word(Word myWord) {
    int v = 0;
    for (int i = 0; i < 5; i++) {
        v += ((int) pow(64, i)) * myWord.a[i];
    }
    if (myWord.myBool == 1) {
        return v;
    } else {
        return -v;
    }
}

Word getWord(int value) {
    int flag;
    if (value >= 0) {
        flag = 1;
    } else {
        flag = 0;
        value = -value;
    }
    Word r ;
    for (int i = 4; i >=0; i--) {
        r.a[i] = value % 64;
        value /= 64;
    }
    r.myBool=flag;
    return r;
}

Word add(Word myWord1, Word myWord2) {
    int s = getValue_Word(myWord1) + getValue_Word(myWord2);
    Word r = getWord(s);
    if (s == 0) r.myBool = myWord1.myBool;
    return r;
}

Word subtract(Word myWord1, Word myWord2) {
    int s = getValue_Word(myWord1) - getValue_Word(myWord2);
    Word r = getWord(s);
    if (s == 0) r.myBool = myWord1.myBool;
    return r;
}

Word getPartOfWord(Word x, int F) {
    Word newW;
    reset(&newW);
    int L = F / 8;
    int R = F % 8;
    int L1 = L;
    if (L == 0) {
        newW.myBool = x.myBool;
        L1 = 1;
    }
    if (R > 0) {
        for (int i = R - 1, j = 4; i >= L1 - 1; i--, j--) {
            newW.a[j] = x.a[i];
        }
    }
    return newW;
}

void reset(Word *x) {
    (*x).myBool = 1;
    for (int i = 0; i < 5; i++) {
        (*x).a[i] = 0;
    }
}

Word mutiplyX(Word w1, Word w2) {
    int v = (int) ((long) getValue_Word(w1) * (long) getValue_Word(w2)) % (g * g * g * g * g);
    int flag;
    if (w1.myBool == w2.myBool) flag = 1;
    else flag = 0;
    Word w = getWord(v);
    w.myBool = flag;
    return w;
}

Word mutiplyA(Word w1, Word w2) {
    int v = (int) ((long) getValue_Word(w1) * (long) getValue_Word(w2)) / (g * g * g * g * g);
    int flag;
    if (w1.myBool == w2.myBool) flag = 1;
    else flag = 0;
    Word w = getWord(v);
    w.myBool = flag;
    return w;
}




//
// Created by apple on 2018/8/30.
//

