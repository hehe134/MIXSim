#include <stdio.h>
#include <math.h>
#include "Word.h"
#include <string.h>

static Word rA;
static Word rX;
static Word rI[6];
static Word rJ;
static Word memory[4000];
static const int g = 64;
static int overflow = 0;
static int compare;
static int current;
static int C;
static int F1;
static int M1;
static int I;
static int boolHLT;


static Word TapeUnits[8][100];
static Word DiskOrDrumUnits[8][100];
static char CardReader[16][5];
static char CardPunch[16][5];
static char LinePrinter[24][5];
static char Typewriter[14][5];
static char PaperTape[14][5];

void load(Word x, int M, int F) {
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
        if ((i == 0 || i == 1 || i == 2) && memory[M].a[i] != 0) {
            return;
        }
    }
    load(rI[i], M, F);
}

void LDAN(int M, int F) {
    if (memory[M].myBool == 0) {
        rA.myBool = 1;
    } else memory[M].myBool = 0;
    LDA(M, F);
    if (memory[M].myBool == 0) {
        rA.myBool = 1;
    } else memory[M].myBool = 0;
}

void LDXN(int M, int F) {
    if (memory[M].myBool == 0) {
        rA.myBool = 1;
    } else memory[M].myBool = 0;
    LDX(M, F);
    if (memory[M].myBool == 0) {
        rA.myBool = 1;
    } else memory[M].myBool = 0;
}

void LDiN(int i, int M, int F) {
    if (memory[M].myBool == 0) {
        rA.myBool = 1;
    } else memory[M].myBool = 0;
    LDi(i, M, F);
    if (memory[M].myBool == 0) {
        rA.myBool = 1;
    } else memory[M].myBool = 0;
}

void save(Word x, int M, int F) {
    int L = F / 8;
    int R = F % 8;
    if (L == 0) {
        memory[M].myBool = x.myBool;
        L++;
    }
    for (int i = L - 1; i < R ; i++) {
        memory[M].a[i] = x.a[i];
    }
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
    if (L <= 1 && R >= 1) memory[M].a[0] = rJ.a[3];
    if (L <= 2 && R >= 2) memory[M].a[1] = rJ.a[4];
}

void STZ(int M, int F) {
    int L = F / 8;
    int R = F % 8;
    if (L == 0) L++;
    for (int i = L - 1; i < R - 1; i++) {
        memory[M].a[i] = 0;
    }
}

void ADD(int M, int F) {
    Word newW;
    newW = getPartOfWord(memory[M], F);
    rA = add(rA, newW);
}

void SUB(int M, int F) {
    Word newW;
    newW = getPartOfWord(memory[M], F);
    rA = subtract(rA, newW);
}

