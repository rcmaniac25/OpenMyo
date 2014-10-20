/*
 * event.c
 *
 * Copyright (C) 2013-2014 Thalmic Labs Inc.
 * Developed under violation of the Myo SDK license agreement. See LICENSE.txt for details.
 * AKA, use at your own risk.
 *
 *  Created on: Oct 20, 2014
 *      Author: Vincent Simonetti
 */

#include <stdlib.h>
#include <math.h>
#include "myo/libmyo.h"

LIBMYO_EXPORT uint32_t libmyo_event_get_type(libmyo_event_t event)
{
	//TODO
	return 0;
}

LIBMYO_EXPORT uint64_t libmyo_event_get_timestamp(libmyo_event_t event)
{
	//TODO
	return 0ULL;
}

LIBMYO_EXPORT libmyo_myo_t libmyo_event_get_myo(libmyo_event_t event)
{
	//TODO
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
