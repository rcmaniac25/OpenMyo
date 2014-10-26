/*
 * myo.c
 *
 * Copyright (C) 2013-2014 Thalmic Labs Inc.
 * Developed under violation of the Myo SDK license agreement. See LICENSE.txt for details.
 * AKA, use at your own risk.
 *
 *  Created on: Oct 20, 2014
 *      Author: Vincent Simonetti
 */

#include "myo_internal.h"
#include <string.h>
#include <errno.h>

#include <btapi/btdevice.h>

static int bt_manager_init = 0; //XXX Changes too this should be done atomically

typedef struct libmyo_hub_impl_s {
	//TODO
} libmyo_hub_impl_t;

void myo_bt_callback(const int event, const char *bt_addr, const char *event_data)
{
	//TODO
}

LIBMYO_EXPORT libmyo_result_t libmyo_init_hub(libmyo_hub_t* out_hub, const char* application_identifier, libmyo_error_details_t* out_error)
{
	/* Check arguments */
	if (!out_hub)
	{
		if (out_error)
		{
			*out_error = libmyo_create_error("out_hub is NULL", libmyo_error_invalid_argument);
		}
		return libmyo_error_invalid_argument;
	}
	if (application_identifier && application_identifier[0] != '\0')
	{
		if (strlen(application_identifier) > 255)
		{
			if (out_error)
			{
				*out_error = libmyo_create_error("application_identifier cannot be longer then 255 chars", libmyo_error_invalid_argument);
			}
			return libmyo_error_invalid_argument;
		}
		//TODO: check that it is in correct format
		/// \a application_identifier must follow a reverse domain name format (ex. com.domainname.appname). Application
		/// identifiers can be formed from the set of alphanumeric ASCII characters (a-z, A-Z, 0-9). The hyphen (-) and
		/// underscore (_) characters are permitted if they are not adjacent to a period (.) character (i.e. not at the start or
		/// end of each segment), but are not permitted in the top-level domain. Application identifiers must have three or more
		/// segments. For example, if a company's domain is example.com and the application is named hello-world, one could use
		/// "com.example.hello-world" as a valid application identifier.
		/// @returns libmyo_error_invalid_argument if \a application_identifier is not in the proper reverse domain name format
	}

	/* Setup manager */
	//TODO

	/* Startup device manager */
	if (bt_manager_init == 0 && bt_device_init(myo_bt_callback) != 0)
	{
		if (out_error)
		{
			const char* reason = errno == ENOMEM ? "Out of memory" :
					(errno == EINVAL ? "Bluetooth stack already initialized" : "Could not initialize bluetooth stack");
			*out_error = libmyo_create_error(reason, libmyo_error_invalid_argument);
		}
		return libmyo_error_runtime;
	}
	bt_manager_init++;
	if (!bt_ldev_get_power())
	{
	    if (bt_ldev_set_power(true) != 0)
	    {
	    	//TODO
	    }
	}

	return libmyo_success;
}

LIBMYO_EXPORT libmyo_result_t libmyo_shutdown_hub(libmyo_hub_t hub, libmyo_error_details_t* out_error)
{
	/* Check arguments */
	if (!hub)
	{
		if(out_error)
		{
			*out_error = libmyo_create_error("hub is NULL", libmyo_error_invalid_argument);
		}
		return libmyo_error_invalid_argument;
	}
	libmyo_hub_impl_t* hub_impl = (libmyo_hub_impl_t*)hub;
	if (bt_manager_init == 0) //XXX Any additional check to make sure hub is valid
	{
		if (out_error)
		{
			*out_error = libmyo_create_error("Not a valid hub", libmyo_error);
		}
		return libmyo_error;
	}

	/* Shutdown device manager */
	if (bt_manager_init == 1)
	{
		bt_device_deinit();
	}
	bt_manager_init--;

	/* Cleanup hub */
	//XXX Any additional cleanup the hub needs
	free(hub_impl);

	return libmyo_success;
}

LIBMYO_EXPORT libmyo_result_t libmyo_vibrate(libmyo_myo_t myo, libmyo_vibration_type_t type, libmyo_error_details_t* out_error)
{
	//TODO
	return libmyo_success;
}

LIBMYO_EXPORT libmyo_result_t libmyo_request_rssi(libmyo_myo_t myo, libmyo_error_details_t* out_error)
{
	//TODO
	return libmyo_success;
}

LIBMYO_EXPORT libmyo_result_t libmyo_run(libmyo_hub_t hub, unsigned int duration_ms, libmyo_handler_t handler, void* user_data, libmyo_error_details_t* out_error)
{
	//TODO
	return libmyo_success;
}
