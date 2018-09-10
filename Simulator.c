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
    for (int i = L - 1; i < R - 1; i++) {
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
    char c[11] = "";
    if (w.myBool == 0) strcat(c, "-");
    else
        strcat(c, "+");
    char c1[2];
    for (int i = 0; i < 5; i++) {
        if (w.a[i] < 10) { strcat(c, "0"); }
        c1[0] = w.a[i];
        c1[1] = '\0';
        strcat(c, c1);

    }
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
    if ((fp = fopen("./input.txt", "r")) == NULL)
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

    FILE *fp = fopen("./input.txt", "r");
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

void run() {
    char *st;
    st = strtok(command[current], " ");
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
    } else if (strcmp(st, "ST1") == 0) {
        C = 25;
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
    } else if (strcmp(st, "ST5") == 0) {
        C = 29;
    }
}
int main(void) {
    Word a;
    a = getWord(87);
    a = getPartOfWord(a, 3);
    printf("%d", a.a[3]);
    read();
}