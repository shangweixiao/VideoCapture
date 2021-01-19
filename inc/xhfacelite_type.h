#pragma once

#if defined(_MSC_VER)
#define XHF_API __stdcall
#else
#define XHF_API
#endif

#ifdef __cplusplus
extern "C" {
#endif
	typedef void*   XHF_SESS;

	typedef struct {
		int x;
		int y;
		int w;
		int h;
	} face_rect;

	typedef struct {
		float x;
		float y;
	} key_point;

	typedef struct {
		/* Face Rect Detect */
		face_rect   rect;
		float		score;

		/* Face Pose Locate */
		key_point   points[81];
		int			size;
	} face_info;
#ifdef __cplusplus
}
#endif
