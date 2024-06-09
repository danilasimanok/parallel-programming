typedef struct __attribute__ ((packed)) Vector3 {
    float x;
    float y;
    float z;
} Vector3;

typedef struct __attribute__ ((packed)) Body {
	Vector3 position;
	Vector3 velocity;
	float mass;
} Body;

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

// induced by body_2 on body_1
Vector3 induced_acceleration(
	float g, float body_radius,
	Body body_1, Body body_2
)
{
	Vector3 delta_r = minus(body_2.position, body_1.position);
	Vector3 density = gravity_density(g, body_radius, delta_r);
	return multiply(body_2.mass, density);
}

__kernel void sol(
	__constant float *g,
    __constant float *body_radius,
	__constant int *bodies_count_ptr,
    __constant float *model_dt,
    __constant int *steps_count_ptr,
    __global Vector3 *accelerations,
    __global Body *bodies
)
{
    int i = get_global_id(0),
        j = get_global_id(1),
        bodies_count = *bodies_count_ptr,
        steps_count = *steps_count_ptr;
    
    for (int step = 0; step < steps_count; ++step) {
        // calculate accelerations
        if (i == j)
            accelerations[i * bodies_count + j].x =
                accelerations[i * bodies_count + j].y = 
                accelerations[i * bodies_count + j].z = 0.0;
        else
            accelerations[i * bodies_count + j] = induced_acceleration(
                *g, *body_radius, bodies[i], bodies[j]
            );
        
        barrier(CLK_GLOBAL_MEM_FENCE);

        // sum accelerations
        int length = bodies_count;
        while (length > 1) {
            int length_is_odd = length % 2;
            
            if (j < length / 2)
                accelerations[i * bodies_count + j] = plus(
                    accelerations[i * bodies_count + 2 * j],
                    accelerations[i * bodies_count + 2 * j + 1]
                );
            else if ((j == length / 2) && length_is_odd)
                accelerations[i * bodies_count + j] =
                    accelerations[i * bodies_count + length - 1];
            
            length /= 2;
            if (length_is_odd)
                length += 1;
            
            barrier(CLK_GLOBAL_MEM_FENCE);
        }

        // accelerate
        if (j == 0)
            bodies[i].velocity = plus(
                bodies[i].velocity,
                accelerations[i * bodies_count]
            );
        barrier(CLK_GLOBAL_MEM_FENCE);
        
        // move
        if (j == 0)
            bodies[i].position = plus(
                bodies[i].position,
                multiply(*model_dt, bodies[i].velocity)
            );
        }
        barrier(CLK_GLOBAL_MEM_FENCE);
}