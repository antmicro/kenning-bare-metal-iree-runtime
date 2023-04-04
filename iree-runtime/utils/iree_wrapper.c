#include "iree_wrapper.h"

/**
 * IREE runtime instance
 */
static iree_vm_instance_t *gp_instance = NULL;
/**
 * IREE device
 */
static iree_hal_device_t *gp_device = NULL;
/**
 * IREE execution context where modules are loaded
 */
static iree_vm_context_t *gp_context = NULL;

/**
 * Buffer for model inputs
 */
static iree_vm_list_t *gp_model_inputs = NULL;
/**
 * Buffer for model outputs
 */
static iree_vm_list_t *gp_model_outputs = NULL;

/**
 * Buffer for model weights
 */
static uint8_t *gp_model_weights;

/**
 * Struct describing model IO
 */
extern MlModel g_model_struct;

/**
 * Creates IREE device
 *
 * @param host_allocator allocator
 * @param out_device created device
 *
 * @returns error status
 */
static iree_status_t create_device(iree_allocator_t host_allocator, iree_hal_device_t **out_device)
{
    iree_status_t iree_status = iree_ok_status();
    iree_hal_executable_loader_t *loader = NULL;
    iree_hal_allocator_t *device_allocator = NULL;

    do
    {
        // prepare params
        iree_hal_sync_device_params_t params;
        iree_hal_sync_device_params_initialize(&params);

        // create loader
        iree_status =
            iree_hal_embedded_elf_loader_create(iree_hal_executable_import_provider_default(), host_allocator, &loader);
        BREAK_ON_IREE_ERROR(iree_status);

        // allocate buffers
        iree_string_view_t identifier = iree_make_cstring_view("sync");
        iree_status = iree_hal_allocator_create_heap(identifier, host_allocator, host_allocator, &device_allocator);
        BREAK_ON_IREE_ERROR(iree_status);

        // create device
        iree_status = iree_hal_sync_device_create(identifier, &params, /*loader_count=*/1, &loader, device_allocator,
                                                  host_allocator, out_device);
    } while (0);

    // cleanup
    if (NULL != loader)
    {
        iree_hal_executable_loader_release(loader);
    }
    if (NULL != device_allocator)
    {
        iree_hal_allocator_release(device_allocator);
    }

    return iree_status;
}

static void release_context()
{
    // release resources if already allocated
    if (NULL != gp_context)
    {
        iree_vm_context_release(gp_context);
        gp_context = NULL;
    }
    if (NULL != gp_instance)
    {
        iree_vm_instance_release(gp_instance);
        gp_instance = NULL;
    }
    if (NULL != gp_model_weights)
    {
        free(gp_model_weights);
        gp_model_weights = NULL;
    }
}

IREE_WRAPPER_STATUS create_context(const uint8_t *model_data, const size_t model_data_size)
{
    iree_status_t iree_status = iree_ok_status();
    iree_vm_module_t *hal_module = NULL;
    iree_vm_module_t *module = NULL;

    release_context();

    do
    {
        // prepare model weights
        gp_model_weights = aligned_alloc(4, model_data_size);
        memcpy(gp_model_weights, model_data, model_data_size);

        iree_allocator_t host_allocator = iree_allocator_system();
        iree_status = iree_vm_instance_create(host_allocator, &gp_instance);
        BREAK_ON_IREE_ERROR(iree_status);

        iree_status = iree_hal_module_register_all_types(gp_instance);
        BREAK_ON_IREE_ERROR(iree_status);

        // create device if not already created
        if (NULL == gp_device)
        {
            iree_status = create_device(host_allocator, &gp_device);
            BREAK_ON_IREE_ERROR(iree_status);
        }

        // create bytecode module
        iree_status =
            iree_vm_bytecode_module_create(gp_instance, iree_make_const_byte_span(gp_model_weights, model_data_size),
                                           iree_allocator_null(), host_allocator, &module);
        BREAK_ON_IREE_ERROR(iree_status);

        // create hal_module
        iree_status =
            iree_hal_module_create(gp_instance, gp_device, IREE_HAL_MODULE_FLAG_NONE, host_allocator, &hal_module);
        BREAK_ON_IREE_ERROR(iree_status);

        iree_vm_module_t *modules[] = {hal_module, module};

        // allocate context
        iree_status = iree_vm_context_create_with_modules(
            gp_instance, IREE_VM_CONTEXT_FLAG_NONE, IREE_ARRAYSIZE(modules), &modules[0], host_allocator, &gp_context);
    } while (0);

    // cleanup
    if (NULL != hal_module)
    {
        iree_vm_module_release(hal_module);
    }
    if (NULL != module)
    {
        iree_vm_module_release(module);
    }
    if (!iree_status_is_ok(iree_status))
    {
        release_context();
    }

    return (IREE_WRAPPER_STATUS)iree_status;
}

