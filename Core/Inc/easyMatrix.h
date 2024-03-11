/*************************************************************************
	> File Name: easyMatrix.h
	> Author: Zhang Yuteng
	> Mail:fellylanma@aliyun.com
	> Created Time: 2019年05月13日 星期一 22时06分13秒
 ************************************************************************/

#ifndef _MAGRIDE_PLANNING_EASYMATRIX_H_
#define _MAGRIDE_PLANNING_EASYMATRIX_H_

#include <stdlib.h>
typedef unsigned char uint8;
typedef double DATA_TYPE;


#define CREATE_MATRIX_ONSTACK(x,y,matrix,initval) \
struct easyMatrix matrix;\
DATA_TYPE val##x##N##y##N##matrix[x*y];\
    matrix.rows = x;\
    matrix.cols = y;\
    matrix.element = val##x##N##y##N##matrix;\
    if(initval!=NULL) setMatrix(initval, &(matrix))

struct easyMatrix {\
    uint8 rows,cols;\
    DATA_TYPE* element;
};\

struct easyMatrix* setMatrix(DATA_TYPE* const a,struct easyMatrix* c);

struct easyMatrix* copyMatrix(const struct easyMatrix* const a,struct easyMatrix* c);

struct easyMatrix* transMatrix(const struct easyMatrix* const a,struct easyMatrix* c);

struct easyMatrix* scaleMatrix(DATA_TYPE, const struct easyMatrix* const a, struct easyMatrix*);

struct easyMatrix* addMatrix(const struct easyMatrix* const a, const struct easyMatrix *const  b, struct easyMatrix * c);

struct easyMatrix* leftMatrix(uint8, uint8, const struct easyMatrix* const a, struct easyMatrix* b);

struct easyMatrix* subMatrix(const struct easyMatrix* const a, const struct easyMatrix* const  b, struct easyMatrix* c);

struct easyMatrix* multiMatrix(const struct easyMatrix* const a, const struct easyMatrix* const b, struct easyMatrix* c);

struct easyMatrix* zerosMatrix(struct easyMatrix* e);

struct easyMatrix* eyesMatrix(struct easyMatrix* e);

void dumpMatrix(const struct easyMatrix* const e);

struct easyMatrix* getLUMatrix(const struct easyMatrix* const A, struct easyMatrix* L,struct easyMatrix* U) ;

struct easyMatrix* invLMatrix(const struct easyMatrix* const L, struct easyMatrix* L_inv) ;
struct easyMatrix* invUMatrix(const struct easyMatrix* const U, struct easyMatrix* U_inv) ;

struct easyMatrix *solveLMatrix(const struct easyMatrix *L, const struct easyMatrix *B, struct easyMatrix *X);
struct easyMatrix *solveUMatrix(const struct easyMatrix *U, const struct easyMatrix *B, struct easyMatrix *X);

#endif//_MAGRIDE_PLANNING_EASYMATRIX_H_
