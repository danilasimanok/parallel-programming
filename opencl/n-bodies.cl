typedef struct __attribute__ ((packed)) Vector3 {
    float x;
    float y;
    float z;
} Vector3;

Vector3 plus(Vector3 v1, Vector3 v2)
{
	Vector3 sum = { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	return sum;
}

Vector3 minus(Vector3 v1, Vector3 v2)
{
	Vector3 delta_r = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	return delta_r;
}

Vector3 multiply(float a, Vector3 v)
{
	Vector3 product = { a * v.x, a * v.y, a * v.z };
	return product;
}

float absolute(Vector3 v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

__kernel void dist(
    __global Vector3 *a,
    __global Vector3 *b,
    __global float *c
)
{
    Vector3 diff = minus(*b, *a);
    *c = absolute(diff);
}