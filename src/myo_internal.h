/*
 * myo_internal.h
 *
 * Copyright (C) 2013-2014 Thalmic Labs Inc.
 * Developed under violation of the Myo SDK license agreement. See LICENSE.txt for details.
 * AKA, use at your own risk.
 *
 *  Created on: Oct 25, 2014
 *      Author: Vincent Simonetti
 */
#ifndef MYO_INTERNAL_H_
#define MYO_INTERNAL_H_

#include <stdlib.h>
#include <time.h>
#include "myo/libmyo.h"

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
	unsigned int device_count;
	const char** devices;
	//TODO
} libmyo_hub_impl_t;

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

#endif /* MYO_INTERNAL_H_ */
