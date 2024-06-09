#include <stdio.h>
#include <math.h>
#include <time.h>

typedef struct Vector3 {
    double x;
    double y;
    double z;
} Vector3;

Vector3 read_vector(FILE *stream)
{
    Vector3 result;
    fscanf(
        stream, "%lf %lf %lf",
        &(result.x), &(result.y), &(result.z)
    );
    return result;
}

void write_vector(FILE *stream, Vector3 v)
{
    fprintf(stream, "(%lf, %lf, %lf)", v.x, v.y, v.z);
}

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

Vector3 multiply(double a, Vector3 v)
{
    Vector3 product = { a * v.x, a * v.y, a * v.z };
    return product;
}

double absolute(Vector3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 gravity_density(
    double gravitation_const, double body_radius,
    Vector3 delta_r
)
{
    double distance = absolute(delta_r);
    double denominator = distance > body_radius ? pow(distance, 2.0) : -pow(distance, 3.0);
    double abs_density = gravitation_const / denominator;
    Vector3 density = {
        abs_density * delta_r.x / distance,
        abs_density * delta_r.y / distance,
        abs_density * delta_r.z / distance
    };
    return density;
}

typedef struct Body {
    Vector3 position;
    Vector3 velocity;
    double mass;
} Body;

Body read_body(FILE *stream)
{
    double mass;
    fscanf(stream, "%lf", &mass);
    Body result = { read_vector(stream), read_vector(stream), mass };
    return result;
}

void write_body(FILE *stream, Body body)
{
    fprintf(stream, "body {\n\t'mass': %lf\n\t'position': ", body.mass);
    write_vector(stream, body.position);
    fprintf(stream, "\n\t'velocity': ");
    write_vector(stream, body.velocity);
    fprintf(stream, "\n}");
}

// induced by body_2 on body_1
Vector3 induced_acceleration(
    double gravitation_const, double body_radius,
    Body body_1, Body body_2
)
{
    Vector3 delta_r = minus(body_2.position, body_1.position);
    Vector3 density = gravity_density(gravitation_const, body_radius, delta_r);
    return multiply(body_2.mass, density);
}

void calculate_accelerations(
    double gravitation_const, double body_radius,
    int bodies_count, Body *bodies, Vector3 *accelerations
)
{
    for (int i = 0; i < bodies_count; ++i)
        for (int j = 0; j < bodies_count; ++j)
            if (i == j)
                accelerations[i * bodies_count + j].x =
                    accelerations[i * bodies_count + j].y = 
                    accelerations[i * bodies_count + j].z = 0.0;
            else
                accelerations[i * bodies_count + j] = induced_acceleration(
                    gravitation_const, body_radius, bodies[i], bodies[j]
                );
}

void accelerate(
    int bodies_count, Body *bodies, Vector3 *accelerations
)
{
    for (int i = 0; i < bodies_count; ++i)
        for (int j = 0; j < bodies_count; ++j)
            bodies[i].velocity = plus(
                bodies[i].velocity, accelerations[i * bodies_count + j]
            );
}

void move(
    double model_delta_t, int bodies_count, Body *bodies
)
{
    for (int i = 0; i < bodies_count; ++i)
        bodies[i].position = plus(
            bodies[i].position,
            multiply(model_delta_t, bodies[i].velocity)
        );
}

int main(int argc, char **argv)
{
    double gravitation_const, body_radius, model_delta_t;
    int bodies_count, simulation_steps;
    
    FILE *task_file = fopen(argv[1], "r");
    fscanf(
        task_file, "%lf %lf %lf %d %d",
        &gravitation_const, &body_radius, &model_delta_t,
        &bodies_count, &simulation_steps
    );
    
    Body bodies[bodies_count];
    for (int i = 0; i < bodies_count; ++i)
        bodies[i] = read_body(task_file);
            
    fclose(task_file);

    clock_t begin, end;
    begin = clock();
    
    Vector3 accelerations[bodies_count * bodies_count];
    for (int i = 0; i < simulation_steps; ++i) {
        calculate_accelerations(gravitation_const, body_radius, bodies_count, bodies, accelerations);
        accelerate(bodies_count, bodies, accelerations);
        move(model_delta_t, bodies_count, bodies);
    }

    end = clock();
    printf("Time taken: %lf sec\n", ((double) (end - begin)) / CLOCKS_PER_SEC);

    FILE *solution_file = fopen(argv[2], "w");
    for (int i = 0; i < bodies_count; ++i) {
        write_body(solution_file, bodies[i]);
        fprintf(solution_file, "\n");
    }
    fclose(solution_file);
    
    return 0;
}