/**
 * Prepares model input HAL buffers
 *
 * @param model_input model input
 * @param arg_buffer_views output buffers views
 *
 * @returns error status
 */
static iree_status_t prepare_input_hal_buffer_views(const uint8_t *model_input,
                                                    iree_hal_buffer_view_t **arg_buffer_views)
{
    iree_status_t iree_status = iree_ok_status();

    iree_const_byte_span_t *byte_span[MAX_MODEL_INPUT_NUM] = {NULL};
    size_t offset = 0;

    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        size_t size = g_model_struct.input_size_bytes[i] * g_model_struct.input_length[i];
        byte_span[i] = malloc(sizeof(iree_const_byte_span_t));
        *byte_span[i] = iree_make_const_byte_span(model_input + offset, size);
        offset += size;
    }

    iree_hal_buffer_params_t buffer_params = {.type =
                                                  IREE_HAL_MEMORY_TYPE_HOST_LOCAL | IREE_HAL_MEMORY_TYPE_DEVICE_VISIBLE,
                                              .access = IREE_HAL_MEMORY_ACCESS_READ,
                                              .usage = IREE_HAL_BUFFER_USAGE_DEFAULT};
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        iree_status = iree_hal_buffer_view_allocate_buffer(
            iree_hal_device_allocator(gp_device), g_model_struct.num_input_dim[i], g_model_struct.input_shape[i],
            g_model_struct.hal_element_type, IREE_HAL_ENCODING_TYPE_DENSE_ROW_MAJOR, buffer_params, *byte_span[i],
            &(arg_buffer_views[i]));
        BREAK_ON_IREE_ERROR(iree_status);
    }

    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        if (NULL != byte_span[i])
        {
            free(byte_span[i]);
            byte_span[i] = NULL;
        }
    }

    return iree_status;
}

IREE_WRAPPER_STATUS prepare_input_buffer(const MlModel *model_struct, const uint8_t *model_input)
{
    iree_status_t iree_status = iree_ok_status();

    iree_status = iree_vm_list_create(
        /*element_type=*/NULL, /*capacity=*/model_struct->num_input, iree_allocator_system(), &gp_model_inputs);
    RETURN_ON_ERROR(iree_status, (IREE_WRAPPER_STATUS)iree_status);

    iree_hal_buffer_view_t *arg_buffer_views[MAX_MODEL_INPUT_NUM] = {NULL};
    iree_status = prepare_input_hal_buffer_views(model_input, arg_buffer_views);
    RETURN_ON_ERROR(iree_status, (IREE_WRAPPER_STATUS)iree_status);

    iree_vm_ref_t arg_buffer_view_ref;
    for (int i = 0; i < model_struct->num_input; ++i)
    {
        arg_buffer_view_ref = iree_hal_buffer_view_move_ref(arg_buffer_views[i]);
        iree_status = iree_vm_list_push_ref_move(gp_model_inputs, &arg_buffer_view_ref);
        RETURN_ON_ERROR(iree_status, (IREE_WRAPPER_STATUS)iree_status);
    }

    return (IREE_WRAPPER_STATUS)iree_status;
}