void MUL(int M, int F) {
    Word newW;
    newW = getPartOfWord(memory[M], F);
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
    Word w2 = getPartOfWord(memory[M], F);
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
static const char allNum[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

int getIndex(char tc) {
    int index = -1;
    for (int i = 0; i < 56; i++) {
        if (tc == allChar[i]) {
            index = i;
            break;
        }
    }
    return index;
}

void output(Word w) {
    char c[11] = "";
    char c1[2];
    for (int i = 0; i < 5; i++) {
        c1[0] = allChar[w.a[i]];
        c1[1] = '\0';
        strcat(c, c1);
    }
    printf("%s\n", c);
}

void printWord(Word w) {
    char c[20] = "";
    if (w.myBool == 0) strcat(c, "- ");
    else
        strcat(c, "+ ");
    char c1[3];
    for (int i = 0; i < 5; i++) {
      //  if (w.a[i] < 10) { strcat(c, "0"); }
       // c1[0] = w.a[i];
       // c1[1] = '\0';
       c1[0]=allNum[w.a[i]/10];
       c1[1]=allNum[w.a[i]%10];
       c1[2]='\0';
        strcat(c, c1);
        strcat(c, " ");
    }
    printf("%s\n", c);
}

int sizeOfDeivice(int F) {
    int size = 0;
    //Tape units (devices 0…7)
    if (F >= 0 && F < 8) {
        size = 100;
    }
        //Disk or drum units (devices 8…15)
    else if (F > 7 && F < 16) {
        size = 100;
    }
        //Card reader (device 16)
    else if (F == 16) {
        size = 16;
    }
        //Card punch (device 17)
    else if (F == 17) {
        size = 14;
    }
        //Line printer (device 18)
    else if (F == 18) {
        size = 24;
    }
        //Typewriter terminal (device 19)
    else if (F == 19) {
        size = 14;
    }
        //Paper tape (device 20)
    else if (F == 20) {
        size = 14;
    }
    return size;
}

void OUT(int M, int F) {
    for (int i = M; i < M + sizeOfDeivice(F); i++) {
        if (F > 15) output(memory[i]);
        else if (F >= 0 && F < 16) printWord(memory[i]);
    }
}

Word input(char c1[]) {
    Word w;
    w.myBool = 1;
    for (int i = 0; i < 5; i++) {
        w.a[i] = getIndex(c1[i]);
    }
    return w;
}

void IN(int M, int F) {
    int k = M;
    for (int i = 0; i < sizeOfDeivice(F); i++) {
        if (F == 16) {
            memory[k] = input(CardReader[i]);
        } else if (F == 17) {
            memory[k] = input(CardPunch[i]);
        } else if (F == 18) {
            memory[k] = input(LinePrinter[i]);
        } else if (F == 19) {
            memory[k] = input(Typewriter[i]);
        } else if (F == 20) {
            memory[k] = input(PaperTape[i]);
        } else if (F >= 0 && F < 8) {
            memory[k] = TapeUnits[F][i];
        } else if (F > 7 && F < 16) {
            memory[k] = DiskOrDrumUnits[F][i];
        }
        k++;
    }
}

void NUM() {
    int value = 0;
    int flag = rA.myBool;
    for (int i = 0; i < 5; i++) {
        value = value * 10 + (rA.a[i] % 10);
    }
    for (int i = 0; i < 5; i++) {
        value = value * 10 + (rX.a[i] % 10);
    }

    rA = getWord(value);
    rA.myBool = flag;
}

const int row() {
    FILE *fp;
    int flag = 0, file_row = 0, count = 0;
    if ((fp = fopen("input.txt", "r")) == NULL)
        return -1;
    while (!feof(fp)) {
        flag = fgetc(fp);
        if (flag == '\n')
            count++;
    }
    file_row = count + 1; //加上最后一行
    printf("row = %d\n", file_row);
    fclose(fp);
    const int count1 = count;
    return count1;
}

static char command[500][50];

void read() {

    char szTest[50] = {0};
    int len = 0;

    FILE *fp = fopen("input.txt", "r");
    if (NULL == fp) {
        printf("failed to open txt\n");
        return;
    }
    int i = 0;
    while (!feof(fp)) {
        memset(szTest, 0, sizeof(szTest));
        fgets(szTest, sizeof(szTest) - 1, fp);
        printf("%s", szTest);

        strcpy(command[i], szTest);
        i++;
    }
    fclose(fp);
}

void carryOut() {
    if (C == 8) {
        LDA(M1, F1);
    } else if (C > 8 && C < 15) {
        LDi(C - 8, M1, F1);
    } else if (C == 15) {
        LDX(M1, F1);
    } else if (C == 16) {
        LDAN(M1, F1);
    } else if (C == 23) {
        LDXN(M1, F1);
    } else if (C > 16 && C < 23) {
        LDiN(C - 16, M1, F1);
    } else if (C == 24) {
        STA(M1, F1);
    } else if (C == 31) {
        STX(M1, F1);
    } else if (C > 24 && C < 31) {
        STi(C - 24, M1, F1);
    } else if (C == 32) {
        STJ(M1, F1);
    } else if (C == 33) {
        STZ(M1, F1);
    } else if (C == 1) {
        ADD(M1, F1);
    } else if (C == 2) {
        SUB(M1, F1);
    } else if (C == 3) {
        MUL(M1, F1);
    } else if (C == 4) {
        DIV(M1, F1);
    } else if (C == 48) {
        if (F1 == 2) ENTA(M1);
        else if (F1 == 3) ENNA(M1);
        else if (F1 == 0) INCA(M1);
        else if (F1 == 1) DECA(M1);
    } else if (C == 55) {
        if (F1 == 2) ENTX(M1);
        else if (F1 == 3) ENNX(M1);
        else if (F1 == 0) INCX(M1);
        else if (F1 == 1) DECX(M1);
    } else if (C > 48 && C < 55) {
        if (F1 == 2) ENTi(C - 48, M1);
        else if (F1 == 3) ENNi(C - 48, M1);
        else if (F1 == 0) INCi(C - 48, M1);
        else if (F1 == 1) DECi(C - 48, M1);
    } else if (C == 56) {
        CMPA(M1, F1);
    } else if (C == 63) {
        CMPX(M1, F1);
    } else if (C > 56 && C < 63) {
        CMPi(C - 56, M1, F1);
    } else if (C == 39) {
        if (F1 == 0) JMP(M1);
        else if (F1 == 1) JSJ(M1);
        else if (F1 == 2) JOV(M1);
        else if (F1 == 3) JNOV(M1);
        else if (F1 == 4) JL(M1);
        else if (F1 == 5) JE(M1);
        else if (F1 == 6) JG(M1);
        else if (F1 == 7) JGE(M1);
        else if (F1 == 8) JNE(M1);
        else if (F1 == 9) JLE(M1);
    } else if (C == 40) {
        switch (F1) {
            case 0:
                JAN(M1);
                break;
            case 1:
                JAZ(M1);
                break;
            case 2:
                JAP(M1);
                break;
            case 3:
                JANN(M1);
                break;
            case 4:
                JANZ(M1);
                break;
            case 5:
                JANP(M1);
                break;
        }
    } else if (C == 47) {
        switch (F1) {
            case 0:
                JXN(M1);
                break;
            case 1:
                JXZ(M1);
                break;
            case 2:
                JXP(M1);
                break;
            case 3:
                JXNN(M1);
                break;
            case 4:
                JXNZ(M1);
                break;
            case 5:
                JXNP(M1);
                break;
        }
    } else if (C > 40 && C < 47) {
        switch (F1) {
            case 0:
                JiN(C - 40, M1);
                break;
            case 1:
                JiZ(C - 40, M1);
                break;
            case 2:
                JiP(C - 40, M1);
                break;
            case 3:
                JiNN(C - 40, M1);
                break;
            case 4:
                JiNZ(C - 40, M1);
                break;
            case 5:
                JiNP(C - 40, M1);
        }
    } else if (C == 6) {
        switch (F1) {
            case 0:
                SLA(M1);
                break;
            case 1:
                SRA(M1);
                break;
            case 2:
                SLAX(M1);
                break;
            case 3:
                SRAX(M1);
                break;
            case 4:
                SLC(M1);
                break;
            case 5:
                SRC(M1);
                break;
        }
    } else if (C == 0) {
        boolHLT = 1;
    } else if (C == 36) {
        IN(M1, F1);
    } else if (C == 37) {
        OUT(M1, F1);
    }
    current++;

}

void run() {
    char *st1;
    F1 = -1;
    st1 = strtok(command[current], " ");
    char st[6];
    strcpy(st, st1);
    if (strcmp(st, "LDA") == 0) {
        C = 8;
    } else if (strcmp(st, "LD1") == 0) {
        C = 9;
    } else if (strcmp(st, "LD2") == 0) {
        C = 10;
    } else if (strcmp(st, "LD3") == 0) {
        C = 11;
    } else if (strcmp(st, "LD4") == 0) {
        C = 12;
    } else if (strcmp(st, "LD5") == 0) {
        C = 13;
    } else if (strcmp(st, "LD6") == 0) {
        C = 14;
    } else if (strcmp(st, "LDX") == 0) {
        C = 15;
    } else if (strcmp(st, "LDAN") == 0) {
        C = 16;
    } else if (strcmp(st, "LD1N") == 0) {
        C = 17;
    } else if (strcmp(st, "LD2N") == 0) {
        C = 18;
    } else if (strcmp(st, "LD3N") == 0) {
        C = 19;
    } else if (strcmp(st, "LD4N") == 0) {
        C = 20;
    } else if (strcmp(st, "LD5N") == 0) {
        C = 21;
    } else if (strcmp(st, "LD6N") == 0) {
        C = 22;
    } else if (strcmp(st, "LDXN") == 0) {
        C = 23;
    } else if (strcmp(st, "STA") == 0) {
        C = 24;
    } else if (strcmp(st, "STX") == 0) {
        C = 31;
    } else if (strcmp(st, "ST1") == 0) {
        C = 25;
    } else if (strcmp(st, "ST2") == 0) {
        C = 26;
    } else if (strcmp(st, "ST3") == 0) {
        C = 27;
    } else if (strcmp(st, "ST4") == 0) {
        C = 28;
    } else if (strcmp(st, "ST5") == 0) {
        C = 29;
    } else if (strcmp(st, "ST6") == 0) {
        C = 30;
    } else if (strcmp(st, "STJ") == 0) {
        C = 32;
    } else if (strcmp(st, "STZ") == 0) {
        C = 33;
    } else if (strcmp(st, "ADD") == 0) {
        C = 1;
    } else if (strcmp(st, "SUB") == 0) {
        C = 2;
    } else if (strcmp(st, "MUL") == 0) {
        C = 3;
    } else if (strcmp(st, "DIV") == 0) {
        C = 4;
    } else if (st[0] == 'E' && st[1] == 'N') {
        if (st[2] == 'T') {
            if (st[3] == 'A') {
                C = 48;
                F1 = 2;
            } else if (st[3] == 'X') {
                C = 55;
                F1 = 2;
            } else if (st[3] == '1') {
                C = 49;
                F1 = 2;
            } else if (st[3] == '2') {
                C = 50;
                F1 = 2;
            } else if (st[3] == '3') {
                C = 51;
                F1 = 2;
            } else if (st[3] == '4') {
                C = 52;
                F1 = 2;
            } else if (st[3] == '5') {
                C = 53;
                F1 = 2;
            } else if (st[3] == '6') {
                C = 54;
                F1 = 2;
            }
        } else if (st[2] == 'N') {
            if (st[3] == 'A') {
                C = 48;
                F1 = 3;
            } else if (st[3] == 'X') {
                C = 55;
                F1 = 3;
            } else if (st[3] == '1') {
                C = 49;
                F1 = 3;
            } else if (st[3] == '2') {
                C = 50;
                F1 = 3;
            } else if (st[3] == '3') {
                C = 51;
                F1 = 3;
            } else if (st[3] == '4') {
                C = 52;
                F1 = 3;
            } else if (st[3] == '5') {
                C = 53;
                F1 = 3;
            } else if (st[3] == '6') {
                C = 54;
                F1 = 3;
            }
        }
    } else if (st[0] == 'I') {
        if (st[3] == 'A') {
            C = 48;
            F1 = 0;
        } else if (st[3] == 'X') {
            C = 55;
            F1 = 0;
        } else if (st[3] == '1') {
            C = 49;
            F1 = 0;
        } else if (st[3] == '2') {
            C = 50;
            F1 = 0;
        } else if (st[3] == '3') {
            C = 51;
            F1 = 0;
        } else if (st[3] == '4') {
            C = 52;
            F1 = 0;
        } else if (st[3] == '5') {
            C = 53;
            F1 = 0;
        } else if (st[3] == '6') {
            C = 54;
            F1 = 0;
        }
    } else if (st[0] == 'D') {
        if (st[3] == 'A') {
            C = 48;
            F1 = 1;
        } else if (st[3] == 'X') {
            C = 55;
            F1 = 1;
        } else if (st[3] == '1') {
            C = 49;
            F1 = 1;
        } else if (st[3] == '2') {
            C = 50;
            F1 = 1;
        } else if (st[3] == '3') {
            C = 51;
            F1 = 1;
        } else if (st[3] == '4') {
            C = 52;
            F1 = 1;
        } else if (st[3] == '5') {
            C = 53;
            F1 = 1;
        } else if (st[3] == '6') {
            C = 54;
            F1 = 1;
        }
    } else if (st[0] == 'C') {
        if (st[3] == 'A') {
            C = 56;
        } else if (st[3] == 'X') {
            C = 63;
        } else if (st[3] == '1') {
            C = 57;
        } else if (st[3] == '2') {
            C = 58;
        } else if (st[3] == '3') {
            C = 59;
        } else if (st[3] == '4') {
            C = 60;
        } else if (st[3] == '5') {
            C = 61;
        } else if (st[3] == '6') {
            C = 62;
        }
    } else if (st[0] == 'J') {
        if (st[2] == 'P') {
            C = 39;
            F1 = 0;
        } else if (st[2] == 'J') {
            C = 39;
            F1 = 1;
        } else if (st[2] == 'V') {
            C = 39;
            F1 = 2;
        } else if (st[1] == 'N' && st[2] == 'O') {
            C = 39;
            F1 = 3;
        } else if (strcmp(st, "JL") == 0) {
            C = 39;
            F1 = 4;
        } else if (strcmp(st, "JE") == 0) {
            C = 39;
            F1 = 5;
        } else if (strcmp(st, "JG") == 0) {
            C = 39;
            F1 = 6;
        } else if (strcmp(st, "JGE") == 0) {
            C = 39;
            F1 = 7;
        } else if (strcmp(st, "JNE") == 0) {
            C = 39;
            F1 = 8;
        } else if (strcmp(st, "JLE") == 0) {
            C = 39;
            F1 = 9;
        } else if (strcmp(st, "JAN") == 0) {
            C = 40;
            F1 = 0;
        } else if (strcmp(st, "JAZ") == 0) {
            C = 40;
            F1 = 1;
        } else if (strcmp(st, "JAP") == 0) {
            C = 40;
            F1 = 2;
        } else if (strcmp(st, "JANN") == 0) {
            C = 40;
            F1 = 3;
        } else if (strcmp(st, "JANZ") == 0) {
            C = 40;
            F1 = 4;
        } else if (strcmp(st, "JANP") == 0) {
            C = 40;
            F1 = 5;
        } else if (strcmp(st, "JXN") == 0) {
            C = 47;
            F1 = 0;
        } else if (strcmp(st, "JXZ") == 0) {
            C = 47;
            F1 = 1;
        } else if (strcmp(st, "JXP") == 0) {
            C = 47;
            F1 = 2;
        } else if (strcmp(st, "JXNN") == 0) {
            C = 47;
            F1 = 3;
        } else if (strcmp(st, "JXNZ") == 0) {
            C = 47;
            F1 = 4;
        } else if (strcmp(st, "JXNP") == 0) {
            C = 47;
            F1 = 5;
        } else if (st[2] == 'N' && st[3] == 'N') {
            for (int i = 1; i < 7; i++) {
                if (st[1] == allNum[i]) {
                    C = 40 + i;
                    F1 = 3;
                    break;
                }
            }
        } else if (st[2] == 'N' && st[3] == 'Z') {
            for (int i = 1; i < 7; i++) {
                if (st[1] == allNum[i]) {
                    C = 40 + i;
                    F1 = 4;
                    break;
                }
            }
        } else if (st[2] == 'N' && st[3] == 'P') {
            for (int i = 1; i < 7; i++) {
                if (st[1] == allNum[i]) {
                    C = 40 + i;
                    F1 = 5;
                    break;
                }
            }
        } else if (st[2] == 'N') {
            for (int i = 1; i < 7; i++) {
                if (st[1] == allNum[i]) {
                    C = 40 + i;
                    F1 = 0;
                    break;
                }
            }
        } else if (st[2] == 'Z') {
            for (int i = 1; i < 7; i++) {
                if (st[1] == allNum[i]) {
                    C = 40 + i;
                    F1 = 1;
                    break;
                }
            }
        } else if (st[2] == 'P') {
            for (int i = 1; i < 7; i++) {
                if (st[1] == allNum[i]) {
                    C = 40 + i;
                    F1 = 2;
                    break;
                }
            }
        }
    } else if (strcmp(st, "SLA") == 0) {
        C = 6;
        F1 = 0;
    } else if (strcmp(st, "SRA") == 0) {
        C = 6;
        F1 = 1;
    } else if (strcmp(st, "SLAX") == 0) {
        C = 6;
        F1 = 2;
    } else if (strcmp(st, "SRAX") == 0) {
        C = 6;
        F1 = 3;
    } else if (strcmp(st, "SLC") == 0) {
        C = 6;
        F1 = 4;
    } else if (strcmp(st, "SRC") == 0) {
        C = 6;
        F1 = 5;
    } else if (strcmp(st, "NOP") == 0) {
        C = 0;
    } else if (strcmp(st, "HLT") == 0) {
        C = 5;
        F1 = 2;
    } else if (strcmp(st, "IN") == 0) {
        C = 36;
    } else if (strcmp(st, "OUT") == 0) {
        C = 37;
    } else if (strcmp(st, "NUM") == 0) {
        C = 5;
        F1 = 0;
    } else if (strcmp(st, "CHAR") == 0) {
        C = 5;
        F1 = 1;
    }


    char address[6];
    st1 = strtok(NULL, ",");
    if (st1 != NULL) {
        strcpy(address, st1);
        M1 = 0;
        for (int j = 0; j < 10; ++j) {
            if (address[0] == allNum[j]) {
                M1 += j * 1000;
                break;
            }
        }
        for (int j = 0; j < 10; ++j) {
            if (address[1] == allNum[j]) {
                M1 += j * 100;
                break;
            }
        }
        for (int j = 0; j < 10; ++j) {
            if (address[2] == allNum[j]) {
                M1 += j * 10;
                break;
            }
        }
        for (int j = 0; j < 10; ++j) {
            if (address[3] == allNum[j]) {
                M1 += j;
                break;
            }
        }
    }

    char I1[4];
    st1 = strtok(NULL, "(");

    if (st1 != NULL) {
        strcpy(I1, st1);
        I = 0;
        for (int j = 0; j < 10; ++j) {
            if (I1[0] == allNum[j]) {
                I += j * 10;
                break;
            }
        }
        for (int j = 0; j < 10; ++j) {
            if (I1[1] == allNum[j]) {
                I += j;
                break;
            }
        }
    }

    int FL = -1;
    int FR = -1;
    char FLR[6];
    st1 = strtok(NULL, "(");

    if (st1 != NULL) {
        strcpy(FLR, st1);
        for (int i = 0; i < 10; ++i) {
            if (FLR[0] == allNum[i]) {
                FL = i;
                break;
            }
        }
        for (int i = 0; i < 10; ++i) {
            if (FLR[2] == allNum[i]) {
                FR = i;
                break;
            }
        }
        if (FL != -1 && FR != -1) {
            F1 = FL * 8 + FR;
        } else if (F1 == -1) {
            F1 = 5;
        }
    }
}


int main(void) {

    reset(&rA);
    reset(&rX);

    for (int j = 0; j < 4000; ++j) {
        memory[j].myBool = 1;
        for (int i = 0; i < 5; ++i) {
            memory[j].a[i] = 0;
        }
    }
    read();

    printf("\n");
    while (boolHLT == 0) {
        run();
        carryOut();
        printf("rA: ");

        if (rA.myBool==0) printf("- ");
        else printf("+ ");
        for (int i = 0; i < 5; ++i) {
            printf("%d ", rA.a[i]);
        }
        printf("\n");
        printf("rX: ");
        if (rA.myBool==0) printf("- ");
        else printf("+ ");
        for (int i = 0; i < 5; ++i) {
            printf("%d ", rX.a[i]);
        }
        printf("\n");
    }
}