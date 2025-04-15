/*
 * timestamp.h
 *
 *  Created on: 2024. okt. 21.
 *      Author: nitro5
 */

#ifndef _TIMESTAMP_H_
#define _TIMESTAMP_H_


#include <inttypes.h>


typedef struct {
    uint64_t main_to_remote_S;
    uint64_t main_to_remote_R;
    uint64_t remote_to_main_S;
    uint64_t remote_to_main_R;
} TimeStamps;

//TimeStamps timestamp_array[1000u] __attribute((section(".bss.project_shared_mem")));


//#pragma DATA_SECTION(iterator, ".bss.user_shared_mem")
//uint32_t iterator __attribute((section(".bss.user_shared_mem")));

//uint32_t max_i __attribute((section(".bss.user_shared_mem")));



#endif /* TIME_MEASURMENT_TIMESTAMP_H_ */
