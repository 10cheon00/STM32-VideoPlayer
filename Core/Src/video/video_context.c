#include "video/video_context.h"

void video_context_init(video_shared_context_t *shared_context) {
    shared_context->total_bytes_read = 0;
    shared_context->next_frame_start_byte = 0;
}
