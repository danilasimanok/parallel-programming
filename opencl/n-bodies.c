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

const char *err_code (cl_int err_in)
{
    switch (err_in) {
        case CL_SUCCESS:
            return (char*)"CL_SUCCESS";
        case CL_DEVICE_NOT_FOUND:
            return (char*)"CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE:
            return (char*)"CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE:
            return (char*)"CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            return (char*)"CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES:
            return (char*)"CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY:
            return (char*)"CL_OUT_OF_HOST_MEMORY";
        case CL_PROFILING_INFO_NOT_AVAILABLE:
            return (char*)"CL_PROFILING_INFO_NOT_AVAILABLE";
        case CL_MEM_COPY_OVERLAP:
            return (char*)"CL_MEM_COPY_OVERLAP";
        case CL_IMAGE_FORMAT_MISMATCH:
            return (char*)"CL_IMAGE_FORMAT_MISMATCH";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            return (char*)"CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case CL_BUILD_PROGRAM_FAILURE:
            return (char*)"CL_BUILD_PROGRAM_FAILURE";
        case CL_MAP_FAILURE:
            return (char*)"CL_MAP_FAILURE";
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            return (char*)"CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            return (char*)"CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case CL_INVALID_VALUE:
            return (char*)"CL_INVALID_VALUE";
        case CL_INVALID_DEVICE_TYPE:
            return (char*)"CL_INVALID_DEVICE_TYPE";
        case CL_INVALID_PLATFORM:
            return (char*)"CL_INVALID_PLATFORM";
        case CL_INVALID_DEVICE:
            return (char*)"CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT:
            return (char*)"CL_INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES:
            return (char*)"CL_INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE:
            return (char*)"CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR:
            return (char*)"CL_INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT:
            return (char*)"CL_INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            return (char*)"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE:
            return (char*)"CL_INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER:
            return (char*)"CL_INVALID_SAMPLER";
        case CL_INVALID_BINARY:
            return (char*)"CL_INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS:
            return (char*)"CL_INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM:
            return (char*)"CL_INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE:
            return (char*)"CL_INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME:
            return (char*)"CL_INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION:
            return (char*)"CL_INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL:
            return (char*)"CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX:
            return (char*)"CL_INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE:
            return (char*)"CL_INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE:
            return (char*)"CL_INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS:
            return (char*)"CL_INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION:
            return (char*)"CL_INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE:
            return (char*)"CL_INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE:
            return (char*)"CL_INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET:
            return (char*)"CL_INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST:
            return (char*)"CL_INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT:
            return (char*)"CL_INVALID_EVENT";
        case CL_INVALID_OPERATION:
            return (char*)"CL_INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT:
            return (char*)"CL_INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE:
            return (char*)"CL_INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL:
            return (char*)"CL_INVALID_MIP_LEVEL";
        case CL_INVALID_GLOBAL_WORK_SIZE:
            return (char*)"CL_INVALID_GLOBAL_WORK_SIZE";
        case CL_INVALID_PROPERTY:
            return (char*)"CL_INVALID_PROPERTY";

        default:
            return (char*)"UNKNOWN ERROR";
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        return 1488;
    }

    Vector3 v1 = { 0.0f, 4.0f, 0.0f },
        v2 = { 3.0f, 0.0f, 0.0f };
    float distance;

    cl_device_id device_id;
    cl_int status = get_device_id(&device_id);
    cl_context context = clCreateContext(0, 1, &device_id, NULL, NULL, &status);
    cl_command_queue commands = clCreateCommandQueueWithProperties(context, device_id, NULL, &status);

    cl_program program;
    status = build_from_source(argv[1], context, device_id, &program);
    cl_kernel vector3_add = clCreateKernel(program, "dist", &status);
    if (status != CL_SUCCESS) {
        fprintf(stderr, "Boom! Status: %s\n", err_code(status));
        return 1488;
    }

    cl_mem v1_mem = clCreateBuffer(context,  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(Vector3), &v1, &status),
        v2_mem = clCreateBuffer(context,  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  sizeof(Vector3), &v2, &status),
        distance_mem = clCreateBuffer(context,  CL_MEM_READ_WRITE, sizeof(float), NULL, &status);
    
    status = clSetKernelArg(vector3_add, 0u, sizeof(cl_mem), &v1_mem);
    status |= clSetKernelArg(vector3_add, 1u, sizeof(cl_mem), &v2_mem);
    status |= clSetKernelArg(vector3_add, 2u, sizeof(cl_mem), &distance_mem);

    size_t global_work_size = 1;
    status = clEnqueueNDRangeKernel(commands, vector3_add, 1, NULL, &global_work_size, NULL, 0u, NULL, NULL);

    status = clEnqueueReadBuffer(commands, distance_mem, CL_TRUE, 0, sizeof(float), &distance, 0, NULL, NULL);
    printf("distance = %f\n", distance);

    clReleaseMemObject(v1_mem);
    clReleaseMemObject(v2_mem);
    clReleaseMemObject(distance_mem);
    clReleaseProgram(program);
    clReleaseKernel(vector3_add);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    return 0;
}