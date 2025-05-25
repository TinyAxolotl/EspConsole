#pragma once

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void (*handle_input_event)(uint32_t key);
void process_game_logic(void);

#ifdef __cplusplus
}
#endif
