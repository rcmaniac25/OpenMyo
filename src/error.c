/*
 * error.c
 *
 * Copyright (C) 2013-2014 Thalmic Labs Inc.
 * Developed under violation of the Myo SDK license agreement. See LICENSE.txt for details.
 * AKA, use at your own risk.
 *
 *  Created on: Oct 20, 2014
 *      Author: Vincent Simonetti
 */

#include <stdlib.h>
#include "myo/libmyo.h"

typedef struct libmyo_error_details_impl_s
{
	//TODO
} libmyo_error_details_impl_t;

LIBMYO_EXPORT const char* libmyo_error_cstring(libmyo_error_details_t error)
{
	libmyo_error_details_impl_t* error_details = (libmyo_error_details_impl_t*)error;
	//TODO
	return NULL;
}

LIBMYO_EXPORT libmyo_result_t libmyo_error_kind(libmyo_error_details_t error)
{
	libmyo_error_details_impl_t* error_details = (libmyo_error_details_impl_t*)error;
	//TODO
	return libmyo_success;
}

LIBMYO_EXPORT void libmyo_free_error_details(libmyo_error_details_t error)
{
	libmyo_error_details_impl_t* error_details = (libmyo_error_details_impl_t*)error;
	//TODO
}
