//
// Created by apple on 2018/8/31.
//

#ifndef MIXSIM_WORD_H
#define MIXSIM_WORD_H

#endif //MIXSIM_WORD_H
typedef struct {
    int myBool;
    int a[5];
} Word;

int getValue_Word(Word myWord);

Word getWord(int value);

Word add(Word myWord1, Word myWord2);

Word subtract(Word myWord1, Word myWord2);

Word getPartOfWord(Word x, int F);

void reset(Word *x);

Word mutiplyX(Word w1, Word w2);

Word mutiplyA(Word w1, Word w2);
