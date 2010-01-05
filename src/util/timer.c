/**************************************************************************
*
* Copyright (C) 2009 Andreas.Fink (Andreas.Fink85@gmail.com)
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**************************************************************************/

#include <time.h>
#include <stdlib.h>

#include "timer.h"

GSList* timeout_list = 0;
struct timeval next_timeout;

void add_timeout_intern(int value_msec, int interval_msec, void(*_callback)(void*), void* arg, struct timeout* t);
gint compare_timeouts(gconstpointer t1, gconstpointer t2);
gint compare_timespecs(const struct timespec* t1, const struct timespec* t2);
int timespec_subtract(struct timespec* result, struct timespec* x, struct timespec* y);


/** Implementation notes for timeouts: The timeouts are kept in a GSList sorted by their
	* expiration time.
	* That means that update_next_timeout() only have to consider the first timeout in the list,
	* and callback_timeout_expired() only have to consider the timeouts as long as the expiration time
	* is in the past to the current time.
	* As time measurement we use clock_gettime(CLOCK_MONOTONIC) because this refers to a timer, which
	* reference point lies somewhere in the past and cannot be changed, but just queried.
	* If a single shot timer is installed it will be automatically deleted. I.e. the returned value
	* of add_timeout will not be valid anymore. You do not need to call stop_timeout for these timeouts,
	* however it's save to call it.
**/

const struct timeout* add_timeout(int value_msec, int interval_msec, void (*_callback)(void*), void* arg)
{
	struct timeout* t = malloc(sizeof(struct timeout));
	add_timeout_intern(value_msec, interval_msec, _callback, arg, t);
	return t;
}


void change_timeout(const struct timeout *t, int value_msec, int interval_msec, void(*_callback)(), void* arg)
{
	if ( g_slist_find(timeout_list, t) == 0 )
		printf("timeout already deleted...");
	else {
		timeout_list = g_slist_remove(timeout_list, t);
		add_timeout_intern(value_msec, interval_msec, _callback, arg, (struct timeout*)t);
	}
}


void update_next_timeout()
{
	if (timeout_list) {
		struct timeout* t = timeout_list->data;
		struct timespec cur_time;
		struct timespec next_timeout2 = { .tv_sec=next_timeout.tv_sec, .tv_nsec=next_timeout.tv_usec*1000 };
		clock_gettime(CLOCK_MONOTONIC, &cur_time);
		if (timespec_subtract(&next_timeout2, &t->timeout_expires, &cur_time)) {
			next_timeout.tv_sec = 0;
			next_timeout.tv_usec = 0;
		}
		else {
			next_timeout.tv_sec = next_timeout2.tv_sec;
			next_timeout.tv_usec = next_timeout2.tv_nsec/1000;
		}
	}
	else
		next_timeout.tv_sec = -1;
}


void callback_timeout_expired()
{
	struct timespec cur_time;
	struct timeout* t;
	while (timeout_list) {
		clock_gettime(CLOCK_MONOTONIC, &cur_time);
		t = timeout_list->data;
		if (compare_timespecs(&t->timeout_expires, &cur_time) <= 0) {
			// it's time for the callback function
			t->_callback(t->arg);
			if (g_slist_find(timeout_list, t)) {
				// if _callback() calls stop_timeout(t) the timeout 't' was freed and is not in the timeout_list
				timeout_list = g_slist_remove(timeout_list, t);
				if (t->interval_msec > 0)
					add_timeout_intern(t->interval_msec, t->interval_msec, t->_callback, t->arg, t);
				else
					free(t);
			}
		}
		else
			return;
	}
}


void stop_timeout(const struct timeout* t)
{
	// if not in the list, it was deleted in callback_timeout_expired
	if (g_slist_find(timeout_list, t)) {
		timeout_list = g_slist_remove(timeout_list, t);
		free((void*)t);
	}
}


void stop_all_timeouts()
{
	while (timeout_list) {
		free(timeout_list->data);
		timeout_list = g_slist_remove(timeout_list, timeout_list->data);
	}
}


void add_timeout_intern(int value_msec, int interval_msec, void(*_callback)(), void* arg, struct timeout *t)
{
	t->interval_msec = interval_msec;
	t->_callback = _callback;
	t->arg = arg;
	struct timespec expire;
	clock_gettime(CLOCK_MONOTONIC, &expire);
	expire.tv_sec += value_msec / 1000;
	expire.tv_nsec += (value_msec % 1000)*1000000;
	if (expire.tv_nsec >= 1000000000) {  // 10^9
		expire.tv_sec++;
		expire.tv_nsec -= 1000000000;
	}
	t->timeout_expires = expire;
	timeout_list = g_slist_insert_sorted(timeout_list, t, compare_timeouts);
}


gint compare_timeouts(gconstpointer t1, gconstpointer t2)
{
	return compare_timespecs(&((const struct timeout*)t1)->timeout_expires,
													 &((const struct timeout*)t2)->timeout_expires);
}


gint compare_timespecs(const struct timespec* t1, const struct timespec* t2)
{
	if (t1->tv_sec < t2->tv_sec)
		return -1;
	else if (t1->tv_sec == t2->tv_sec) {
		if (t1->tv_nsec < t2->tv_nsec)
			return -1;
		else if (t1->tv_nsec == t2->tv_nsec)
			return 0;
		else
			return 1;
	}
	else
		return 1;
}

int timespec_subtract(struct timespec* result, struct timespec* x, struct timespec* y)
{
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_nsec < y->tv_nsec) {
		int nsec = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
		y->tv_nsec -= 1000000000 * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_nsec - y->tv_nsec > 1000000000) {
		int nsec = (x->tv_nsec - y->tv_nsec) / 1000000000;
		y->tv_nsec += 1000000000 * nsec;
		y->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait. tv_nsec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_nsec = x->tv_nsec - y->tv_nsec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}
