#include "iree_wrapper.h"

IREE_WRAPPER_STATUS create_context(const uint8_t *model_data, const size_t model_data_size) {}
IREE_WRAPPER_STATUS prepare_input_buffer(const MlModel *model_struct, const uint8_t *model_input) {}
IREE_WRAPPER_STATUS prepare_output_buffer() {}
IREE_WRAPPER_STATUS run_inference() {}
IREE_WRAPPER_STATUS get_output(uint8_t *model_output) {}
IREE_WRAPPER_STATUS get_model_stats(const size_t statistics_buffer_size, uint8_t *statistics_buffer,
                                    size_t *statistics_size)
{
}
void release_input_buffer() {}
void release_output_buffer() {}
