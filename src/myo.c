/*
 * Copyright (C) 2013-2015 Thalmic Labs Inc.
 * Copyright (C) 2014-2015 Vincent Simonetti
 *
 * See LICENSE for details.
 *
 * Created on: Oct 20, 2014
 */

#include "myo_internal.h"
#include <string.h>
#include <errno.h>

#include <pthread.h>
#include <time.h>
#include <math.h>

#include <btapi/btdevice.h>
#include <btapi/btgatt.h>
#include <btapi/btle.h>

#include <bbndk.h>

void free_myo(libmyo_myo_impl_t* myo);
const char* myo_create_uuid(myohw_services service);
const char* myo_create_standard_uuid(myohw_standard_services service);

//XXX init and shutdown need some cleanup. They were written to be small, but between locks and state changes, it got pretty ugly quickly

/** System variables */
pthread_mutex_t manager_lock = PTHREAD_MUTEX_INITIALIZER; /* PTHREAD_RMUTEX_INITIALIZER for recursive mutexes */
static int32_t bt_manager_init = 0; /* Could possibly replace with hubs.count... */
static libmyo_list_t hubs = MYO_INIT_EMPTY_LIST;
static libmyo_list_t uuids = MYO_INIT_EMPTY_LIST;

pthread_mutex_t cmd_lock = PTHREAD_MUTEX_INITIALIZER;
static libmyo_cmd_chain_t* commands = NULL;
static libmyo_cmd_chain_t* last_command = NULL;

bool validate_app_id(const char* app_id, libmyo_error_details_t* out_error)
{
	const char* error = NULL;
	int32_t seperator_count = 0;

	uint32_t len = strlen(app_id);
	for (uint32_t i = 0; i < len && !error; i++)
	{
		if (app_id[i] == '.')
		{
			/* Check to make sure that a . separates sections and it isn't just ".." */
			if (i > 0 && app_id[i - 1] != '.' &&
					(i + 1) < len && app_id[i + 1] != '.')
			{
				seperator_count++;
			}
		}
		else
		{
			char c = app_id[i];
			if (c == '-' || c == '_')
			{
				if (seperator_count < 1)
				{
					error = "Hyphen (-) and underscore (_) are not allowed in the top-level domain of the application identifier.";
				}
				else if ((i > 0 && app_id[i - 1] == '.') ||
						(((i + 1) < len) && app_id[i + 1] == '.'))
				{
					error = "Hyphen (-) and underscore (_) are not allowed to be adjacent to a period (.) character";
				}
			}
			else if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')))
			{
				error = "Application identifier can only be made up of alphanumeric ASCII characters (a-z, A-Z, 0-9), hyphen (-), or underscore (_) characters.";
			}
		}
	}
	if (!error && seperator_count < 2)
	{
		error = "Not enough sections to be considered a valid application identifier";
	}
	if (out_error && error)
	{
		*out_error = libmyo_create_error(error, libmyo_error_invalid_argument);
	}
	return error == NULL;
}

bool validate_hub(libmyo_hub_impl_t* hub)
{
	if (bt_manager_init == 0 || hubs.count == 0)
	{
		return false;
	}
	//XXX Actually validate somehow?
	return true;
}

double get_time()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (1000.0 * ts.tv_sec) + (0.000001 * ts.tv_nsec);
}

void myo_bt_callback(const int event, const char *bt_addr, const char *event_data)
{
	switch (event)
	{
	case BT_EVT_RADIO_INIT:
	case BT_EVT_RADIO_SHUTDOWN:
	case BT_EVT_DEVICE_ADDED:
	case BT_EVT_LE_DEVICE_CONNECTED:
	case BT_EVT_LE_NAME_UPDATED:
	case BT_EVT_PAIRING_STARTED: //XXX don't do anything with new devices until has_checked_devices is true, after that it register new commands for new devices
	case BT_EVT_PAIRING_COMPLETE:
	case BT_EVT_DEVICE_DELETED:
	case BT_EVT_LE_DEVICE_DISCONNECTED:
		//TODO
		break;
	}
}

void myo_bt_gatt_services_connected(const char *bdaddr, const char *service, int instance, int err, uint16_t connInt, uint16_t latency, uint16_t superTimeout, void *userData)
{
	if ((bdaddr == NULL) || (service == NULL) || (userData == NULL))
	{
		return;
	}

	//TODO
}

