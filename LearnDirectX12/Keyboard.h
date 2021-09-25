#pragma once
#include "CommonDefine.h"
#define NOMINMAX
#include <Windows.h>

struct KeyCode {
	enum KeyCodeEnum;
	friend KeyCode operator+(char charCode, KeyCodeEnum sysCode);
	friend bool operator&(KeyCode code, char charCode);
	friend bool operator&(KeyCode code, KeyCodeEnum sysCode);
public:
	union {
		struct {
			char	charCode;	// char code
			char	sysCode;	// system code
		};
		int16_t	code;
	};
};

enum KeyCode::KeyCodeEnum {
	KEYCODE_UNKNOW = (0x1 << 16),
	KEYCODE_ESC,
	KEYCODE_SPACE,
	KEYCODE_ENTER,
	KEYCODE_TABLE,
	KEYCODE_BACKSPACE,
	KEYCODE_INSERT,
	KEYCODE_DELETE,
	KEYCODE_F1,
	KEYCDOE_F2,
	KEYCODE_F3,
	KEYCDOE_F4,
	KEYCODE_F5,
	KEYCODE_F6,
	KEYCODE_F7,
	KEYCODE_F8,
	KEYCODE_F9,
	KEYCODE_F10,
};

class Window;
class Keyboard {
	Window	*window;
public:
	
};

