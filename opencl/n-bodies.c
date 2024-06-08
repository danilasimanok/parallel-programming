#include <CL/cl_platform.h>
#include <stdio.h>
#include <CL/cl.h>

typedef struct __attribute__ ((packed)) Vector3 {
    float x;
    float y;
    float z;
} Vector3;

cl_int get_device_id(cl_device_id *result)
{
    cl_int status;
    cl_uint num_platforms;
    status = clGetPlatformIDs(0, NULL, &num_platforms);

    if (status != CL_SUCCESS)
        return status;
    if (num_platforms == 0)
        return -1488;

    cl_platform_id Platform[num_platforms];
    status = clGetPlatformIDs(num_platforms, Platform, NULL);

    if (status != CL_SUCCESS)
        return status;

    status+= 1;
    uint i = 0;
    while ((status != CL_SUCCESS) || i < num_platforms) {
        status = clGetDeviceIDs(Platform[i], CL_DEVICE_TYPE_DEFAULT, 1, result, NULL);
        i += 1;
    }

    return status;
}

char *getKernelSource(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error: Could not open kernel source file\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    int len = ftell(file) + 1;
    rewind(file);

    char *source = (char *)calloc(sizeof(char), len);
    if (!source)
    {
        fprintf(stderr, "Error: Could not allocate memory for source string\n");
        exit(EXIT_FAILURE);
    }
    fread(source, sizeof(char), len, file);
    fclose(file);
    return source;
}

cl_int build_from_source(char *filename, cl_context context, cl_device_id device_id, cl_program *result)
{
    cl_int status;
    char *kernel_source = getKernelSource(filename);
    *result = clCreateProgramWithSource(context, 1, (const char **) & kernel_source, NULL, &status);
    free(kernel_source);

    if (status != CL_SUCCESS)
        return status;

    status = clBuildProgram(*result, 0, NULL, NULL, NULL, NULL);

    if (status != CL_SUCCESS) {
        size_t len;
        char buffer[2048];

        fprintf(stderr, "Error: Failed to build program executable!\n%i\n", status);
        clGetProgramBuildInfo(*result, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        return EXIT_FAILURE;
    }

    return status;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        return 1488;
    }

    Vector3 v1 = { 1.0f, 2.0f, 3.0f },
        v2 = { 1.0f, 2.0f, 3.0f },sum;

    cl_device_id device_id;
    cl_int status = get_device_id(&device_id);
    cl_context context = clCreateContext(0, 1, &device_id, NULL, NULL, &status);
    cl_command_queue commands = clCreateCommandQueueWithProperties(context, device_id, NULL, &status);

    cl_program program;
    status = build_from_source(argv[1], context, device_id, &program);
    cl_kernel vector3_add = clCreateKernel(program, "add", &status);

    cl_mem v1_mem = clCreateBuffer(context,  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(Vector3), &v1, &status),
        v2_mem = clCreateBuffer(context,  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(Vector3), &v2, &status),
        sum_mem = clCreateBuffer(context,  CL_MEM_READ_WRITE, sizeof(Vector3), NULL, &status);
    
    status = clSetKernelArg(vector3_add, 0u, sizeof(cl_mem), &v1_mem);
    status |= clSetKernelArg(vector3_add, 1u, sizeof(cl_mem), &v2_mem);
    status |= clSetKernelArg(vector3_add, 2u, sizeof(cl_mem), &sum_mem);

    size_t global_work_size = 1;
    status = clEnqueueNDRangeKernel(commands, vector3_add, 1, NULL, &global_work_size, NULL, 0u, NULL, NULL);

    status = clEnqueueReadBuffer(commands, sum_mem, CL_TRUE, 0, sizeof(Vector3), &sum, 0, NULL, NULL);
    printf("sum = (%f, %f, %f)\n", sum.x, sum.y, sum.z);

    clReleaseMemObject(v1_mem);
    clReleaseMemObject(v2_mem);
    clReleaseMemObject(sum_mem);
    clReleaseProgram(program);
    clReleaseKernel(vector3_add);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    return 0;
}