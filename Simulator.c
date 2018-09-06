#include <stdio.h>
#include <math.h>
#include "Word.h"

static Word rA;
static Word rX;
static Word rI[6];
static Word rJ;
static Word memory[4000];
static const int g = 64;
static int overflow = 0;
static int compare;
static int current;

void load(Word x, int M, int F) {
    M--;
    reset(&x);
    int L = F / 8;
    int R = F % 8;
    int L1 = L;
    if (L == 0) {
        x.myBool = memory[M].myBool;
        L1 = 1;
    }
    if (R > 0) {
        for (int i = R - 1, j = 4; i >= L1 - 1; i--, j--) {
            x.a[j] = memory[M].a[i];
        }
    }
    M++;
}

void LDA(int M, int F) {
    load(rA, M, F);
}

void LDX(int M, int F) {
    load(rX, M, F);
}

void LDi(int i, int M, int F) {
    int L = F / 8;
    int R = F % 8;

    for (int i = R - 1; i >= L - 1; i--) {
        if ((i == 0 || i == 1 || i == 2) && memory[M - 1].a[i] != 0) {
            return;
        }
    }
    load(rI[i], M, F);
}

void LDAN(int M, int F) {
    if (memory[M - 1].myBool == 0) {
        rA.myBool = 1;
    } else memory[M - 1].myBool = 0;
    LDA(M, F);
    if (memory[M - 1].myBool == 0) {
        rA.myBool = 1;
    } else memory[M - 1].myBool = 0;
}

void LDXN(int M, int F) {
    if (memory[M - 1].myBool == 0) {
        rA.myBool = 1;
    } else memory[M - 1].myBool = 0;
    LDX(M, F);
    if (memory[M - 1].myBool == 0) {
        rA.myBool = 1;
    } else memory[M - 1].myBool = 0;
}

void LDiN(int i, int M, int F) {
    if (memory[M - 1].myBool == 0) {
        rA.myBool = 1;
    } else memory[M - 1].myBool = 0;
    LDi(i, M, F);
    if (memory[M - 1].myBool == 0) {
        rA.myBool = 1;
    } else memory[M - 1].myBool = 0;
}

void save(Word x, int M, int F) {
    M--;
    int L = F / 8;
    int R = F % 8;
    if (L == 0) {
        memory[M].myBool = x.myBool;
        L++;
    }
    for (int i = L - 1; i < R - 1; i++) {
        memory[M].a[i] = x.a[i];
    }
    M++;
}

void STA(int M, int F) {
    save(rA, M, F);
}

void STX(int M, int F) {
    save(rX, M, F);
}

void STi(int i, int M, int F) {
    save(rI[i], M, F);
}

void STJ(int M, int F) {
    int L = F / 8;
    int R = F % 8;
    memory[M - 1].myBool = 1;
    if (L <= 1 && R >= 1) memory[M - 1].a[0] = rJ.a[3];
    if (L <= 2 && R >= 2) memory[M - 1].a[1] = rJ.a[4];
}

void STZ(int M, int F) {
    int L = F / 8;
    int R = F % 8;
    if (L == 0) L++;
    for (int i = L - 1; i < R - 1; i++) {
        memory[M - 1].a[i] = 0;
    }
}

void ADD(int M, int F) {
    Word newW;
    newW = getPartOfWord(memory[M - 1], F);
    rA = add(rA, newW);
}

void SUB(int M, int F) {
    Word newW;
    newW = getPartOfWord(memory[M - 1], F);
    rA = subtract(rA, newW);
}

void MUL(int M, int F) {
    Word newW;
    newW = getPartOfWord(memory[M - 1], F);
    rX = mutiplyX(rA, newW);
    rA = mutiplyA(rA, newW);
}

void DIV(int M, int F) {
    int max = g * g * g * g * g;
    long v = (long) getValue_Word(rA) * max + (long) getValue_Word(rX);
    Word newW;
    newW = getPartOfWord(memory[M - 1], F);
    int quotite = v / getValue_Word(newW);
    int remainder = v % getValue_Word(newW);
    rA = getWord(quotite);
    rX = getWord(remainder);
}

void ENTA(int M) {
    rA = getWord(M);
}

void ENTX(int M) {
    rX = getWord(M);
}

