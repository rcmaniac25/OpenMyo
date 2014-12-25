/*
 * Copyright (C) 2013-2014 Thalmic Labs Inc.
 * Copyright (C) 2014 Vincent Simonetti
 *
 * See LICENSE for details.
 *
 * Built without violation of the Myo SDK license agreement. See the Myo SDK's LICENSE.txt for details.
 *
 * Created on: Oct 20, 2014
 */

#include "myo_internal.h"
#include <string.h>

typedef struct libmyo_error_details_impl_s
{
	const char* error_msg;
	libmyo_result_t error_kind;
} libmyo_error_details_impl_t;

libmyo_error_details_t libmyo_create_error(const char* error, libmyo_result_t error_kind)
{
	libmyo_error_details_impl_t* err = malloc(sizeof(libmyo_error_details_impl_t));
	if (err)
	{
		err->error_msg = strdup(error);
		err->error_kind = error_kind;
	}
	return (libmyo_error_details_t)err;
}

LIBMYO_EXPORT const char* libmyo_error_cstring(libmyo_error_details_t error)
{
	return ((libmyo_error_details_impl_t*)error)->error_msg;
}

LIBMYO_EXPORT libmyo_result_t libmyo_error_kind(libmyo_error_details_t error)
{
	return ((libmyo_error_details_impl_t*)error)->error_kind;
}

LIBMYO_EXPORT void libmyo_free_error_details(libmyo_error_details_t error)
{
	libmyo_error_details_impl_t* error_details = (libmyo_error_details_impl_t*)error;
	if (error_details)
	{
		free((void*)error_details->error_msg);
		free(error_details);
	}
}
