
#ifndef _R_VEC_H
#define _R_VEC_H

typedef struct
{
    int x, y;
    int w, h;
} rect_t;

typedef float vec_t;

typedef vec_t vec2_t[2];        // 2 space

typedef vec_t vec3_t[3];        // 3 space

typedef vec_t vec4_t[4];        // colours

typedef int veci_t;

typedef veci_t veci2_t[2];

typedef veci_t veci3_t[3];

typedef veci_t veci4_t[4];

#define vec2X(a) 		((a)[0])
#define vec2Y(a) 		((a)[1])
#define vec2Copy(a,b) 		((b)[0] = (a)[0], (b)[1] = (a)[1])
#define vec3Copy(a,b) 		((b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2])
#define vec4Copy(a,b) 		((b)[0] = (a)[0], (b)[1] = (a)[1], (b)[2] = (a)[2], (b)[3] = (a)[3])
//#define vec2Add(a,b,c) ((c)[0]=(a)[0], (c)[1]=(a)[1], (c)[0]+=(b)[0], (c)[1]+=(b)[1])
#define vec2Clear(a) 		((a)[0]=0, (a)[1]=0)
#define vec3Clear(a) 		((a)[0]=0, (a)[1]=0, (a)[2]=0)
#define vec2Set(a, x, y) 	((a)[0]=x, (a)[1]=y)
#define vec3Set(a, x, y, z) 	((a)[0]=x, (a)[1]=y, (a)[2]=z)
#define vec4Set(a, r, g, b, d)	((a)[0]=r, (a)[1]=g, (a)[2]=b, (a)[3]=d)

#define vec2Equal(a, b)	(a)[0]==(b)[0] && (a)[1]==(b)[1]

#define vec2Mul(a,b,c) 		((c)[0]=(a)[0] * (b)[0], (c)[1]=(a)[1] * (b)[1])

#define vec2Add(a,b,c) 		((c)[0]=(a)[0] + (b)[0], (c)[1]=(a)[1] + (b)[1])
#define vec2Addto(a,b) 		((b)[0]+=(a)[0], (b)[1]+=(a)[1])
#define vec3Add(a,b,c) 		((c)[0]=(a)[0] + (b)[0], (c)[1]=(a)[1] + (b)[1], (c)[2]=(a)[2] + (b)[2])
#define vec3Addto(a,b) 		((b)[0]+=(a)[0], (b)[1]+=(a)[1], (b)[2]+=(a)[2])
#define vec2MA(a,b,c,d) 	((d)[0]=(a)[0]+(c)[0]*(b), (d)[1]=(a)[1]+(c)[1]*(b))
#define vec3MA(a,b,c,d) 	((d)[0]=(a)[0]+(c)[0]*(b), (d)[1]=(a)[1]+(c)[1]*(b), (d)[2]=(a)[2]+(c)[2]*(b))
//#define vec2MA(a,b,c,d) ((d)[0]=(a)[0]+(c)[0]*(b), (d)[1]=(a)[1]+(c)[1]*(b))
#define vec2Subtract(a,b,c) 	((c)[0]=(a)[0]-(b)[0], (c)[1]=(a)[1]-(b)[1])
#define vec2Subtractfrom(a,b) 	((a)[0]=(a)[0]-(b)[0], (a)[1]=(a)[1]-(b)[1])
#define vec3Subtract(a,b,c) 	((c)[0]=(a)[0]-(b)[0], (c)[1]=(a)[1]-(b)[1], (c)[2]=(a)[2]-(b)[2])
#define vec2Scale(a,s,c) 	((c)[0]=(a)[0]*(s), (c)[1]=(a)[1]*(s))
// #define vec2ScaleTo(a,b)     ((b)[0]=(b)[0]*(a), (b)[1]=(b)[1]*(a))
#define vec3Scale(a,s,c) 	((c)[0]=(a)[0]*(s), (c)[1]=(a)[1]*(s), (c)[2]=(a)[2]*(s))
#define vec2Normal(a,b) 	((b)[0]=-(a)[1], (b)[1]= (a)[0])
//#define vec3Normal(a,b)       ((b)[0]=-(a)[1], (b)[1]= (a)[0])
#define vec2Normal2(a,b) 	((b)[0]=(a)[1], (b)[1]= -(a)[0])
#define vec2Snapvalue(a,s)	((a)[0] -= (int)(a)[0]%(s), (a)[1] -= (int)(a)[1]%(s))
#define vec2Snap(a) 		((a)[0]=((int)((a)[0])), (a)[1]=((int)((a)[1])))
#define vec2DimensionSpeed(vel, dimension) (vel[dimension] < 0 ? -vel[dimension] : vel[dimension])

#define vec3Clamp(a)		((a)[0]=(int)(a)[0], (a)[1]=(int)(a)[1], (a)[2]=(int)(a)[2])

#define vec2DotProduct(x,y)	(x[0]*y[0]+x[1]*y[1])
#define vec3DotProduct(x,y)	(x[0]*y[0]+x[1]*y[1]+x[2]*y[2])
#define vec3Cross(a,b,o)	(o[0] = (a)[1]*(b)[2] - (a)[2]*(b)[1], o[1] = (a)[2]*(b)[0] - (a)[0]*(b)[2], o[2] = (a)[0]*(b)[1] - (a)[1]*(b)[0])

#define vec2Dot(x,y) vec2DotProduct(x,y)
#define vec3Dot(x,y) vec3DotProduct(x,y)

#define vec3IsCleared(a) 	(0 == (a)[0] && 0 == (a)[1] && 0 == (a)[2])

//#define vec4Invert(a) ((a)[0] *= -1, (a)[1] *= -1, (a)[2] *= -1, (a)[3] *= -1)
#define vec4Invert(a) 		((a)[0] = 1 - (a)[0], (a)[1] = 1 - (a)[1], (a)[2] = 1 - (a)[2], (a)[3] *= 1)

#endif

/*--------------------------------------------------------------79-characters-*/
/* vim: set expandtab! : */
