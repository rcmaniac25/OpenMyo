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

#include <pthread.h>

#include <btapi/btdevice.h>
#include <btapi/btgatt.h>
#include <btapi/btle.h>

#include <bbndk.h>

//XXX init and shutdown need some cleanup. They were written to be small, but between locks and state changes, it got pretty ugly quickly

pthread_mutex_t manager_lock = PTHREAD_MUTEX_INITIALIZER; /* PTHREAD_RMUTEX_INITIALIZER for recursive mutexes */
static int bt_manager_init = 0; /* Could possibly combine with hub_count... */
static unsigned int hub_count = 0;
static libmyo_hub_impl_t** hubs = NULL;

static libmyo_cmd_chain_t* commands = NULL;
static libmyo_cmd_chain_t* last_command = NULL;

bool validate_app_id(const char* app_id, libmyo_error_details_t* out_error)
{
	const char* error = NULL;
	int section_count = 0;

	unsigned int len = strlen(app_id);
	for (unsigned int i = 0; i < len && !error; i++)
	{
		if (app_id[i] == '.')
		{
			/* Check to make sure that a . separates sections and it isn't just ".." */
			if (i > 0 && app_id[i - 1] != '.' &&
					(i + 1) < len && app_id[i + 1] != '.')
			{
				section_count++;
			}
		}
		else
		{
			char c = app_id[i];
			if (c == '-' || c == '_')
			{
				if (section_count < 1)
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
	if (!error && section_count < 3)
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
	if (bt_manager_init == 0 || hub_count == 0)
	{
		return false;
	}
	//XXX Actually validate somehow?
	return true;
}

void free_myo(libmyo_myo_impl_t* myo)
{
	myo->hub = NULL;
	//TODO
	free(myo);
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
	case BT_EVT_PAIRING_STARTED:
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
	}

	/* Expand hub list */
	libmyo_hub_impl_t** old_hubs = hubs;
	hubs = (libmyo_hub_impl_t**)realloc(hubs, sizeof(libmyo_hub_impl_t*) * (hub_count++));
	if (!hubs)
	{
		if (bt_manager_init == 1)
		{
			bt_le_deinit();
			bt_gatt_deinit();
			bt_device_deinit();
			bt_manager_init = 0;
		}
		hub_count--;
		hubs = old_hubs;
		pthread_mutex_unlock(&manager_lock);
		if (out_error)
		{
			*out_error = libmyo_create_error("Out of memory", libmyo_error_runtime);
		}
		return libmyo_error_runtime;
	}

	/* Setup hub */
	libmyo_hub_impl_t* hub = malloc(sizeof(libmyo_hub_impl_t));
	if (!hub)
	{
		if (hub_count == 1)
		{
			free(hubs);
			hubs = NULL;
			hub_count = 0;
		}
		else
		{
			old_hubs = hubs;
			hubs = (libmyo_hub_impl_t**)realloc(hubs, sizeof(libmyo_hub_impl_t*) * (hub_count--));
			if (!hubs)
			{
				/* Gesh, we can't get a break can we... */
				hubs = old_hubs;
			}
		}
		if (bt_manager_init == 1)
		{
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
	hub->device_count = 0;
	hub->devices = NULL;
	hub->myo_count = 0;
	hub->myos = NULL;
	hub->locking_policy = libmyo_locking_policy_standard; //XXX Is this correct?
	hubs[hub_count - 1] = hub;

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
	if (validate_hub(hub_impl))
	{
		if (out_error)
		{
			*out_error = libmyo_create_error("Not a valid hub", libmyo_error);
		}
		return libmyo_error;
	}

	/* Remove hub from list */
	int index = -1;
	for (int i = 0; i < hub_count; i++)
	{
		if (hubs[i] == hub_impl)
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
	if (hub_count == 1)
	{
		free(hubs);
		hub_count = 0;
		hubs = NULL;
	}
	else
	{
		hubs[index] = NULL;
		hub_count--;
		if (index == 0)
		{
			memcpy(hubs, &hubs[1], sizeof(libmyo_hub_impl_t*) * hub_count);
		}
		else if (index != hub_count)
		{
			memcpy(&hubs[index], &hubs[index + 1], sizeof(libmyo_hub_impl_t*) * (hub_count - index));
		}
		libmyo_hub_impl_t** old_hubs = hubs;
		hubs = (libmyo_hub_impl_t**)realloc(hubs, sizeof(libmyo_hub_impl_t*) * hub_count);
		if (!hubs)
		{
			/* We can ignore the realloc. It will just be freed or resized another time. */
			hubs = old_hubs;
		}
	}

	/* Cleanup hub */
	while (hub_impl->device_count--)
	{
		free_myo(hub_impl->myos[hub_impl->myo_count - 1]);
	}
	free(hub_impl->myos);
	hub_impl->myos = NULL;
	while (hub_impl->device_count--)
	{
		free((void*)hub_impl->devices[hub_impl->device_count - 1]);
	}
	free(hub_impl->devices);
	hub_impl->devices = NULL;
	free((void*)hub_impl->app_id);
	hub_impl->app_id = NULL;
	free(hub_impl);

	/* Shutdown device manager */
	if (bt_manager_init == 1)
	{
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

//bool firstRun = true;
LIBMYO_EXPORT libmyo_result_t libmyo_run(libmyo_hub_t hub, unsigned int duration_ms, libmyo_handler_t handler, void* user_data, libmyo_error_details_t* out_error)
{
	//TODO
	return libmyo_success;
	/* Testing
	if(firstRun)
	{
		firstRun = false;
		bt_remote_device_t *next_remote_device = 0;
		bt_remote_device_t **remote_device_array = bt_disc_retrieve_devices(BT_DISCOVERY_PREKNOWN, 0);
		if (remote_device_array) {
		    for (int i = 0; (next_remote_device = remote_device_array[i]); ++i) {
		        char device_name[128];
		        char device_addr[128];
		        int device_class = -1;
		        int device_type = -1;
		        bool encrypted = false;
		        bool paired = false;
		        const int bufferSize = sizeof(device_name);

		        // Get the friendly name of the remote device. The
		        // friendly name is a string value that makes it easy
		        // to identify the device.
		        if (bt_rdev_get_friendly_name(next_remote_device, device_name, bufferSize) != 0) {
		            // handle error
		        	//TODO
		        }
		        // Retrieve the MAC address of the remote device.
		        if (bt_rdev_get_address(next_remote_device, device_addr) != 0) {
		            // handle error
		        	//TODO
		        }
		        // Retrieve the Class of Device value of the
		        // remote device.
		        device_class = bt_rdev_get_device_class(next_remote_device, BT_COD_DEVICECLASS);
		        device_class = bt_rdev_get_device_class(next_remote_device, BT_COD_MAJORSERVICECLASS);
		        device_class = bt_rdev_get_device_class(next_remote_device, BT_COD_MAJORDEVICECLASS);
		        device_class = bt_rdev_get_device_class(next_remote_device, BT_COD_MINORDEVICECLASS);

		        // Retrieve the remote device type.
				device_type = bt_rdev_get_type(next_remote_device);

				if (bt_rdev_is_encrypted(next_remote_device)) {
					encrypted = true;
				}

				// Determine if the device is paired.
				if (bt_rdev_is_paired(next_remote_device, &paired) != 0) {
					// Handle error
					//TODO
				}

				// Retrieve services
				char **services_array = bt_rdev_get_services(next_remote_device);
				if (services_array != NULL) {
					// Work with retrieved services
					//TODO

					// Free the resources used for the services array.
					bt_rdev_free_services(services_array);
				}

				// If the remote device is a low-energy device,
				// retrieve low-energy services.
				if (device_type == BT_DEVICE_TYPE_LE_PUBLIC || device_type == BT_DEVICE_TYPE_LE_PRIVATE) {

					services_array = bt_rdev_get_services_gatt(next_remote_device);
					if (services_array != NULL) {
						// Work with retrieved services
						const char* service = NULL;
						int k;
						for (k = 0; (service = services_array[k]); ++k) {
							//TODO
							if(service[0] != '0')
							{
								//TODO
								k = k + 1;
								k = k - 1;
							}
						}

						// Free the resources used for the services array.
						bt_rdev_free_services(services_array);
					}
				}
		    }
		    bt_rdev_free_array(remote_device_array);
		}
	}
	return libmyo_success;
	*/
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
