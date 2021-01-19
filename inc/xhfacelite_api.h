#pragma once

#include "xhfacelite_type.h"

enum{
	XHF_OK,                             // 接口调用成功

	XHF_SESS_NULLPTR_ERR = 1,           // 会话地址为空

	XHF_PARAMETER_NULLPTR_ERR = 2,      // 参数地址为空

	XHF_PARAMETER_SET_ERR = 3,          // 参数值不合法

	XHF_MEMORY_ALLOCATE_ERR = 4,        // 内存分配不足

	XHF_FACE_POINTS_ERR = 5,            // 关键点数不对应

	XHF_FACE_EXTRACT_ERR = 6,           // 特征提取失败

	XHF_NO_FACE = 7,                    // 未检测到人脸

	XHF_FACE_NG = 8,                    // 检测到的人脸不合格

	XHF_ERR                             // 接口调用致命错误
};

#ifdef __cplusplus
extern "C" {
#endif
	/* Global prepare for Basic and Advanced Edition */
	int XHF_API XHFInit(const char *model_dir, const char *product_code);
	typedef int (XHF_API *PXHFInit)(const char *, const char *);


	/* Basic Edition Interface */
	/// Create FDB(Face Database)
	int XHF_API FECreate(XHF_SESS *sess, int threads_num=1, int points_num=5, const char *model_type="best");   // points_num: 5 or 81; model_type: "best" or "fast"
	typedef int (XHF_API *PFECreate)(XHF_SESS *, int, int, const char *);

	int XHF_API FELoad(XHF_SESS sess, const char *db_path);
	typedef int (XHF_API *PFELoad)(XHF_SESS, const char *);

	int XHF_API FESave(XHF_SESS sess, const char *db_path);
	typedef int (XHF_API *PFESave)(XHF_SESS, const char *);

	/// Enroll face to FDB
	int XHF_API FEInsert(XHF_SESS sess, unsigned char *data, int width, int height, int *index, face_rect *max_face, float strict=0, int detected=0, int min_face_scale=16);   // strict=0.4, min_face_scale>=2
	typedef int (XHF_API *PFEInsert)(XHF_SESS, unsigned char *, int, int, int *, face_rect *, float, int, int);

	int XHF_API FEDelete(XHF_SESS sess, int index);
	typedef int (XHF_API *PFEDelete)(XHF_SESS, int);

	int XHF_API FECount(XHF_SESS sess, int *num);
	typedef int (XHF_API *PFECount)(XHF_SESS, int *);

	int XHF_API FEClear(XHF_SESS sess);
	typedef int (XHF_API *PFEClear)(XHF_SESS);

	/// Verify(1:1) two faces
	int XHF_API FECompareFace(XHF_SESS sess, unsigned char *data_a, int width_a, int height_a, unsigned char *data_b, int width_b, int height_b, float *score, int min_face_scale=16);	// min_face_scale>=2
	typedef int (XHF_API *PFECompareFace)(XHF_SESS, unsigned char *, int, int , unsigned char *, int, int, float *, int);

	/// Identify(1:N) face from FDB
	int XHF_API FEQueryTopN(XHF_SESS sess, unsigned char *data, int width, int height, int *index_list, float *score_list, int *list_length, int N=1, float threshold=0, float strict=0, int detected=0, int min_face_scale=16);   // strict=0.4, min_face_scale>=2
	typedef int (XHF_API *PFEQueryTopN)(XHF_SESS, unsigned char *, int, int, int *, float *, int *, int, float, float, int, int);

	/// Destroy Face Database
	int XHF_API FEDestroy(XHF_SESS *sess);
	typedef int (XHF_API *PFEDestroy)(XHF_SESS *);


	/* Advanced Edition Interface */
	/// Face Rect Detect
	int XHF_API FDCreate(XHF_SESS *sess);
	typedef int (XHF_API *PFDCreate)(XHF_SESS *);

	int XHF_API FaceDetect(XHF_SESS sess, unsigned char *data, int width, int height, face_info* *faces, int *count, int min_face_scale=16, int max_face_first=1);	// min_face_scale>=2
	typedef int (XHF_API *PFaceDetect)(XHF_SESS, unsigned char *, int, int, face_info* *, int *, int, int);

	int XHF_API FDDestroy(XHF_SESS *sess);
	typedef int (XHF_API *PFDDestroy)(XHF_SESS *);

	/// Face Pose Locate
	int XHF_API PLCreate(XHF_SESS *sess, int points_num=5); // 5 or 81
	typedef int (XHF_API *PPLCreate)(XHF_SESS *, int);

	int XHF_API PoseLocate(XHF_SESS sess, unsigned char *data, int width, int height, face_info* face);
	typedef int (XHF_API *PPoseLocate)(XHF_SESS, unsigned char *, int, int, face_info*);

	int XHF_API PLDestroy(XHF_SESS *sess);
	typedef int (XHF_API *PPLDestroy)(XHF_SESS *);

	/// Face Pose Estimate
	int XHF_API PECreate(XHF_SESS *sess);
	typedef int (XHF_API *PPECreate)(XHF_SESS *);

	int XHF_API PoseEstimate(XHF_SESS sess, unsigned char *data, int width, int height, face_info* face);
	typedef int (XHF_API *PPoseEstimate)(XHF_SESS, unsigned char *, int, int, face_info*);

	int XHF_API PEDestroy(XHF_SESS *sess);
	typedef int (XHF_API *PPEDestroy)(XHF_SESS *);

	/// Face Feature Recog
	int XHF_API FRCreate(XHF_SESS *sess, const char *model_type="best");    // "best" or "fast"
	typedef int (XHF_API *PFRCreate)(XHF_SESS *, const char *);

	int XHF_API FeatureExtract(XHF_SESS sess, unsigned char *data, int width, int height, face_info* face, float* *feature, int *length);
	typedef int (XHF_API *PFeatureExtract)(XHF_SESS, unsigned char *, int, int, face_info*, float* *, int *);

	int XHF_API FeatureCompare(XHF_SESS sess, float* feature_a, float* feature_b, float *score);
	typedef int (XHF_API *PFeatureCompare)(XHF_SESS, float *, float *, float *);

	int XHF_API FRDestroy(XHF_SESS *sess);
	typedef int (XHF_API *PFRDestroy)(XHF_SESS *);


	/* Global finish for Basic and Advanced Edition */
	int XHF_API XHFFinal();
	typedef int (XHF_API *PXHFFinal)();
#ifdef __cplusplus
}
#endif