void myo_bt_gatt_services_disconnected(const char *bdaddr, const char *service, int instance, int reason, void *userData)
{
	if ((bdaddr == NULL) || (service == NULL) || (userData == NULL))
	{
		return;
	}

	//TODO
}

/** XXX NOTE: these don't do any safety checks... */
void myo_build_uuids()
{
	myo_list_append_item(&uuids, (void*)myo_create_uuid(ControlService));
	//TODO
}

void myo_cleanup_uuids()
{
	free(myo_list_remove_item(&uuids, 0));
	//TODO
}

const char* myo_get_service_uuid(myohw_services service)
{
	void* result = NULL;
	switch(service)
	{
	case ControlService:
		result = uuids.data[0];
		break;
		//TODO
	}
	return (const char*)result;
}

const char* myo_get_standard_service_uuid(myohw_standard_services service)
{
	//TODO
	return NULL;
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
		if (!validate_app_id(application_identifier, out_error))
		{
			return libmyo_error_invalid_argument;
		}
	}

	pthread_mutex_lock(&manager_lock);

	/* Startup device manager */
	if (bt_manager_init++ == 0)
	{
		if (bt_device_init(myo_bt_callback) != EOK)
		{
			bt_manager_init--;
			pthread_mutex_unlock(&manager_lock);
			if (out_error)
			{
				const char* reason = errno == ENOMEM ? "Out of memory, bluetooth" :
						(errno == EINVAL ? "Bluetooth stack already initialized" : "Could not initialize bluetooth stack");
				*out_error = libmyo_create_error(reason, libmyo_error_runtime);
			}
			return libmyo_error_runtime;
		}
		bt_gatt_callbacks_t gatt_callback;
		gatt_callback.connected = myo_bt_gatt_services_connected;
		gatt_callback.disconnected = myo_bt_gatt_services_disconnected;
		gatt_callback.updated = NULL;
		if (bt_gatt_init(&gatt_callback) != EOK)
		{
			bt_manager_init--;
			pthread_mutex_unlock(&manager_lock);
			bt_device_deinit();
			if (out_error)
			{
				const char* reason = errno == ENOMEM ? "Out of memory, GATT bluetooth" :
						(errno == EACCES ? "No permission to initialize GATT bluetooth" :
								(errno == ENODEV ? "GATT bluetooth is unavailable" : "Could not initialize GATT bluetooth"));
				*out_error = libmyo_create_error(reason, libmyo_error_runtime);
			}
			return libmyo_error_runtime;
		}
		if (bt_le_init(NULL) != EOK)
		{
			bt_manager_init--;
			pthread_mutex_unlock(&manager_lock);
			bt_gatt_deinit();
			bt_device_deinit();
			if (out_error)
			{
				const char* reason = errno == ENOMEM ? "Out of memory, low energy bluetooth" :
						(errno == EACCES ? "No permission to initialize low energy bluetooth" :
								(errno == ENODEV ? "Low energy bluetooth is unavailable" : "Could not initialize low energy bluetooth"));
				*out_error = libmyo_create_error(reason, libmyo_error_runtime);
			}
			return libmyo_error_runtime;
		}

#if BBNDK_VERSION_AT_LEAST(10,3,0)
		//XXX 10.3.1 Gold needs this, but we'll enable it anyway as a precaution
		//https://twitter.com/ekkescorner/status/535774701856256000
		bt_ldev_set_filters(BT_EVT_ALL_EVENT, true);
#endif

		myo_build_uuids();
	}

	/* Setup hub */
	libmyo_hub_impl_t* hub = malloc(sizeof(libmyo_hub_impl_t));
	if (!hub || !myo_list_append_item(&hubs, hub))
	{
		if (hub)
		{
			free(hub);
		}
		if (bt_manager_init == 1)
		{
			myo_cleanup_uuids();
			bt_le_deinit();
			bt_gatt_deinit();
			bt_device_deinit();
			bt_manager_init = 0;
		}
		pthread_mutex_unlock(&manager_lock);
		if (out_error)
		{
			*out_error = libmyo_create_error("Out of memory, hub", libmyo_error_runtime);
		}
		return libmyo_error_runtime;
	}
	*out_hub = hub;
	hub->app_id = strdup(application_identifier);
	memset(&hub->myos, 0, sizeof(libmyo_list_t));
	hub->locking_policy = libmyo_locking_policy_standard; //XXX Is this correct?

	pthread_mutex_unlock(&manager_lock);

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

	pthread_mutex_lock(&manager_lock);

	libmyo_hub_impl_t* hub_impl = (libmyo_hub_impl_t*)hub;
	if (!validate_hub(hub_impl))
	{
		if (out_error)
		{
			*out_error = libmyo_create_error("Not a valid hub", libmyo_error);
		}
		return libmyo_error;
	}

	/* Remove hub from list */
	int32_t index = -1;
	for (int32_t i = 0; i < hubs.count; i++)
	{
		if (hubs.data[i] == hub_impl)
		{
			index = i;
			break;
		}
	}
	if (index < 0)
	{
		pthread_mutex_unlock(&manager_lock);
		if (out_error)
		{
			*out_error = libmyo_create_error("Not a valid hub", libmyo_error);
		}
		return libmyo_error;
	}
	myo_list_remove_item(&hubs, index);

	/* Cleanup hub */
	while (hub_impl->myos.count)
	{
		free_myo(myo_list_remove_item(&hub_impl->myos, 0));
	}
	free((void*)hub_impl->app_id);
	hub_impl->app_id = NULL;
	free(hub_impl);

	/* Shutdown device manager */
	if (bt_manager_init == 1)
	{
		myo_cleanup_uuids();
		bt_le_deinit();
		bt_gatt_deinit();
		bt_device_deinit();

		/* Cleanup commands */
		libmyo_cmd_chain_t* chain = commands;
		while (!__sync_bool_compare_and_swap(&commands, chain, NULL))
		{
			chain = commands;
		}
		__sync_bool_compare_and_swap(&last_command, last_command, NULL);
		while (chain)
		{
			libmyo_cmd_chain_t* cmd = chain;
			chain = cmd->next;

			free((void*)cmd->cmd.data);
			free((void*)cmd);
		}
	}
	bt_manager_init--;

	pthread_mutex_unlock(&manager_lock);

	return libmyo_success;
}

