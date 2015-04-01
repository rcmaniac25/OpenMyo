/*
 * Copyright (C) 2013-2015 Thalmic Labs Inc.
 * Copyright (C) 2014-2015 Vincent Simonetti
 *
 * See LICENSE for details.
 *
 * Created on: Mar 31, 2015
 */

#include "myo_internal.h"

#include <stdio.h>
#include <strings.h>

bool myo_list_append_item(libmyo_list_t* list, void* item)
{
	if (list && item)
	{
		if (list->count >= list->allocated)
		{
			uint32_t new_allocation = list->allocated * 2;
			if (new_allocation == 0)
			{
				new_allocation = 4;
			}
			void** new_data = (void**)realloc(list->data, sizeof(void*) * new_allocation);
			if (new_data)
			{
				list->data = new_data;
				list->allocated = new_allocation;
			}
			else
			{
				return false;
			}
		}
		list->data[list->count++] = item;
		return true;
	}
	return false;
}

void* myo_list_remove_item_ctrl_free(libmyo_list_t* list, uint32_t index, bool free_contents_on_empty)
{
	if (list && (index < list->count))
	{
		void* old = list->data[index];
		list->data[index] = NULL;
		if (index != (--list->count))
		{
			memmove(&list->data[index], &list->data[index + 1], sizeof(void*) * (list->count - index));
		}
		if (free_contents_on_empty && list->count == 0)
		{
			myo_list_free_contents(list);
		}
		return old;
	}
	return NULL;
}

void myo_list_free_contents(libmyo_list_t* list)
{
	if (list)
	{
		list->count = 0;
		list->allocated = 0;
		free(list->data);
		list->data = NULL;
	}
}

static const uint8_t myo_service_base[] = MYO_SERVICE_BASE_UUID;

const char* myo_create_uuid(myohw_services service)
{
	char* buffer = (char*)malloc(39); //0xXXXXSSSS-XXXX-XXXX-XXXX-XXXXXXXXXXXX\0
	if (!buffer)
	{
		return NULL;
	}
	if (snprintf(buffer, 39, "0x%02X%02X%04X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
			myo_service_base[15], myo_service_base[14], service,
			myo_service_base[11], myo_service_base[10],
			myo_service_base[9], myo_service_base[8],
			myo_service_base[7], myo_service_base[6],
			myo_service_base[5], myo_service_base[4], myo_service_base[3], myo_service_base[2], myo_service_base[1], myo_service_base[0]) != 38)
	{
		free(buffer);
		return NULL;
	}
	return buffer;
}

const char* myo_create_standard_uuid(myohw_standard_services service)
{
	char* buffer = (char*)malloc(7); //0xSSSS\0
	if (!buffer)
	{
		return NULL;
	}
	if (snprintf(buffer, 7, "0x%04X", service) != 6)
	{
		free(buffer);
		return NULL;
	}
	return buffer;
}
