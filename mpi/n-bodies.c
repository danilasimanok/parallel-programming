#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>

#define MASTER_RANK 0

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

void accelerate(
    double gravitation_const, double body_radius,
    int bodies_count, Body *bodies,
    int offset, int subtask_size
)
{
    for (int i = offset; i < offset + subtask_size; ++i)
        for (int j = 0; j < bodies_count; ++j)
            if (i != j)
                bodies[i].velocity = plus(
                    bodies[i].velocity,
                    induced_acceleration(
                        gravitation_const, body_radius, bodies[i], bodies[j]
                    )
                );
}

void get_subtask_parameters(
    int bodies_count, int world_size, int p_rank,
    int *offset, int *subtask_size
)
{
    *subtask_size = bodies_count / world_size;    
    *offset = p_rank * (*subtask_size);
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

void master_process(
    MPI_Datatype mpi_body, int world_size,
    char *task_file_name, char *solution_file_name
)
{
    double g_radius_dt[3]; // gravitation_const, body_radius, model_delta_t
    int bcount_steps[2]; // bodies_count, simulation_steps

    FILE *task_file = fopen(task_file_name, "r");
    
    fscanf(
        task_file, "%lf %lf %lf %d %d",
        g_radius_dt, g_radius_dt + 1, g_radius_dt + 2,
        bcount_steps, bcount_steps + 1
    );
    
    Body bodies[bcount_steps[0]];
    for (int i = 0; i < bcount_steps[0]; ++i)
        bodies[i] = read_body(task_file);
    
    fclose(task_file);
    
    // broadcast parameters
    MPI_Bcast(g_radius_dt, 3, MPI_DOUBLE, MASTER_RANK, MPI_COMM_WORLD);
    MPI_Bcast(bcount_steps, 2, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

    int offset, subtask_size;
    get_subtask_parameters(bcount_steps[0], world_size, MASTER_RANK, &offset, &subtask_size);
    Body body_buff[subtask_size];

    double begin = MPI_Wtime(),
        end;

    for (int step = 0; step < bcount_steps[1]; ++step) {
        MPI_Bcast(bodies, bcount_steps[0], mpi_body, MASTER_RANK, MPI_COMM_WORLD);

        accelerate(g_radius_dt[0], g_radius_dt[1], bcount_steps[0], bodies, offset, subtask_size);

        for (int i = 0; i < subtask_size; ++i)
            body_buff[i] = bodies[offset + i];
        MPI_Gather(body_buff, subtask_size, mpi_body, bodies, subtask_size, mpi_body, MASTER_RANK, MPI_COMM_WORLD);

        MPI_Scatter(bodies, subtask_size, mpi_body, body_buff, subtask_size, mpi_body, MASTER_RANK, MPI_COMM_WORLD);

        move(g_radius_dt[2], subtask_size, body_buff);

        MPI_Gather(body_buff, subtask_size, mpi_body, bodies, subtask_size, mpi_body, MASTER_RANK, MPI_COMM_WORLD);
    }

    end = MPI_Wtime();
    printf("Time taken: %lf sec\n", end - begin);

    FILE *solution_file = fopen(solution_file_name, "w");
    for (int i = 0; i < bcount_steps[0]; ++i) {
        write_body(solution_file, bodies[i]);
        fprintf(solution_file, "\n");
    }
}

void slave_process(
    int p_rank, int world_size,
    MPI_Datatype mpi_body
)
{
    double g_radius_dt[3]; // gravitation_const, body_radius, model_delta_t
    int bcount_steps[2]; // bodies_count, simulation_steps

    // receive parameters
    MPI_Bcast(g_radius_dt, 3, MPI_DOUBLE, MASTER_RANK, MPI_COMM_WORLD);
    MPI_Bcast(bcount_steps, 2, MPI_INT, MASTER_RANK, MPI_COMM_WORLD);

    int offset, subtask_size;
    get_subtask_parameters(bcount_steps[0], world_size, p_rank, &offset, &subtask_size);

    Body bodies[bcount_steps[0]],
        body_buff[subtask_size];

    for (int step = 0; step < bcount_steps[1]; ++step) {
        MPI_Bcast(bodies, bcount_steps[0], mpi_body, MASTER_RANK, MPI_COMM_WORLD);

        accelerate(g_radius_dt[0], g_radius_dt[1], bcount_steps[0], bodies, offset, subtask_size);

        for (int i = 0; i < subtask_size; ++i)
            body_buff[i] = bodies[offset + i];
        MPI_Gather(body_buff, subtask_size, mpi_body, bodies, subtask_size, mpi_body, MASTER_RANK, MPI_COMM_WORLD);

        MPI_Scatter(bodies, subtask_size, mpi_body, body_buff, subtask_size, mpi_body, MASTER_RANK, MPI_COMM_WORLD);

        move(g_radius_dt[2], subtask_size, body_buff);

        MPI_Gather(body_buff, subtask_size, mpi_body, bodies, subtask_size, mpi_body, MASTER_RANK, MPI_COMM_WORLD);
    }
}

int main(int argc, char** argv)
{
    MPI_Init(NULL, NULL);

    // create MPI type for Vector3
    const int n_items = 3;
    int block_lengths[] = {1, 1, 1};
    MPI_Datatype types_vec3[] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE },
        mpi_vector3;
    MPI_Aint offsets_vec3[] = {
        offsetof(Vector3, x), offsetof(Vector3, y), offsetof(Vector3, z)
    };
    MPI_Type_create_struct(n_items, block_lengths, offsets_vec3, types_vec3, &mpi_vector3);
    MPI_Type_commit(&mpi_vector3);

    // create mpi type for Body
    MPI_Datatype types_body[] = { mpi_vector3, mpi_vector3, MPI_DOUBLE },
        mpi_body;
    MPI_Aint offsets_body[] = {
        offsetof(Body, position), offsetof(Body, velocity), offsetof(Body, mass)
    };
    MPI_Type_create_struct(n_items, block_lengths, offsets_body, types_body, &mpi_body);
    MPI_Type_commit(&mpi_body);

    int world_size, p_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &p_rank);

    if (p_rank == MASTER_RANK)
        master_process(mpi_body, world_size, argv[1], argv[2]);
    else
        slave_process(p_rank, world_size, mpi_body);

    // freeing types
    MPI_Type_free(&mpi_vector3);
    MPI_Type_free(&mpi_body);

    MPI_Finalize();

    return 0;
}