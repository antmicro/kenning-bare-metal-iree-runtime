#include "model.h"

const char *const MODEL_STATUS_STR[] = {MODEL_STATUSES(GENERATE_STR)};

static MlModel g_model_struct;
static MODEL_STATE g_model_state = MODEL_STATE_UNINITIALIZED;
static iree_vm_instance_t *gp_instance = NULL;
static iree_hal_device_t *gp_device = NULL;
static iree_vm_context_t *gp_context = NULL;

static iree_vm_list_t *gp_model_inputs = NULL;
static iree_vm_list_t *gp_model_outputs = NULL;

static uint8_t *gp_model_weights = NULL;

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

        // create vm instance
        iree_vm_instance_t *instance = NULL;
        iree_status = iree_vm_instance_create(host_allocator, &instance);
        BREAK_ON_IREE_ERROR(iree_status);

        // create loader
        iree_status = iree_hal_vmvx_module_loader_create(instance, /*user_module_count=*/0, /*user_modules=*/NULL,
                                                         host_allocator, &loader);
        iree_vm_instance_release(instance);
        BREAK_ON_IREE_ERROR(iree_status);

        // allocate buffers
        iree_string_view_t identifier = iree_make_cstring_view("vmvx");
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

/**
 * Clears context that hold modules' state
 */
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

/**
 * Creates context that hold modules' state
 *
 * @param model_data compiled model data
 * @param model_data_size size of compiled model data
 *
 * @returns error status
 */
static iree_status_t create_context(const uint8_t *model_data, const size_t model_data_size)
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

    return iree_status;
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

MODEL_STATUS load_model_struct(const uint8_t *model_struct_data, const size_t data_size)
{
    MODEL_STATUS status = MODEL_STATUS_OK;

    if (sizeof(MlModel) != data_size)
    {
        LOG_ERROR("Wrong model struct size: %d. Should be: %d.", data_size, sizeof(MlModel));
        return MODEL_STATUS_INVALID_ARGUMENT;
    }

    g_model_struct = *((MlModel *)model_struct_data);

    char *dtype = (char *)&g_model_struct.hal_element_type;

    // this x-macro retrieves string label and HAL element enum value from IREE_HAL_ELEMENT_TYPES table and
    // compares this label with the string received with the struct. If label is equal to this string then the
    // relevand enum value is assigned. If none of the labels is equal to this string then the final else is
    // hit and error is returned
#define CHECK_HAL_ELEM_TYPE(label, element_type)        \
    if (0 == strncmp(dtype, label, 4))                  \
    {                                                   \
        g_model_struct.hal_element_type = element_type; \
    }                                                   \
    else
    IREE_HAL_ELEMENT_TYPES(CHECK_HAL_ELEM_TYPE)
    {
        LOG_ERROR("Wrong dtype %s", dtype);
        return MODEL_STATUS_INVALID_ARGUMENT;
    }
#undef CHECK_HAL_ELEM_TYPE

    LOG_INFO("Loaded model struct. Model name: %s", g_model_struct.model_name);

    g_model_state = MODEL_STATE_STRUCT_LOADED;

    return status;
}

MODEL_STATUS load_model_weights(const uint8_t *model_weights_data, const size_t data_size)
{
    MODEL_STATUS status = MODEL_STATUS_OK;
    iree_status_t iree_status = iree_ok_status();

    if (g_model_state < MODEL_STATE_STRUCT_LOADED)
    {
        return MODEL_STATUS_INVALID_STATE;
    }

    // free input/output resources
    if (NULL != gp_model_outputs)
    {
        iree_vm_list_release(gp_model_outputs);
        gp_model_outputs = NULL;
    }
    if (NULL != gp_model_inputs)
    {
        iree_vm_list_release(gp_model_inputs);
        gp_model_inputs = NULL;
    }

    iree_status = create_context(model_weights_data, data_size);
    CHECK_IREE_STATUS(iree_status);

    LOG_INFO("Loaded model weights");

    g_model_state = MODEL_STATE_WEIGHTS_LOADED;

    return status;
}

