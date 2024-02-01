#ifndef __COLOR_H__
#define __COLOR_H__

// Attributes
#define attr_normal     "0" // 正常
#define attr_bold       "1" // 加粗/高亮
#define attr_underlined "4" // 下划线
#define attr_blinking   "5" // 闪烁
#define attr_reversed   "7" // 反转
#define attr_concealed  "8" // 隐藏

// Foreground color
// Normal
#define fg_black        "30"
#define fg_red          "31"
#define fg_green        "32"
#define fg_yellow       "33"
#define fg_blue         "34"
#define fg_purple       "35"
#define fg_cyan         "36"
#define fg_grey         "37"
// Lighter
#define fg_dark_grey    "90"
#define fg_light_red    "91"
#define fg_light_green  "92"
#define fg_light_yellow "93"
#define fg_light_blue   "94"
#define fg_light_purple "95"
#define fg_light_cyan   "96"
#define fg_light_grey   "97"

// Background color
// Normal
#define bg_black        "40"
#define bg_red          "41"
#define bg_green        "42"
#define bg_yellow       "43"
#define bg_blue         "44"
#define bg_purple       "45"
#define bg_cyan         "46"
#define bg_grey         "47"
// Lighter
#define bg_dark_grey    "100"
#define bg_light_red    "101"
#define bg_light_green  "102"
#define bg_light_yellow "103"
#define bg_light_blue   "104"
#define bg_light_purple "105"
#define bg_light_cyan   "106"
#define bg_light_grey   "107"

// Easy use
#define NORMAL          "\x1b[0m"
#define BLACK           "\x1b[" attr_normal ";" fg_black "m"
#define RED             "\x1b[" attr_normal ";" fg_red "m"
#define GREEN           "\x1b[" attr_normal ";" fg_green "m"
#define YELLOW          "\x1b[" attr_normal ";" fg_yellow "m"
#define BLUE            "\x1b[" attr_normal ";" fg_blue "m"
#define PURPLE          "\x1b[" attr_normal ";" fg_purple "m"
#define CYAN            "\x1b[" attr_normal ";" fg_cyan "m"
#define GREY            "\x1b[" attr_normal ";" fg_grey "m"
#define L_DKGREY        "\x1b[" attr_normal ";" fg_dark_grey "m"
#define L_RED           "\x1b[" attr_normal ";" fg_light_red "m"
#define L_GREEN         "\x1b[" attr_normal ";" fg_light_green "m"
#define L_YELLOW        "\x1b[" attr_normal ";" fg_light_yellow "m"
#define L_BLUE          "\x1b[" attr_normal ";" fg_light_blue "m"
#define L_PURPLE        "\x1b[" attr_normal ";" fg_light_purple "m"
#define L_CYAN          "\x1b[" attr_normal ";" fg_light_cyan "m"
#define L_GREY          "\x1b[" attr_normal ";" fg_light_grey "m"
#define B_DKGREY        "\x1b[" attr_bold ";" fg_dark_grey "m"
#define B_RED           "\x1b[" attr_bold ";" fg_red "m"
#define B_GREEN         "\x1b[" attr_bold ";" fg_green "m"
#define B_ORANGE        "\x1b[" attr_bold ";" fg_yellow "m"
#define B_BLUE          "\x1b[" attr_bold ";" fg_blue "m"
#define B_PURPLE        "\x1b[" attr_bold ";" fg_purple "m"
#define B_CYAN          "\x1b[" attr_bold ";" fg_cyan "m"
#define B_GREY          "\x1b[" attr_bold ";" fg_grey "m"

#endif

// enum ColorAttribute {
//     caNORMAL = 0,
//     caBOLD = 1,
//     caUNDERLINE = 4,
//     caBLINKING = 5,
//     caREVERSED = 7,
//     caCONCEALED = 8
// };

// enum ForegroundColor {
//     fgBLACK = 30,
//     fgRED = 31,
//     fgGREEN = 32,
//     fgORANGE = 33,
//     fgBLUE = 34,
//     fgPURPLE = 35,
//     fgCYAN = 36,
//     fgGREY = 37,
//     fgGRAY = 37,
//     fgDARKGREY = 90,
//     fgDARKGRAY = 90,
//     fgLIGHTRED = 91,
//     fgLIGHTGREEN = 92,
//     fgYELLOW = 93,
//     fgLIGHTBLUE = 94,
//     fgLIGHTPURPLE = 95,
//     fgTURQUOISE = 96
// };

// enum BackgroundColor {
//     bgBLACK = 40,
//     bgRED = 41,
//     bgGREEN = 42,
//     bgORANGE = 43,
//     bgBLUE = 44,
//     bgPURPLE = 45,
//     bgCYAN = 46,
//     bgGREY = 47,
//     bgGRAY = 47,
//     bgDARKGREY = 100,
//     bgDARKGRAY = 100,
//     bgLIGHTRED = 101,
//     bgLIGHTGREEN = 102,
//     bgYELLOW = 103,
//     bgLIGHTBLUE = 104,
//     bgLIGHTPURPLE = 105,
//     bgTURQUOISE = 106
// };