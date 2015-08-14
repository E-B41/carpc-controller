#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/*
 * Defines
 */
#define DEBUG                   (0)
#define MAX_COMMANDS            (5)
#define BUTTONS_NB              (10)
#define ENCODERS_NB             (5)
#define ACTION_MAX_SIZE         (500)
#define LINE_MAX_SIZE           (500)

#define RADIO_START_VOL         (1)
#define RADIO_START_FREQ        (10670)
#define XBMC_START_VOL          (RADIO_START_VOL * 6 + 5)
#define XBMC_VOL_INC            (6)


#define SYS_CMD_VOLUME          "volume"
#define XBMCBUILTIN_CMD         "xbmcbuiltin_"
#define XBMCBUTTON_CMD          "KB_"
#define RADIO_CMD               "radio_"

#define TDA_CMD					"tda_"
#define TDA_INIT				"tda_init"
#define TDA_VOL_UP              "tda_volume_up"
#define TDA_VOL_DOWN			"tda_volume_down"
#define TDA_BA_UP				"tda_bass_up"
#define TDA_BA_DOWN				"tda_bass_down"
#define TDA_TR_UP				"tda_treble_up"
#define TDA_TR_DOWN				"tda_treble_down"
#define TDA_FA_FR				"tda_fade_front"
#define TDA_FA_RE				"tda_fade_rear"
#define TDA_BL_LE				"tda_balance_left"
#define TDA_BL_RE				"tda_balance_right"
#define TDA_IN1					"tda_input1"
#define TDA_IN2					"tda_input2"
#define TDA_IN3					"tda_input3"
#define TDA_IN4					"tda_input4"

#define RADIO_CMD_SEEK_DOWN     "radio_seek_down"
#define RADIO_CMD_SEEK_UP       "radio_seek_up"
#define RADIO_CMD_TUNE_DOWN     "radio_tune_down"
#define RADIO_CMD_TUNE_UP       "radio_tune_up"
#define RADIO_CMD_TUNE          "radio_tune_"
#define RADIO_UPDATE_RDS        "radio_update_rds"
#define RADIO_UPDATE_FREQUENCY  "radio_update_frequency"

#define NAV_CMD_UP              "nav_up"
#define NAV_CMD_DOWN            "nav_down"
#define NAV_CMD_LEFT            "nav_left"
#define NAV_CMD_RIGHT           "nav_right"
#define NAV_CMD_ZOOM_IN         "nav_zoom_in"
#define NAV_CMD_ZOOM_OUT        "nav_zoom_out"


#define SYSTEM_MODE_TOGGLE      "system_mode_toggle"

#if DEBUG
#define print(...)              printf(__VA_ARGS__)
#else
#define print(...)              printf(" ")
#endif

#ifdef __cplusplus
}
#endif

#endif /* _CONFIG_H_ */