MODEL_STATUS load_model_input(const uint8_t *model_input, const size_t model_input_size)
{
    MODEL_STATUS status = MODEL_STATUS_OK;
    iree_status_t iree_status = iree_ok_status();

    if (g_model_state < MODEL_STATE_WEIGHTS_LOADED)
    {
        return MODEL_STATUS_INVALID_STATE;
    }

    // validate size of received data
    size_t expected_size = 0;
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        expected_size += g_model_struct.input_length[i] * g_model_struct.input_size_bytes[i];
    }
    if (model_input_size != expected_size)
    {
        LOG_ERROR("Invalid model input size: %d. Expected size: %d", model_input_size, expected_size);
        return MODEL_STATUS_INVALID_ARGUMENT;
    }

    // free resources
    if (NULL != gp_model_inputs)
    {
        iree_vm_list_release(gp_model_inputs);
        gp_model_inputs = NULL;
    }

    // setup buffers for inputs
    iree_status = iree_vm_list_create(
        /*element_type=*/NULL, /*capacity=*/g_model_struct.num_input, iree_allocator_system(), &gp_model_inputs);
    CHECK_IREE_STATUS(iree_status);

    iree_hal_buffer_view_t *arg_buffer_views[MAX_MODEL_INPUT_NUM] = {NULL};
    iree_status = prepare_input_hal_buffer_views(model_input, arg_buffer_views);
    CHECK_IREE_STATUS(iree_status);

    iree_vm_ref_t arg_buffer_view_ref;
    for (int i = 0; i < g_model_struct.num_input; ++i)
    {
        arg_buffer_view_ref = iree_hal_buffer_view_move_ref(arg_buffer_views[i]);
        iree_status = iree_vm_list_push_ref_move(gp_model_inputs, &arg_buffer_view_ref);
        CHECK_IREE_STATUS(iree_status);
    }

    LOG_INFO("Loaded model input");

    g_model_state = MODEL_STATE_INPUT_LOADED;

    return status;
}

MODEL_STATUS run_model()
{
    MODEL_STATUS status = MODEL_STATUS_OK;
    iree_status_t iree_status = iree_ok_status();

    if (g_model_state < MODEL_STATE_INPUT_LOADED)
    {
        return MODEL_STATUS_INVALID_STATE;
    }

    iree_vm_function_t main_function;

    // free resources
    if (NULL != gp_model_outputs)
    {
        iree_vm_list_release(gp_model_outputs);
        gp_model_outputs = NULL;
    }

    // setup buffers for outputs
    iree_status = iree_vm_list_create(
        /*element_type=*/NULL, /*capacity=*/1, iree_allocator_system(), &gp_model_outputs);
    CHECK_IREE_STATUS(iree_status);

    // look for entry function
    iree_status =
        iree_vm_context_resolve_function(gp_context, iree_make_cstring_view(g_model_struct.entry_func), &main_function);
    CHECK_IREE_STATUS(iree_status);

    // invoke model
    iree_status = iree_vm_invoke(gp_context, main_function,
                                 IREE_VM_INVOCATION_FLAG_NONE /*IREE_VM_INVOCATION_FLAG_TRACE_EXECUTION*/,
                                 /*policy=*/NULL, gp_model_inputs, gp_model_outputs, iree_allocator_system());
    CHECK_IREE_STATUS(iree_status);

    LOG_INFO("Model inference done");

    g_model_state = MODEL_STATE_INFERENCE_DONE;

    return status;
}

MODEL_STATUS get_model_output(const size_t buffer_size, uint8_t *model_output, size_t *model_output_size)
{
    MODEL_STATUS status = MODEL_STATUS_OK;
    iree_status_t iree_status = iree_ok_status();

    if (g_model_state < MODEL_STATE_INFERENCE_DONE)
    {
        return MODEL_STATUS_INVALID_STATE;
    }

    size_t output_size = 0;
    for (int i = 0; i < g_model_struct.num_output; ++i)
    {
        output_size += g_model_struct.output_length[i] * g_model_struct.output_size_bytes;
    }
    if (buffer_size < output_size)
    {
        LOG_ERROR("Buffer is too small. Buffer size: %d. Model output size: %d", buffer_size, output_size);
        return MODEL_STATUS_INVALID_ARGUMENT;
    }
    *model_output_size = output_size;

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
            return MODEL_STATUS_IREE_ERROR;
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
            return MODEL_STATUS_IREE_ERROR;
        }
        memcpy(&model_output[model_output_idx], mapped_memory.contents.data,
               g_model_struct.output_size_bytes * g_model_struct.output_length[output_idx]);

        iree_hal_buffer_unmap_range(&mapped_memory);
    }

    LOG_INFO("Model output retrieved");

    return status;
}

MODEL_STATUS get_statistics(iree_hal_allocator_statistics_t *statistics)
{
    MODEL_STATUS status = MODEL_STATUS_OK;

    if (g_model_state < MODEL_STATE_WEIGHTS_LOADED)
    {
        return MODEL_STATUS_INVALID_STATE;
    }

    iree_hal_allocator_query_statistics(iree_hal_device_allocator(gp_device), statistics);

    LOG_INFO("Model statistics retrieved");

    return status;
}