LIBMYO_EXPORT libmyo_result_t libmyo_set_locking_policy(libmyo_hub_t hub, libmyo_locking_policy_t locking_policy, libmyo_error_details_t* out_error)
{
	if (!hub)
	{
		if (out_error)
		{
			*out_error = libmyo_create_error("hub is NULL", libmyo_error_invalid_argument);
		}
		return libmyo_error_invalid_argument;
	}
	libmyo_hub_impl_t* hub_impl = (libmyo_hub_impl_t*)hub;
	if (!validate_hub(hub_impl))
	{
		if (out_error)
		{
			*out_error = libmyo_create_error("Not a valid hub", libmyo_error);
		}
		return libmyo_error;
	}
	hub_impl->locking_policy = locking_policy;
	return libmyo_success;
}

LIBMYO_EXPORT libmyo_result_t libmyo_set_stream_emg(libmyo_myo_t myo, libmyo_stream_emg_t emg, libmyo_error_details_t* out_error)
{
	if (!myo)
	{
		if (out_error)
		{
			*out_error = libmyo_create_error("myo is NULL", libmyo_error_invalid_argument);
		}
		return libmyo_error_invalid_argument;
	}
	((libmyo_myo_impl_t*)myo)->emg_setting = emg;
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

LIBMYO_EXPORT libmyo_result_t libmyo_myo_unlock(libmyo_myo_t myo, libmyo_unlock_type_t type, libmyo_error_details_t* out_error)
{
	//TODO
	return libmyo_success;
}

LIBMYO_EXPORT libmyo_result_t libmyo_myo_lock(libmyo_myo_t myo, libmyo_error_details_t* out_error)
{
	//TODO
	return libmyo_success;
}

LIBMYO_EXPORT libmyo_result_t libmyo_myo_notify_user_action(libmyo_myo_t myo, libmyo_user_action_type_t type, libmyo_error_details_t* out_error)
{
	//TODO
	return libmyo_success;
}