void ENTi(int i, int M) {
    rI[i] = getWord(M);
}

void ENNA(int M) {
    rA = getWord(M);
    if (rA.myBool == 1) rA.myBool = 0;
    else rA.myBool = 1;
}

void ENNX(int M) {
    rX = getWord(M);
    if (rX.myBool == 1) rX.myBool = 0;
    else rX.myBool = 1;
}

void ENNi(int i, int M) {
    rI[i] = getWord(M);
    if (rI[i].myBool == 1) rI[i].myBool = 0;
    else rI[i].myBool = 1;
}

void INCA(int M) {
    rA = getWord(getValue_Word(rA) + M);
}

void INCX(int M) {
    rX = getWord(getValue_Word(rX) + M);
}

void INCi(int i, int M) {
    int v = getValue_Word(rI[i]) + M;
    if (v >= g * g * g * g * g) {
        return;
    } else {
        rI[i] = getWord(v);
    }
}

void DECA(int M) {
    rA = getWord(getValue_Word(rA) - M);
}

void DECX(int M) {
    rX = getWord(getValue_Word(rX) - M);
}

void DECi(int i, int M) {
    int v = getValue_Word(rI[i]) - M;
    if (v <= -g * g * g * g * g) {
        return;
    } else {
        rI[i] = getWord(v);
    }
}

void CMPA(int M, int F) {
    Word w1 = getPartOfWord(rA, F);
    Word w2 = getPartOfWord(memory[M - 1], F);
    if ((F / 8) == 0) {
        w1.myBool = 1;
        w2.myBool = 1;
    }
    if (getValue_Word(w1) > getValue_Word(w2)) {
        compare = 1;
    } else if (getValue_Word(w1) == getValue_Word(w2)) {
        compare = 0;
    } else compare = -1;
}

void CMPX(int M, int F) {
    Word w1 = getPartOfWord(rX, F);
    Word w2 = getPartOfWord(memory[M - 1], F);
    if ((F / 8) == 0) {
        w1.myBool = 1;
        w2.myBool = 1;
    }
    if (getValue_Word(w1) > getValue_Word(w2)) {
        compare = 1;
    } else if (getValue_Word(w1) == getValue_Word(w2)) {
        compare = 0;
    } else compare = -1;
}

void CMPi(int i, int M, int F) {
    Word w1 = getPartOfWord(rI[i], F);
    Word w2 = getPartOfWord(memory[M - 1], F);
    if ((F / 8) == 0) {
        w1.myBool = 1;
        w2.myBool = 1;
    }
    if (getValue_Word(w1) > getValue_Word(w2)) {
        compare = 1;
    } else if (getValue_Word(w1) == getValue_Word(w2)) {
        compare = 0;
    } else compare = -1;
}

void JMP(int M) {
    rJ = getWord(current + 1);
    current = M;
}

void JSJ(int M) {
    current = M;
}

void JOV(int M) {
    if (overflow == 1) {
        overflow = 0;
        JMP(M);
    }
}

void JNOV(int M) {
    if (overflow == 0) {
        overflow = 0;
        JMP(M);
    }
}

void JL(int M) {
    if (compare == -1) {
        JMP(M);
    }
}

void JE(int M) {
    if (compare == 0) {
        JMP(M);
    }
}

void JG(int M) {
    if (compare == 1) {
        JMP(M);
    }
}

void JGE(int M) {
    if (compare == 1 || compare == 0) {
        JMP(M);
    }
}

void JNE(int M) {
    if (compare != 0) {
        JMP(M);
    }
}

void JLE(int M) {
    if (compare == 0 || compare == -1) {
        JMP(M);
    }
}

void JAN(int M) {
    if (rA.myBool == 0) {
        JMP(M);
    }
}

void JAZ(int M) {
    if (getValue_Word(rA) == 0) {
        JMP(M);
    }
}

void JAP(int M) {
    if (rA.myBool == 1) {
        JMP(M);
    }
}

void JANN(int M) {
    if (rA.myBool != 0) {
        JMP(M);
    }
}

void JANZ(int M) {
    if (getValue_Word(rA) != 0) {
        JMP(M);
    }
}

void JANP(int M) {
    if (rA.myBool != 1) {
        JMP(M);
    }
}

