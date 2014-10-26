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
#include "myo/libmyo.h"

libmyo_error_details_t libmyo_create_error(const char* error, libmyo_result_t error_kind);

#endif /* MYO_INTERNAL_H_ */
