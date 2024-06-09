#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>

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
    int bodies_count, Body *bodies, Vector3 *accelerations,
    int offset, int length
)
{
    for (int i = offset; i < offset + length; ++i)
        for (int j = 0; j < bodies_count; ++j) {
            if (i == j)
                accelerations[i * bodies_count + j].x =
                    accelerations[i * bodies_count + j].y =
                    accelerations[i * bodies_count + j].z = 0.0;
            else
                accelerations[i * bodies_count + j] = induced_acceleration(
                    gravitation_const, body_radius, bodies[i], bodies[j]
                );
            printf("%d %d %p\n", i, j, accelerations);
        }
}

void master_process(char *task_file_name)
{
    double gravitation_const, body_radius, model_delta_t;
    int bodies_count, simulation_steps;

    FILE *task_file = fopen(task_file_name, "r");
    fscanf(
        task_file, "%lf %lf %lf %d %d",
        &gravitation_const, &body_radius, &model_delta_t,
        &bodies_count, &simulation_steps
    );
    
    Body bodies[bodies_count];
    for (int i = 0; i < bodies_count; ++i)
        bodies[i] = read_body(task_file);
    
    fclose(task_file);
    
    
}

void slave_process()
{

}

int main(int argc, char** argv)
{
    MPI_Init(NULL, NULL);

    // create MPI types for Vector3
    const int n_items_vec3 = 3;
    int block_lengths_vec3[] = {1, 1, 1};
    MPI_Datatype types_vec3[] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE },
        mpi_vector3;
    MPI_Aint offsets_vec3[] = {
        offsetof(Vector3, x), offsetof(Vector3, y), offsetof(Vector3, z)
    };
    MPI_Type_create_struct(n_items_vec3, block_lengths_vec3, offsets_vec3, types_vec3, &mpi_vector3);
    MPI_Type_commit(&mpi_vector3);

    int size, rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
        Vector3 v = { 1.0, 2.0, 3.0 };

        const int dest = 1;
        MPI_Send(&v, 1, mpi_vector3, dest, 0, MPI_COMM_WORLD);

        printf("Rank %d: sent structure vector\n", rank);
    }
    if (rank == 1) {
        MPI_Status status;
        const int src = 0;

        Vector3 v;

        MPI_Recv(&v, 1, mpi_vector3, src, 0, MPI_COMM_WORLD, &status);
        write_vector(stdout, v);
        printf("\n");
    }

    MPI_Type_free(&mpi_vector3);
    MPI_Finalize();

    return 0;
}