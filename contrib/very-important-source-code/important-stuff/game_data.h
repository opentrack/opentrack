#ifndef GAME_DATA__H
#define GAME_DATA__H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool get_game_data(const char *input_fname, const char *output_fname, bool from_update);

#ifdef __cplusplus
}
#endif


#endif