IREE_WRAPPER_STATUS prepare_output_buffer()
{
    iree_status_t iree_status = iree_ok_status();

    iree_status = iree_vm_list_create(
        /*element_type=*/NULL, /*capacity=*/1, iree_allocator_system(), &gp_model_outputs);

    return (IREE_WRAPPER_STATUS)iree_status;
}

IREE_WRAPPER_STATUS run_inference()
{
    iree_status_t iree_status = iree_ok_status();
    iree_vm_function_t main_function;

    iree_status =
        iree_vm_context_resolve_function(gp_context, iree_make_cstring_view(g_model_struct.entry_func), &main_function);
    RETURN_ON_ERROR(iree_status, (IREE_WRAPPER_STATUS)iree_status);

    // invoke model
    iree_status = iree_vm_invoke(gp_context, main_function,
                                 IREE_VM_INVOCATION_FLAG_NONE /*IREE_VM_INVOCATION_FLAG_TRACE_EXECUTION*/,
                                 /*policy=*/NULL, gp_model_inputs, gp_model_outputs, iree_allocator_system());

    return (IREE_WRAPPER_STATUS)iree_status;
}

IREE_WRAPPER_STATUS get_output(uint8_t *model_output)
{
    iree_status_t iree_status = iree_ok_status();

    size_t model_output_idx = 0;
    for (int output_idx = 0; output_idx < g_model_struct.num_output; ++output_idx)
    {
        iree_hal_buffer_mapping_t mapped_memory = {0};
        iree_hal_buffer_view_t *ret_buffer_view = NULL;
        // get the result buffers from the invocation.
        ret_buffer_view = (iree_hal_buffer_view_t *)iree_vm_list_get_ref_deref(gp_model_outputs, output_idx,
                                                                               iree_hal_buffer_view_get_descriptor());
        if (NULL == ret_buffer_view)
        {
            return IREE_WRAPPER_STATUS_ERROR;
        }
        iree_status =
            iree_hal_buffer_map_range(iree_hal_buffer_view_buffer(ret_buffer_view), IREE_HAL_MAPPING_MODE_SCOPED,
                                      IREE_HAL_MEMORY_ACCESS_READ, 0, IREE_WHOLE_BUFFER, &mapped_memory);
        CHECK_IREE_STATUS(iree_status);

        if ((output_idx > g_model_struct.num_output ||
             mapped_memory.contents.data_length / g_model_struct.output_size_bytes !=
                 g_model_struct.output_length[output_idx]) &&
            NULL == ret_buffer_view)
        {
            return IREE_WRAPPER_STATUS_ERROR;
        }
        memcpy(&model_output[model_output_idx], mapped_memory.contents.data,
               g_model_struct.output_size_bytes * g_model_struct.output_length[output_idx]);

        iree_hal_buffer_unmap_range(&mapped_memory);
    }

    return (IREE_WRAPPER_STATUS)iree_status;
}

IREE_WRAPPER_STATUS get_model_stats(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                    size_t *statistics_size)
{
    iree_status_t iree_status = iree_ok_status();

    if (statistics_buffer_size < sizeof(iree_hal_allocator_statistics_t))
    {
        return IREE_WRAPPER_STATUS_ERROR;
    }
    iree_hal_allocator_query_statistics(iree_hal_device_allocator(gp_device),
                                        (iree_hal_allocator_statistics_t *)statistics_buffer);
    *statistics_size = sizeof(iree_hal_allocator_statistics_t);

    return (IREE_WRAPPER_STATUS)iree_status;
}

void release_input_buffer()
{
    if (NULL != gp_model_inputs)
    {
        iree_vm_list_release(gp_model_inputs);
        gp_model_inputs = NULL;
    }
}

void release_output_buffer()
{
    if (NULL != gp_model_outputs)
    {
        iree_vm_list_release(gp_model_outputs);
        gp_model_outputs = NULL;
    }
}