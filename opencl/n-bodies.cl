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

Vector3 gravity_density(
    float g, float body_radius,
    Vector3 delta_r
)
{
	float distance = absolute(delta_r);
	float denominator =
        distance > body_radius ? distance * distance : -distance * distance * distance;
	float abs_density = g / denominator;
	Vector3 density = {
		abs_density * delta_r.x / distance,
		abs_density * delta_r.y / distance,
		abs_density * delta_r.z / distance
	};
	return density;
}

__kernel void dens(
    __constant float *g,
    __constant float *body_radius,
    __constant Vector3 *v1,
    __constant Vector3 *v2,
    __global Vector3 *result
)
{
    Vector3 delta_r = minus(*v2, *v1);
    *result = gravity_density(*g, *body_radius, delta_r);
}