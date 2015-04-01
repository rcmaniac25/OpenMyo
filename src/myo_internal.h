/*
 * Copyright (C) 2013-2015 Thalmic Labs Inc.
 * Copyright (C) 2014-2015 Vincent Simonetti
 *
 * See LICENSE for details.
 *
 * Created on: Oct 25, 2014
 */
#ifndef MYO_INTERNAL_H_
#define MYO_INTERNAL_H_

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "myo/libmyo.h"
#include "myohw.h"

/** Utilities */
typedef struct myo_list_s
{
	uint32_t count;
	uint32_t allocated;
	void** data;
} libmyo_list_t;

#define MYO_INIT_EMPTY_LIST {0, 0, NULL}
#define myo_list_remove_item(list, index) myo_list_remove_item_ctrl_free((list), (index), true)

/** Note: these functions don't manipulate data in any way, only it's internal state */
bool myo_list_append_item(libmyo_list_t* list, void* item);
void* myo_list_remove_item_ctrl_free(libmyo_list_t* list, uint32_t index, bool free_contents_on_empty);
void myo_list_free_contents(libmyo_list_t* list);

/** Library Structs */
typedef enum
{
	libmyo_cmd_device_paired,
	libmyo_cmd_device_connected,
	libmyo_cmd_device_disconnected,
	libmyo_cmd_device_unpaired
} libmyo_cmd_type;

typedef struct libmyo_hub_impl_s
{
	const char* app_id;
	libmyo_list_t myos;
	libmyo_locking_policy_t locking_policy;
	//TODO
} libmyo_hub_impl_t;

typedef struct libmyo_myo_impl_s
{
	libmyo_hub_impl_t* hub;
	const char* device;
	libmyo_stream_emg_t emg_setting;
	//TODO
} libmyo_myo_impl_t;

typedef struct libmyo_event_impl_s
{
	libmyo_event_type_t type;
	uint64_t timestamp;
	libmyo_myo_impl_t* myo;
	//TODO
} libmyo_event_impl_t;

typedef struct libmyo_cmd_s
{
	libmyo_cmd_type type;
	time_t time;
	uint32_t data_len;
	char* data;
} libmyo_cmd_t;

typedef struct libmyo_cmd_chain_s
{
	libmyo_cmd_t cmd;
	struct libmyo_cmd_chain_s* next;
} libmyo_cmd_chain_t;

/** Library Functions */
libmyo_error_details_t libmyo_create_error(const char* error, libmyo_result_t error_kind);
bool validate_hub(libmyo_hub_impl_t* hub);
double get_time();

//TODO

#endif /* MYO_INTERNAL_H_ */
