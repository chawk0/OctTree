#include "Log.h"

char* Log::SDLEvents[] = {
    "SDL_NOEVENT",				/* Unused (do not remove) */
    "SDL_ACTIVEEVENT",			/* Application loses/gains visibility */
    "SDL_KEYDOWN",				/* Keys pressed */
    "SDL_KEYUP",				/* Keys released */
    "SDL_MOUSEMOTION",			/* Mouse moved */
    "SDL_MOUSEBUTTONDOWN",		/* Mouse button pressed */
    "SDL_MOUSEBUTTONUP",		/* Mouse button released */
    "SDL_JOYAXISMOTION",		/* Joystick axis motion */
    "SDL_JOYBALLMOTION",		/* Joystick trackball motion */
    "SDL_JOYHATMOTION",			/* Joystick hat position change */
    "SDL_JOYBUTTONDOWN",		/* Joystick button pressed */
    "SDL_JOYBUTTONUP",			/* Joystick button released */
    "SDL_QUIT",					/* User-requested quit */
    "SDL_SYSWMEVENT",			/* System specific event */
    "SDL_EVENT_RESERVEDA",		/* Reserved for future use.. */
    "SDL_EVENT_RESERVEDB",		/* Reserved for future use.. */
    "SDL_VIDEORESIZE",			/* User resized video mode */
    "SDL_VIDEOEXPOSE",			/* Screen needs to be redrawn */
    "SDL_EVENT_RESERVED2",		/* Reserved for future use.. */
    "SDL_EVENT_RESERVED3",		/* Reserved for future use.. */
    "SDL_EVENT_RESERVED4",		/* Reserved for future use.. */
    "SDL_EVENT_RESERVED5",		/* Reserved for future use.. */
    "SDL_EVENT_RESERVED6",		/* Reserved for future use.. */
    "SDL_EVENT_RESERVED7",		/* Reserved for future use.. */
    "SDL_USEREVENT_23",
    "SDL_USEREVENT_24",
    "SDL_USEREVENT_25",
    "SDL_USEREVENT_26",
    "SDL_USEREVENT_27",
    "SDL_USEREVENT_28",
    "SDL_USEREVENT_29",
    "SDL_USEREVENT_30",
    "SDL_USEREVENT_31",
    "SDL_NUMEVENTS"
};

char Log::_buffer[1024];

void Log::verbose(const char* v)
{
#ifdef LOG_COMPILE
	cout << v << endl;
#endif
}

void Log::verbosef(const char* v, ...)
{
#ifdef LOG_COMPILE
	va_list args;
	int n;

	_buffer[0] = 0;

	va_start(args, v);
	n = vsnprintf_s(_buffer, 1024, _TRUNCATE, v, args);
	va_end(args);

	cout << _buffer << endl;
#endif
}

void Log::error(const char* e)
{
#ifdef LOG_COMPILE
	cout << e << endl;
#endif
}

void Log::errorf(const char* e, ...)
{
#ifdef LOG_COMPILE
	va_list args;
	int n;

	_buffer[0] = 0;

	va_start(args, e);
	n = vsnprintf_s(_buffer, 1024, _TRUNCATE, e, args);
	va_end(args);

	cout << _buffer << endl;
#endif
}
