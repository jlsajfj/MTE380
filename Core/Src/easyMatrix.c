/*************************************************************************
> File Name: easyMatrix.c
> Author: Zhang Yuteng
> Mail:
> Created Time: 2019年05月23日 星期四 21时57分31秒
************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "easyMatrix.h"

int isFiniteNumber(double d) {
    return (d<=DBL_MAX&&d>=-DBL_MAX);
}
struct easyMatrix* setMatrix(DATA_TYPE * const a,struct easyMatrix* c) {
    uint8 x = c->rows;
    uint8 y = c->cols;
    int t = x*y;
    for(int i=0;i<t;++i) {
        c->element[i] = a[i];
    }
    return c;
}

struct easyMatrix* copyMatrix(const struct easyMatrix* const a,
                              struct easyMatrix* c) {
    if(a->rows != c->rows) return NULL;
    if(a->cols != c->cols) return NULL;
    int t = a->rows*a->cols;
    for(int i=0;i<t;++i) {
        c->element[i] = a->element[i];
    }
    return c;
}

struct easyMatrix* transMatrix(const struct easyMatrix* const a,
                               struct easyMatrix* c) {
    if(a->rows != c->cols) return NULL;
    if(a->cols != c->rows) return NULL;
    int index = 0;
    int index_src = 0;
    for(uint8 ii=0;ii<a->cols;++ii) {
        index_src=ii;
        for(uint8 jj=0;jj<a->rows;++jj) {
            //c->element[index] = a->element[jj*a->cols+ii];
            c->element[index] = a->element[index_src];
            index++;
            index_src+=a->cols;
        }
    }
    return c;
}

struct easyMatrix* leftMatrix(uint8 x_i,uint8 y_i, const struct easyMatrix* const in, 
                              struct easyMatrix* out) {
    if(in->rows != in->cols) return NULL;
    if(out->rows != out->cols) return NULL;
    if(in->rows != (out->rows+1)) return NULL;
    int index = 0;
    int index_src = 0;
    uint8 x =in->rows;
    uint8 y =in->cols;
    for(uint8 kk=0;kk<x;++kk) {
        for(uint8 ww=0;ww<y;++ww) {
            if(!(kk==x_i||ww==y_i)) {
                //out->element[index] = in->element[kk*y+ww];
                out->element[index] = in->element[index_src];
                index++;
            }
            index_src++;
        }
    }
    return out;
}

struct easyMatrix* getLUMatrix(const struct easyMatrix* const A, 
                               struct easyMatrix* L,
                               struct easyMatrix* U) {
    DATA_TYPE s = 0;
    uint8 N = A->cols;
    int t = N*N;
    for(int i =0;i<t;i++) {
        L->element[i] = 1e-20;
        U->element[i] = 1e-20;
    }
    for(int i=0;i<N;i++) {
        L->element[i*N+i] = 1.0;
    }
    for(int i=0;i<N;i++) {
        for(int j=i;j<N;j++) {
            s = 0.0;
            for(int k=0;k<i;++k) {
                s+=L->element[i*N+k]*U->element[k*N+j];
            }
            U->element[i*N+j]= A->element[i*N+j] - s; 
        }
        for (int j = i + 1;j < N;j++) {
            s = 0.0;
            for (int k = 0; k < i; k++)
            {
                s += L->element[j*N+k] * U->element[k*N+i];
            }
            L->element[j*N+i] = (A->element[j*N+i] - s) / U->element[i*N+i];      //按列计算l值
        }
    }
    return L;
 }

struct easyMatrix* invLMatrix(const struct easyMatrix* const L, 
                              struct easyMatrix* L_inv) { 
    uint8 N = L->cols;
    if(N!=L->rows) {
        printf("L matrix is not a sqare matrix!\n");
        return NULL;
    }
    DATA_TYPE s;
    int t = N*N;
    for(int i =0;i<t;i++) {
        L_inv->element[i] = 1e-13;
    }
    for (uint8 i = 0;i < N;i++)  {
        L_inv->element[i*N+i] = 1;
    }
    for (uint8 i= 1;i < N;i++) {
        for (uint8 j = 0;j < i;j++) {
            s = 0;
            for (uint8 k = 0;k < i;k++) {
                s += L->element[i*N+k] * L_inv->element[k*N+j];
            }
            L_inv->element[i*N+j] = -s;
        }
    }
    return L_inv;
}
struct easyMatrix* invUMatrix(const struct easyMatrix* const U, 
                              struct easyMatrix* U_inv) { 
    uint8 N = U->cols;
    DATA_TYPE s;
    int t = N*N;
    for(int i =0;i<t;i++) {
        U_inv->element[i] = 1e-13;
    }
    for (uint8 i = 0;i < N;i++)                    //按列序，列内按照从下到上，计算u的逆矩阵
    {
        U_inv->element[i*N+i] = 1 / U->element[i*N+i];
    }
    for (uint8 i = 1;i < N;i++) {
        for (int j = i - 1;j >=0;j--) {
            s = 0;
            for (uint8 k = j + 1;k <= i;k++) {
                s += U->element[j*N+k] * U_inv->element[k*N+i];
            }
            U_inv->element[j*N+i] = -s / U->element[j*N+j];
        }
    }
    return U_inv;
}

struct easyMatrix* addMatrix(const struct easyMatrix* const a,const struct easyMatrix* const b, struct easyMatrix* c) {
    if(a->cols != b->cols) return NULL;
    if(a->rows != b->rows) return NULL;
    struct easyMatrix* obj = (struct easyMatrix*)a;
    int t = obj->rows*obj->cols;
    for(int i=0;i<t;++i) {
        c->element[i] = obj->element[i]+b->element[i];
    }
    return c;
}

struct easyMatrix* subMatrix(const struct easyMatrix* const a, 
                             const struct easyMatrix* const b,
                             struct easyMatrix* c) {
    if(a->cols != b->cols) return NULL;
    if(a->rows != b->rows) return NULL;
    struct easyMatrix* obj = (struct easyMatrix*)a;
    int t = obj->rows*obj->cols;
    for(int i=0;i<t;++i) {
        c->element[i] = obj->element[i]-b->element[i];
    }
    return c;
}

struct easyMatrix* scaleMatrix(DATA_TYPE scale, const struct easyMatrix* const a, struct easyMatrix* b) {
    int t = a->cols*a->rows;
    for (int i = 0;i<t;++i) {
        b->element[i] = a->element[i]*scale;
    }
    return b;
}

struct easyMatrix* multiMatrix(const struct easyMatrix* const a,
                               const struct easyMatrix* const b, 
                               struct easyMatrix* c) {
    if(NULL==c) return NULL;
    if(c == a || c == b) return NULL;
    if(a->cols != b->rows) return NULL;
    int count = 0;
    int t_cnt = 0;
    int z_cnt = 0;
    uint8 x = a->rows;
    uint8 y = a->cols;
    uint8 z = b->cols;
    for(uint8 i = 0;i<x;++i) {
        for(uint8 k = 0;k<z;++k) {
            c->element[count] = 0;
            z_cnt = 0;
            for(uint8 j = 0;j<y;++j) {
                c->element[count] += a->element[t_cnt+j]*b->element[z_cnt+k];
                z_cnt += z;
            }
            count++;
        }
        t_cnt+=y;
    }
    return c;
}

struct easyMatrix* zerosMatrix(struct easyMatrix* e) {
    int t = e->cols*e->rows;
    for(int i=0;i<t;++i) {
        e->element[i] = 0;
    }
    return e;
}

struct easyMatrix* eyesMatrix(struct easyMatrix* e) {
    if(e->rows != e->cols) return NULL;
    zerosMatrix(e);
    int index = 0;
    for(uint8 i=0;i<e->rows;++i) {
        e->element[index] = 1.0;
        index+=(e->cols);
        ++index;
    }
    return e;
}

void dumpMatrix(const struct easyMatrix* const e) {
    int count = 0;
    int x = e->rows;
    int y = e->cols;
    printf("rows is:%d, cols is:%d\n",x,y);
    for(uint8 i = 0;i<x;++i) {
        for(uint8 j = 0;j<y;++j) {
            printf("%f,",e->element[count]);
            ++count;
        }
        printf(";\n");
    }
    return;
}

struct easyMatrix *solveLMatrix(const struct easyMatrix *L, const struct easyMatrix *B, struct easyMatrix *X) {
    if(L->cols != X->rows || L->rows != B->rows) return NULL;
    if(X->cols != 1 || B->cols != 1) return NULL;

    uint8 N = L->cols;

    DATA_TYPE *l = L->element;
    DATA_TYPE *x = X->element;
    DATA_TYPE *b = B->element;

    for(uint8 i = 0; i < N; i++) {
        x[i] = b[i];
        for(uint8 j = 0; j < i; j++) {
            x[i] -= l[i*N+j] * x[j];
        }
        x[i] /= l[i*N+i];
    }

    return X;
}

struct easyMatrix *solveUMatrix(const struct easyMatrix *U, const struct easyMatrix *B, struct easyMatrix *X) {
    if(U->cols != U->rows) return NULL;
    if(U->cols != X->rows || U->rows != B->rows) return NULL;
    if(X->cols != 1 || B->cols != 1) return NULL;

    uint8 N = U->cols;

    DATA_TYPE *u = U->element;
    DATA_TYPE *x = X->element;
    DATA_TYPE *b = B->element;

    for(uint8 _i = N; _i > 0; _i--) {
        uint8 i = _i - 1;
        x[i] = b[i];
        for(uint8 j = i+1; j < N; j++) {
            x[i] -= u[i*N+j] * x[j];
        }
        x[i] /= u[i*N+i];
    }

    return X;
}
