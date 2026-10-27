// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 1993-2026 Shannon Smith

/*
 * Resource-owned Islamic arithmetic families.
 *
 * Civil/tabular arithmetic and the bounded Umm al-Qura table stay in one
 * module so their epochs, month rules and conversion boundaries are reviewed
 * together.  No locale presentation lives here.
 */

#include "calendar-arithmetic-internal.h"

#include "integer-math.h"
#include "julian-day.h"

enum
{
    CALENDAR_ISLAMIC_CIVIL_EPOCH_JDN = 1948440,
    CALENDAR_ISLAMIC_TBLA_EPOCH_JDN = 1948439
};

#define UMM_AL_QURA_FIRST_YEAR G_GINT64_CONSTANT(1300)
#define UMM_AL_QURA_LAST_YEAR G_GINT64_CONSTANT(1600)
#define UMM_AL_QURA_FIRST_JDN G_GINT64_CONSTANT(2408762)

/*
 * Umm al-Qura month lengths for 1300-1600 AH. Each 12-bit mask stores
 * Muharram in bit 11 through Dhu al-Hijjah in bit 0; one means 30 days and
 * zero means 29. The table is a compact projection of the maintained
 * 1300-1600 data used by the CLDR/ICU-compatible ecosystem. Calendar owns the
 * runtime representation so conversion no longer requires ICU.
 */


/* Cumulative days from 1300-01-01 to each year boundary, plus the 1601 end. */


static gboolean
islamic_is_leap(gint64 year)
{
    const gint64 numerator = calendar_i64_add_saturating(
        calendar_i64_multiply_saturating(11, year), 14);

    return calendar_positive_modulo(numerator, 30) < 11;
}

static gint
islamic_month_length(gint64 year,
                     gint month)
{
    if (month < 1 || month > 12)
        return 0;
    if (month == 12 && islamic_is_leap(year))
        return 30;
    return (month % 2) == 1 ? 30 : 29;
}

static gint64
islamic_to_jdn(gint64 year,
               gint month,
               gint day,
               gint64 epoch)
{
    const gint64 month_index = month - 1;
    const gint64 month_days = calendar_floor_divide(
        calendar_i64_add_saturating(
            calendar_i64_multiply_saturating(59, month_index), 1),
        2);
    gint64 result = calendar_i64_subtract_saturating(epoch, 1);

    result = calendar_i64_add_saturating(result, day);
    result = calendar_i64_add_saturating(result, month_days);
    result = calendar_i64_add_saturating(
        result,
        calendar_i64_multiply_saturating(
            354, calendar_i64_subtract_saturating(year, 1)));
    result = calendar_i64_add_saturating(
        result,
        calendar_floor_divide(
            calendar_i64_add_saturating(
                3, calendar_i64_multiply_saturating(11, year)),
            30));
    return result;
}

static void
islamic_from_jdn(gint64 jdn,
                 gint64 epoch,
                 CalendarCalendarFields *fields)
{
    const gint64 year = calendar_floor_divide(
        calendar_i64_add_saturating(
            calendar_i64_multiply_saturating(
                30, calendar_i64_subtract_saturating(jdn, epoch)),
            10646),
        10631);
    gint month = 1;

    while (month < 12 &&
           jdn >= islamic_to_jdn(year, month + 1, 1, epoch))
    {
        month++;
    }

    *fields = (CalendarCalendarFields){
        .year = year,
        .month = month,
        .day = (gint)calendar_i64_add_saturating(
            calendar_i64_subtract_saturating(
                jdn, islamic_to_jdn(year, month, 1, epoch)),
            1),
        .auxiliary = 0,
        .special = FALSE
    };
}


gboolean
calendar_arithmetic_islamic_fields_from_jdn(
    CalendarCalendarMode mode,
    gint64 jdn,
    CalendarCalendarFields *fields)
{
    (void)mode;
    switch (mode)
    {
        
        case CALENDAR_CALENDAR_MODE_ISLAMIC_TBLA:
            islamic_from_jdn(
                jdn, CALENDAR_ISLAMIC_TBLA_EPOCH_JDN, fields);
            return TRUE;
        
        default:
            return FALSE;
    }
}

gint64
calendar_arithmetic_islamic_fields_to_jdn(
    CalendarCalendarMode mode,
    gint64 year,
    gint month,
    gint day)
{
    (void)mode;
    switch (mode)
    {
        
        case CALENDAR_CALENDAR_MODE_ISLAMIC_TBLA:
            return islamic_to_jdn(
                year, month, day, CALENDAR_ISLAMIC_TBLA_EPOCH_JDN);
        
        default:
            return CALENDAR_UNIX_EPOCH_JDN;
    }
}

gint
calendar_arithmetic_islamic_month_length(
    CalendarCalendarMode mode,
    gint64 year,
    gint month)
{
    (void)mode;
    switch (mode)
    {
        case CALENDAR_CALENDAR_MODE_ISLAMIC_TBLA:
            return islamic_month_length(year, month);
        
        default:
            return 0;
    }
}


