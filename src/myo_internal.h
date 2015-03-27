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
	unsigned int myo_count;
	struct libmyo_myo_impl_s** myos;
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
	unsigned int data_len;
	char* data;
} libmyo_cmd_t;

typedef struct libmyo_cmd_chain_s
{
	libmyo_cmd_t cmd;
	struct libmyo_cmd_chain_s* next;
} libmyo_cmd_chain_t;

libmyo_error_details_t libmyo_create_error(const char* error, libmyo_result_t error_kind);
bool validate_hub(libmyo_hub_impl_t* hub);
double get_time();

#endif /* MYO_INTERNAL_H_ */
