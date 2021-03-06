/*
 * Copyright (C) 2013-2015 Thalmic Labs Inc.
 * Copyright (C) 2014-2015 Vincent Simonetti
 *
 * See LICENSE for details.
 *
 * Created on: Oct 20, 2014
 */

#include <math.h>
#include "myo_internal.h"

LIBMYO_EXPORT uint32_t libmyo_event_get_type(libmyo_event_t event)
{
	/* Just let it segfault if event is null/invalid because we have no way to return 0 anyway (it's the paired event) */
	return ((libmyo_event_impl_t*)event)->type;
}

LIBMYO_EXPORT uint64_t libmyo_event_get_timestamp(libmyo_event_t event)
{
	if (event)
	{
		return ((libmyo_event_impl_t*)event)->timestamp;
	}
	return 0ULL;
}

LIBMYO_EXPORT libmyo_myo_t libmyo_event_get_myo(libmyo_event_t event)
{
	if (event)
	{
		return ((libmyo_event_impl_t*)event)->myo;
	}
	return NULL;
}

LIBMYO_EXPORT unsigned int libmyo_event_get_firmware_version(libmyo_event_t event, libmyo_version_component_t comp)
{
	//TODO
	return 0;
}

LIBMYO_EXPORT libmyo_arm_t libmyo_event_get_arm(libmyo_event_t event)
{
	//TODO
	return libmyo_arm_unknown;
}

LIBMYO_EXPORT libmyo_x_direction_t libmyo_event_get_x_direction(libmyo_event_t event)
{
	//TODO
	return libmyo_x_direction_unknown;
}

LIBMYO_EXPORT float libmyo_event_get_orientation(libmyo_event_t event, libmyo_orientation_index index)
{
	//TODO
	return NAN;
}

LIBMYO_EXPORT float libmyo_event_get_accelerometer(libmyo_event_t event, unsigned int index)
{
	//TODO
	return NAN;
}

LIBMYO_EXPORT float libmyo_event_get_gyroscope(libmyo_event_t event, unsigned int index)
{
	//TODO
	return NAN;
}

LIBMYO_EXPORT libmyo_pose_t libmyo_event_get_pose(libmyo_event_t event)
{
	//TODO
	return libmyo_pose_unknown;
}

LIBMYO_EXPORT int8_t libmyo_event_get_rssi(libmyo_event_t event)
{
	//TODO
	return 0;
}

LIBMYO_EXPORT int8_t libmyo_event_get_emg(libmyo_event_t event, unsigned int sensor)
{
	//TODO
	return 0;
}
