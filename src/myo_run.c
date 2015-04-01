/*
 * Copyright (C) 2013-2015 Thalmic Labs Inc.
 * Copyright (C) 2014-2015 Vincent Simonetti
 *
 * See LICENSE for details.
 *
 * Created on: Mar 31, 2015
 */

#include "myo_internal.h"
#include <string.h>
#include <math.h>

#include <btapi/btdevice.h>

#define SERVICE_0 "0xD5060001-A904-DEB9-4748-2C7F4A124842"
#define SERVICE_1 "0xD5060002-A904-DEB9-4748-2C7F4A124842"
#define SERVICE_2 "0xD5060003-A904-DEB9-4748-2C7F4A124842"
#define SERVICE_3 "0xD5060004-A904-DEB9-4748-2C7F4A124842"
#define SERVICE_4 "0xD5060005-A904-DEB9-4748-2C7F4A124842"
#define SERVICE_5 "0xD5060006-A904-DEB9-4748-2C7F4A124842"
#define SERVICE_6 "0x4248124A-7F2C-4847-B9DE-04A9010006D5"

#define SET_SERVICE_FLAG(flags,ind) ((flags) |= (1 << (ind)))
#define IS_SERVICE_FLAG_SET(flags,ind) (((flags) & (1 << (ind))) != 0)
#define HAS_ALL_SERVICE_FLAGS(flags) ((flags) == 0x7F)

static bool has_checked_devices = false;

libmyo_myo_impl_t* create_myo(const char* device, libmyo_hub_impl_t* hub)
{
	libmyo_myo_impl_t* myo = (libmyo_myo_impl_t*)malloc(sizeof(libmyo_myo_impl_t));
	if (myo)
	{
		myo->hub = hub;
		myo->device = strdup(device);
		myo->emg_setting = libmyo_stream_emg_disabled;
		//TODO
	}
	return myo;
}

void free_myo(libmyo_myo_impl_t* myo)
{
	myo->hub = NULL;
	free((void*)myo->device);
	myo->device = NULL;
	//TODO
	free(myo);
}

libmyo_event_impl_t* create_event(libmyo_event_type_t type, libmyo_myo_impl_t* myo)
{
	libmyo_event_impl_t* evt = (libmyo_event_impl_t*)malloc(sizeof(libmyo_event_impl_t));
	if (evt)
	{
		evt->type = type;
		evt->timestamp = (uint64_t)floor(get_time());
		evt->myo = myo;
		//TODO
	}
	return evt;
}

void free_event(libmyo_event_impl_t* evt)
{
	evt->myo = NULL;
	//TODO
	free(evt);
}

LIBMYO_EXPORT libmyo_result_t libmyo_run(libmyo_hub_t hub, unsigned int duration_ms, libmyo_handler_t handler, void* user_data, libmyo_error_details_t* out_error)
{
	if (!hub)
	{
		if (out_error)
		{
			*out_error = libmyo_create_error("hub is NULL", libmyo_error_invalid_argument);
		}
		return libmyo_error_invalid_argument;
	}
	if (!handler)
	{
		if (out_error)
		{
			*out_error = libmyo_create_error("handler is NULL", libmyo_error_invalid_argument);
		}
		return libmyo_error_invalid_argument;
	}
	libmyo_hub_impl_t* hub_impl = (libmyo_hub_impl_t*)hub;
	double start_time = get_time();

	if (__sync_bool_compare_and_swap(&has_checked_devices, false, true))
	{
		/* Check for devices */
		bt_remote_device_t* next_remote_device = NULL;
		bt_remote_device_t** remote_device_array = bt_disc_retrieve_devices(BT_DISCOVERY_PREKNOWN, 0);
		if (remote_device_array)
		{
			for (int32_t i = 0; (next_remote_device = remote_device_array[i]); ++i)
			{
				if ((get_time() - start_time) > duration_ms)
				{
					break;
				}
				/* Simple tests to check that it's a Myo */
				if (bt_rdev_get_device_class(next_remote_device, BT_COD_DEVICECLASS) != 0 ||
					bt_rdev_get_device_class(next_remote_device, BT_COD_MAJORSERVICECLASS) != 0 ||
					bt_rdev_get_device_class(next_remote_device, BT_COD_MAJORDEVICECLASS) != 0 ||
					bt_rdev_get_device_class(next_remote_device, BT_COD_MINORDEVICECLASS) != 0 ||
					bt_rdev_get_type(next_remote_device) != BT_DEVICE_TYPE_LE_PUBLIC ||
					bt_rdev_is_encrypted(next_remote_device))
				{
					continue;
				}

				/* Iterative check for Myo */
				//XXX
				char **services_array = bt_rdev_get_services(next_remote_device);
				if (services_array != NULL)
				{
					bt_rdev_free_services(services_array);
					continue;
				}
				services_array = bt_rdev_get_services_gatt(next_remote_device);
				if (services_array != NULL)
				{
					const char* service = NULL;
					int32_t service_flags = 0;
					for (int32_t k = 0; !HAS_ALL_SERVICE_FLAGS(service_flags) && (service = services_array[k]); ++k)
					{
						/* This is just ripe for copy-paste errors... */
						if (!IS_SERVICE_FLAG_SET(service_flags, 0) && strcmp(service, SERVICE_0) == 0)
						{
							SET_SERVICE_FLAG(service_flags, 0);
						}
						else if (!IS_SERVICE_FLAG_SET(service_flags, 1) && strcmp(service, SERVICE_1) == 0)
						{
							SET_SERVICE_FLAG(service_flags, 1);
						}
						else if (!IS_SERVICE_FLAG_SET(service_flags, 2) && strcmp(service, SERVICE_2) == 0)
						{
							SET_SERVICE_FLAG(service_flags, 2);
						}
						else if (!IS_SERVICE_FLAG_SET(service_flags, 3) && strcmp(service, SERVICE_3) == 0)
						{
							SET_SERVICE_FLAG(service_flags, 3);
						}
						else if (!IS_SERVICE_FLAG_SET(service_flags, 4) && strcmp(service, SERVICE_4) == 0)
						{
							SET_SERVICE_FLAG(service_flags, 4);
						}
						else if (!IS_SERVICE_FLAG_SET(service_flags, 5) && strcmp(service, SERVICE_5) == 0)
						{
							SET_SERVICE_FLAG(service_flags, 5);
						}
						else if (!IS_SERVICE_FLAG_SET(service_flags, 6) && strcmp(service, SERVICE_6) == 0)
						{
							SET_SERVICE_FLAG(service_flags, 6);
						}
					}
					bt_rdev_free_services(services_array);

					if (HAS_ALL_SERVICE_FLAGS(service_flags))
					{
						char device_addr[128];
						if (bt_rdev_get_address(next_remote_device, device_addr) != 0)
						{
							/* Just ignore it */
							continue;
						}
						libmyo_myo_impl_t* myo_impl = create_myo(device_addr, hub_impl);
						if (myo_impl)
						{
							//TODO: create a new "paired" event, need to get firmware version
							//XXX: what do we do if they want to stop events? Can't reset has_checked_devices, so maybe create but mark as no-event-sent
							free_myo(myo_impl); //XXX
						}
					}
				}
			}
			bt_rdev_free_array(remote_device_array);
			if ((get_time() - start_time) > duration_ms)
			{
				/* Ran out of time */
				return libmyo_success;
			}
		}
		//XXX sleep for a period of time to let bluetooth events kick in?
	}

	//TODO: commands

	return libmyo_success;
}
