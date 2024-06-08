typedef struct __attribute__ ((packed)) Vector3 {
    float x;
    float y;
    float z;
} Vector3;

__kernel void add(
    __global Vector3 *a,
    __global Vector3 *b,
    __global Vector3 *c
)
{
    c->x = a->x + b->x;
    c->y = a->y + b->y;
    c->z = a->z + b->z; 
}