void JXN(int M) {
    if (rX.myBool == 0) {
        JMP(M);
    }
}

void JXZ(int M) {
    if (getValue_Word(rX) == 0) {
        JMP(M);
    }
}

void JXP(int M) {
    if (rX.myBool == 1) {
        JMP(M);
    }
}

void JXNN(int M) {
    if (rX.myBool != 0) {
        JMP(M);
    }
}

void JXNZ(int M) {
    if (getValue_Word(rX) != 0) {
        JMP(M);
    }
}

void JXNP(int M) {
    if (rX.myBool != 1) {
        JMP(M);
    }
}

void JiN(int i, int M) {
    if (rI[i].myBool == 0) {
        JMP(M);
    }
}

void JiZ(int i, int M) {
    if (getValue_Word(rI[i]) == 0) {
        JMP(M);
    }
}

void JiP(int i, int M) {
    if (rI[i].myBool == 1) {
        JMP(M);
    }
}

void JiNN(int i, int M) {
    if (rI[i].myBool != 0) {
        JMP(M);
    }
}

void JiNZ(int i, int M) {
    if (getValue_Word(rI[i]) != 0) {
        JMP(M);
    }
}

void JiNP(int i, int M) {
    if (rI[i].myBool != 1) {
        JMP(M);
    }
}

void SRA(int M) {
    if (M >= 0) {
        for (int j = 0; j < M; ++j) {
            for (int i = 4; i > 0; i--) {
                rA.a[i] = rA.a[i - 1];
            }
            rA.a[0] = 0;
        }
    }

}

void SLA(int M) {
    if (M >= 0) {
        for (int j = 0; j < M; ++j) {
            for (int i = 0; i < 4; ++i) {
                rA.a[i] = rA.a[i + 1];
            }
            rA.a[4] = 0;
        }
    }

}

void SLAX(int M) {
    if (M >= 0) {
        for (int j = 0; j < M; ++j) {
            for (int i = 0; i < 4; ++i) {
                rA.a[i] = rA.a[i + 1];
            }
            rA.a[4] = rX.a[0];
            for (int i = 0; i < 4; ++i) {
                rX.a[i] = rX.a[i + 1];
            }
            rX.a[4] = 0;
        }

    }
}

void SRAX(int M) {
    if (M >= 0) {
        for (int j = 0; j < M; ++j) {
            for (int i = 4; i > 0; i--) {
                rX.a[i] = rX.a[i - 1];
            }
            rX.a[0] = rA.a[4];
            for (int i = 4; i > 0; i--) {
                rA.a[i] = rA.a[i - 1];
            }
            rA.a[0] = 0;
        }
    }
}

void SLC(int M) {
    if (M >= 0) {
        int m = 0;
        for (int j = 0; j < M; ++j) {
            m = rA.a[0];
            for (int i = 0; i < 4; ++i) {
                rA.a[i] = rA.a[i + 1];
            }
            rA.a[4] = rX.a[0];
            for (int i = 0; i < 4; ++i) {
                rX.a[i] = rX.a[i + 1];
            }
            rX.a[4] = m;
        }

    }
}

void SRC(int M) {
    if (M >= 0) {
        int m = 0;
        for (int j = 0; j < M; ++j) {
            m = rX.a[4];
            for (int i = 4; i > 0; i--) {
                rX.a[i] = rX.a[i - 1];
            }
            rX.a[0] = rA.a[4];
            for (int i = 4; i > 0; i--) {
                rA.a[i] = rA.a[i - 1];
            }
            rA.a[0] = m;
        }
    }
}

static const char allChar[56] = {
        ' ', 'A', 'B', 'C', 'D', 'E',
        'F', 'G', 'H', 'I', ' ', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
        'R', ' ', ' ', 'S', 'T', 'U', 'V', 'W',
        'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        '.', ',', '(', ')', '+', '-', '*', '/', '=', '$', '<', '>', '@', ';', ':', '\''
};

int getIndex(char tc) {
    int index = -1;
    for (int i = 0; i < 56; i++) {
        if (tc == allChar) {
            index = i;
            break;
        }
    }
    return index;
}

int main(void) {
    Word a;
    a = getWord(87);
    a = getPartOfWord(a, 3);
    printf("%d", a.a[3]);

}