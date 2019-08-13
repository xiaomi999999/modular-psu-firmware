/* / mcu / sound.h
* EEZ PSU Firmware
* Copyright (C) 2015-present, Envox d.o.o.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if OPTION_DISPLAY

#include <eez/apps/psu/psu.h>

#include <math.h>

#if OPTION_ENCODER
#include <eez/modules/mcu/encoder.h>
#endif

#include <eez/apps/psu/gui/data.h>
#include <eez/apps/psu/gui/edit_mode.h>
#include <eez/apps/psu/gui/edit_mode_step.h>
#include <eez/apps/psu/gui/psu.h>
#include <eez/gui/touch.h>
#include <eez/sound.h>

#define CONF_GUI_EDIT_MODE_STEP_THRESHOLD_PX 5

namespace eez {
namespace psu {
namespace gui {
namespace edit_mode_step {

using data::Value;

static const Value CONF_GUI_U_STEPS[] = { data::Value(5.0f, UNIT_UNKNOWN),
                                          data::Value(2.0f, UNIT_UNKNOWN),
                                          data::Value(1.0f, UNIT_UNKNOWN),
                                          data::Value(0.5f, UNIT_UNKNOWN),
                                          data::Value(0.1f, UNIT_UNKNOWN) };

static const Value CONF_GUI_I_STEPS[] = { data::Value(0.5f, UNIT_UNKNOWN),
                                          data::Value(0.25f, UNIT_UNKNOWN),
                                          data::Value(0.1f, UNIT_UNKNOWN),
                                          data::Value(0.05f, UNIT_UNKNOWN),
                                          data::Value(0.01f, UNIT_UNKNOWN) };

static const Value CONF_GUI_P_STEPS[] = { data::Value(10.0f, UNIT_UNKNOWN),
                                          data::Value(5.0f, UNIT_UNKNOWN),
                                          data::Value(2.0f, UNIT_UNKNOWN),
                                          data::Value(1.0f, UNIT_UNKNOWN),
                                          data::Value(0.5f, UNIT_UNKNOWN) };

static const Value CONF_GUI_TEMP_STEPS[] = { data::Value(20.0f, UNIT_UNKNOWN),
                                             data::Value(10.0f, UNIT_UNKNOWN),
                                             data::Value(5.0f, UNIT_UNKNOWN),
                                             data::Value(2.0f, UNIT_UNKNOWN),
                                             data::Value(1.0f, UNIT_UNKNOWN) };

static const Value CONF_GUI_TIME_STEPS[] = { data::Value(30.0f, UNIT_UNKNOWN),
                                             data::Value(20.0f, UNIT_UNKNOWN),
                                             data::Value(10.0f, UNIT_UNKNOWN),
                                             data::Value(5.0f, UNIT_UNKNOWN),
                                             data::Value(1.0f, UNIT_UNKNOWN) };


static int g_stepIndex[CH_MAX][2];

static const int DEFAULT_INDEX = 3;

static bool g_changed;
static int g_startPos;

float getStepValue() {
    if (edit_mode::getUnit() == UNIT_VOLT) {
        return getStepValues()[getStepIndex()].getFloat();
    } else {
        return getStepValues()[getStepIndex()].getFloat();
    }
}

int getStepValuesCount() {
    if (edit_mode::getUnit() == UNIT_VOLT) {
        return sizeof(CONF_GUI_U_STEPS) / sizeof(Value);
    } else if (edit_mode::getUnit() == UNIT_AMPER) {
        return sizeof(CONF_GUI_I_STEPS) / sizeof(Value);
    } else if (edit_mode::getUnit() == UNIT_WATT) {
        return sizeof(CONF_GUI_P_STEPS) / sizeof(Value);
    } else if (edit_mode::getUnit() == UNIT_CELSIUS) {
        return sizeof(CONF_GUI_TEMP_STEPS) / sizeof(Value);
    } else if (edit_mode::getUnit() == UNIT_SECOND) {
        return sizeof(CONF_GUI_TIME_STEPS) / sizeof(Value);
    } else {
        return sizeof(CONF_GUI_U_STEPS) / sizeof(Value);
    }
}

const data::Value *getStepValues() {
    if (edit_mode::getUnit() == UNIT_VOLT) {
        return CONF_GUI_U_STEPS;
    } else if (edit_mode::getUnit() == UNIT_AMPER) {
        return CONF_GUI_I_STEPS;
    } else if (edit_mode::getUnit() == UNIT_WATT) {
        return CONF_GUI_P_STEPS;
    } else if (edit_mode::getUnit() == UNIT_CELSIUS) {
        return CONF_GUI_TEMP_STEPS;
    } else if (edit_mode::getUnit() == UNIT_SECOND) {
        return CONF_GUI_TIME_STEPS;
    } else {
        return CONF_GUI_U_STEPS;
    }
}

int getStepIndex() {
    int value;
    if (edit_mode::getUnit() == UNIT_VOLT) {
        value = g_stepIndex[g_focusCursor.i][0];
    } else {
        value = g_stepIndex[g_focusCursor.i][1];
    }

    if (value == 0) {
        value = DEFAULT_INDEX;
    }

    return value - 1;
}

void setStepIndex(int value) {
    if (edit_mode::getUnit() == UNIT_VOLT) {
        g_stepIndex[g_focusCursor.i][0] = value + 1;
    } else {
        g_stepIndex[g_focusCursor.i][1] = value + 1;
    }
}

void increment(int counter, bool playClick) {
    float min = edit_mode::getMin().getFloat();
    float max = edit_mode::getMax().getFloat();
    float stepValue = getStepValue();

    float value = edit_mode::getEditValue().getFloat();

    for (int i = 0; i < abs(counter); ++i) {
        if (counter > 0) {
            if (value == min) {
                value = (floorf(min / stepValue) + 1) * stepValue;
            } else {
                value += stepValue;
            }
            if (value > max) {
                value = max;
            }
        } else {
            if (value == max) {
                value = (ceilf(max / stepValue) - 1) * stepValue;
            } else {
                value -= stepValue;
            }
            if (value < min) {
                value = min;
            }
        }
    }

    if (edit_mode::setValue(value)) {
        g_changed = true;
        if (playClick) {
            sound::playClick();
        }
    }
}

#if OPTION_ENCODER

void onEncoder(int counter) {
    mcu::encoder::enableAcceleration(false);
    increment(counter, false);
}

#endif

void test() {
    if (!g_changed) {
        int d = eez::gui::touch::getX() - g_startPos;
        if (abs(d) >= CONF_GUI_EDIT_MODE_STEP_THRESHOLD_PX) {
            increment(d > 0 ? 1 : -1, true);
        }
    }
}

void onTouchDown() {
    g_startPos = eez::gui::touch::getX();
    g_changed = false;
}

void onTouchMove() {
    test();
}

void onTouchUp() {
}

} // namespace edit_mode_step
} // namespace gui
} // namespace psu
} // namespace eez

#endif