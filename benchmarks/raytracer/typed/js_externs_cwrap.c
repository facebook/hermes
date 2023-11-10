#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_time.h"
#include "sokol_imgui.h"
#include <stdbool.h>

bool igBeginChild_Str_cwrap(char* a0, struct ImVec2* a1, bool a2, int a3){
  return igBeginChild_Str(a0, *a1, a2, a3);
}
bool igBeginChild_ID_cwrap(unsigned int a0, struct ImVec2* a1, bool a2, int a3){
  return igBeginChild_ID(a0, *a1, a2, a3);
}
void igSetNextWindowPos_cwrap(struct ImVec2* a0, int a1, struct ImVec2* a2){
  return igSetNextWindowPos(*a0, a1, *a2);
}
void igSetNextWindowSize_cwrap(struct ImVec2* a0, int a1){
  return igSetNextWindowSize(*a0, a1);
}
void igSetNextWindowSizeConstraints_cwrap(struct ImVec2* a0, struct ImVec2* a1, void* a2, void* a3){
  return igSetNextWindowSizeConstraints(*a0, *a1, a2, a3);
}
void igSetNextWindowContentSize_cwrap(struct ImVec2* a0){
  return igSetNextWindowContentSize(*a0);
}
void igSetNextWindowScroll_cwrap(struct ImVec2* a0){
  return igSetNextWindowScroll(*a0);
}
void igSetWindowPos_Vec2_cwrap(struct ImVec2* a0, int a1){
  return igSetWindowPos_Vec2(*a0, a1);
}
void igSetWindowSize_Vec2_cwrap(struct ImVec2* a0, int a1){
  return igSetWindowSize_Vec2(*a0, a1);
}
void igSetWindowPos_Str_cwrap(char* a0, struct ImVec2* a1, int a2){
  return igSetWindowPos_Str(a0, *a1, a2);
}
void igSetWindowSize_Str_cwrap(char* a0, struct ImVec2* a1, int a2){
  return igSetWindowSize_Str(a0, *a1, a2);
}
void igPushStyleColor_Vec4_cwrap(int a0, struct ImVec4* a1){
  return igPushStyleColor_Vec4(a0, *a1);
}
void igPushStyleVar_Vec2_cwrap(int a0, struct ImVec2* a1){
  return igPushStyleVar_Vec2(a0, *a1);
}
unsigned int igGetColorU32_Vec4_cwrap(struct ImVec4* a0){
  return igGetColorU32_Vec4(*a0);
}
void igDummy_cwrap(struct ImVec2* a0){
  return igDummy(*a0);
}
void igSetCursorPos_cwrap(struct ImVec2* a0){
  return igSetCursorPos(*a0);
}
void igSetCursorScreenPos_cwrap(struct ImVec2* a0){
  return igSetCursorScreenPos(*a0);
}
void igTextColored_cwrap(struct ImVec4* a0, char* a1){
  return igTextColored(*a0, a1);
}
void igTextColoredV_cwrap(struct ImVec4* a0, char* a1, char* a2){
  return igTextColoredV(*a0, a1, a2);
}
bool igButton_cwrap(char* a0, struct ImVec2* a1){
  return igButton(a0, *a1);
}
bool igInvisibleButton_cwrap(char* a0, struct ImVec2* a1, int a2){
  return igInvisibleButton(a0, *a1, a2);
}
void igProgressBar_cwrap(float a0, struct ImVec2* a1, char* a2){
  return igProgressBar(a0, *a1, a2);
}
void igImage_cwrap(void* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec4* a4, struct ImVec4* a5){
  return igImage(a0, *a1, *a2, *a3, *a4, *a5);
}
bool igImageButton_cwrap(char* a0, void* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, struct ImVec4* a5, struct ImVec4* a6){
  return igImageButton(a0, a1, *a2, *a3, *a4, *a5, *a6);
}
bool igVSliderFloat_cwrap(char* a0, struct ImVec2* a1, float* a2, float a3, float a4, char* a5, int a6){
  return igVSliderFloat(a0, *a1, a2, a3, a4, a5, a6);
}
bool igVSliderInt_cwrap(char* a0, struct ImVec2* a1, int* a2, int a3, int a4, char* a5, int a6){
  return igVSliderInt(a0, *a1, a2, a3, a4, a5, a6);
}
bool igVSliderScalar_cwrap(char* a0, struct ImVec2* a1, int a2, void* a3, void* a4, void* a5, char* a6, int a7){
  return igVSliderScalar(a0, *a1, a2, a3, a4, a5, a6, a7);
}
bool igInputTextMultiline_cwrap(char* a0, char* a1, long unsigned int a2, struct ImVec2* a3, int a4, void* a5, void* a6){
  return igInputTextMultiline(a0, a1, a2, *a3, a4, a5, a6);
}
bool igColorButton_cwrap(char* a0, struct ImVec4* a1, int a2, struct ImVec2* a3){
  return igColorButton(a0, *a1, a2, *a3);
}
bool igSelectable_Bool_cwrap(char* a0, bool a1, int a2, struct ImVec2* a3){
  return igSelectable_Bool(a0, a1, a2, *a3);
}
bool igSelectable_BoolPtr_cwrap(char* a0, bool* a1, int a2, struct ImVec2* a3){
  return igSelectable_BoolPtr(a0, a1, a2, *a3);
}
bool igBeginListBox_cwrap(char* a0, struct ImVec2* a1){
  return igBeginListBox(a0, *a1);
}
void igPlotLines_FloatPtr_cwrap(char* a0, float* a1, int a2, int a3, char* a4, float a5, float a6, struct ImVec2* a7, int a8){
  return igPlotLines_FloatPtr(a0, a1, a2, a3, a4, a5, a6, *a7, a8);
}
void igPlotLines_FnFloatPtr_cwrap(char* a0, void* a1, void* a2, int a3, int a4, char* a5, float a6, float a7, struct ImVec2* a8){
  return igPlotLines_FnFloatPtr(a0, a1, a2, a3, a4, a5, a6, a7, *a8);
}
void igPlotHistogram_FloatPtr_cwrap(char* a0, float* a1, int a2, int a3, char* a4, float a5, float a6, struct ImVec2* a7, int a8){
  return igPlotHistogram_FloatPtr(a0, a1, a2, a3, a4, a5, a6, *a7, a8);
}
void igPlotHistogram_FnFloatPtr_cwrap(char* a0, void* a1, void* a2, int a3, int a4, char* a5, float a6, float a7, struct ImVec2* a8){
  return igPlotHistogram_FnFloatPtr(a0, a1, a2, a3, a4, a5, a6, a7, *a8);
}
bool igBeginTable_cwrap(char* a0, int a1, int a2, struct ImVec2* a3, float a4){
  return igBeginTable(a0, a1, a2, *a3, a4);
}
void igPushClipRect_cwrap(struct ImVec2* a0, struct ImVec2* a1, bool a2){
  return igPushClipRect(*a0, *a1, a2);
}
bool igIsRectVisible_Nil_cwrap(struct ImVec2* a0){
  return igIsRectVisible_Nil(*a0);
}
bool igIsRectVisible_Vec2_cwrap(struct ImVec2* a0, struct ImVec2* a1){
  return igIsRectVisible_Vec2(*a0, *a1);
}
bool igBeginChildFrame_cwrap(unsigned int a0, struct ImVec2* a1, int a2){
  return igBeginChildFrame(a0, *a1, a2);
}
unsigned int igColorConvertFloat4ToU32_cwrap(struct ImVec4* a0){
  return igColorConvertFloat4ToU32(*a0);
}
bool igIsMouseHoveringRect_cwrap(struct ImVec2* a0, struct ImVec2* a1, bool a2){
  return igIsMouseHoveringRect(*a0, *a1, a2);
}
struct ImColor* ImColor_ImColor_Vec4_cwrap(struct ImVec4* a0){
  return ImColor_ImColor_Vec4(*a0);
}
void ImDrawList_PushClipRect_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, bool a3){
  return ImDrawList_PushClipRect(a0, *a1, *a2, a3);
}
void ImDrawList_AddLine_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, unsigned int a3, float a4){
  return ImDrawList_AddLine(a0, *a1, *a2, a3, a4);
}
void ImDrawList_AddRect_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, unsigned int a3, float a4, int a5, float a6){
  return ImDrawList_AddRect(a0, *a1, *a2, a3, a4, a5, a6);
}
void ImDrawList_AddRectFilled_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, unsigned int a3, float a4, int a5){
  return ImDrawList_AddRectFilled(a0, *a1, *a2, a3, a4, a5);
}
void ImDrawList_AddRectFilledMultiColor_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6){
  return ImDrawList_AddRectFilledMultiColor(a0, *a1, *a2, a3, a4, a5, a6);
}
void ImDrawList_AddQuad_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, unsigned int a5, float a6){
  return ImDrawList_AddQuad(a0, *a1, *a2, *a3, *a4, a5, a6);
}
void ImDrawList_AddQuadFilled_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, unsigned int a5){
  return ImDrawList_AddQuadFilled(a0, *a1, *a2, *a3, *a4, a5);
}
void ImDrawList_AddTriangle_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, unsigned int a4, float a5){
  return ImDrawList_AddTriangle(a0, *a1, *a2, *a3, a4, a5);
}
void ImDrawList_AddTriangleFilled_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, unsigned int a4){
  return ImDrawList_AddTriangleFilled(a0, *a1, *a2, *a3, a4);
}
void ImDrawList_AddCircle_cwrap(struct ImDrawList* a0, struct ImVec2* a1, float a2, unsigned int a3, int a4, float a5){
  return ImDrawList_AddCircle(a0, *a1, a2, a3, a4, a5);
}
void ImDrawList_AddCircleFilled_cwrap(struct ImDrawList* a0, struct ImVec2* a1, float a2, unsigned int a3, int a4){
  return ImDrawList_AddCircleFilled(a0, *a1, a2, a3, a4);
}
void ImDrawList_AddNgon_cwrap(struct ImDrawList* a0, struct ImVec2* a1, float a2, unsigned int a3, int a4, float a5){
  return ImDrawList_AddNgon(a0, *a1, a2, a3, a4, a5);
}
void ImDrawList_AddNgonFilled_cwrap(struct ImDrawList* a0, struct ImVec2* a1, float a2, unsigned int a3, int a4){
  return ImDrawList_AddNgonFilled(a0, *a1, a2, a3, a4);
}
void ImDrawList_AddText_Vec2_cwrap(struct ImDrawList* a0, struct ImVec2* a1, unsigned int a2, char* a3, char* a4){
  return ImDrawList_AddText_Vec2(a0, *a1, a2, a3, a4);
}
void ImDrawList_AddText_FontPtr_cwrap(struct ImDrawList* a0, struct ImFont* a1, float a2, struct ImVec2* a3, unsigned int a4, char* a5, char* a6, float a7, struct ImVec4* a8){
  return ImDrawList_AddText_FontPtr(a0, a1, a2, *a3, a4, a5, a6, a7, a8);
}
void ImDrawList_AddBezierCubic_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, unsigned int a5, float a6, int a7){
  return ImDrawList_AddBezierCubic(a0, *a1, *a2, *a3, *a4, a5, a6, a7);
}
void ImDrawList_AddBezierQuadratic_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, unsigned int a4, float a5, int a6){
  return ImDrawList_AddBezierQuadratic(a0, *a1, *a2, *a3, a4, a5, a6);
}
void ImDrawList_AddImage_cwrap(struct ImDrawList* a0, void* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, struct ImVec2* a5, unsigned int a6){
  return ImDrawList_AddImage(a0, a1, *a2, *a3, *a4, *a5, a6);
}
void ImDrawList_AddImageQuad_cwrap(struct ImDrawList* a0, void* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, struct ImVec2* a5, struct ImVec2* a6, struct ImVec2* a7, struct ImVec2* a8, struct ImVec2* a9, unsigned int a10){
  return ImDrawList_AddImageQuad(a0, a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, a10);
}
void ImDrawList_AddImageRounded_cwrap(struct ImDrawList* a0, void* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, struct ImVec2* a5, unsigned int a6, float a7, int a8){
  return ImDrawList_AddImageRounded(a0, a1, *a2, *a3, *a4, *a5, a6, a7, a8);
}
void ImDrawList_PathLineTo_cwrap(struct ImDrawList* a0, struct ImVec2* a1){
  return ImDrawList_PathLineTo(a0, *a1);
}
void ImDrawList_PathLineToMergeDuplicate_cwrap(struct ImDrawList* a0, struct ImVec2* a1){
  return ImDrawList_PathLineToMergeDuplicate(a0, *a1);
}
void ImDrawList_PathArcTo_cwrap(struct ImDrawList* a0, struct ImVec2* a1, float a2, float a3, float a4, int a5){
  return ImDrawList_PathArcTo(a0, *a1, a2, a3, a4, a5);
}
void ImDrawList_PathArcToFast_cwrap(struct ImDrawList* a0, struct ImVec2* a1, float a2, int a3, int a4){
  return ImDrawList_PathArcToFast(a0, *a1, a2, a3, a4);
}
void ImDrawList_PathBezierCubicCurveTo_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, int a4){
  return ImDrawList_PathBezierCubicCurveTo(a0, *a1, *a2, *a3, a4);
}
void ImDrawList_PathBezierQuadraticCurveTo_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, int a3){
  return ImDrawList_PathBezierQuadraticCurveTo(a0, *a1, *a2, a3);
}
void ImDrawList_PathRect_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, float a3, int a4){
  return ImDrawList_PathRect(a0, *a1, *a2, a3, a4);
}
void ImDrawList_PrimRect_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, unsigned int a3){
  return ImDrawList_PrimRect(a0, *a1, *a2, a3);
}
void ImDrawList_PrimRectUV_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, unsigned int a5){
  return ImDrawList_PrimRectUV(a0, *a1, *a2, *a3, *a4, a5);
}
void ImDrawList_PrimQuadUV_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, struct ImVec2* a5, struct ImVec2* a6, struct ImVec2* a7, struct ImVec2* a8, unsigned int a9){
  return ImDrawList_PrimQuadUV(a0, *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, a9);
}
void ImDrawList_PrimWriteVtx_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, unsigned int a3){
  return ImDrawList_PrimWriteVtx(a0, *a1, *a2, a3);
}
void ImDrawList_PrimVtx_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, unsigned int a3){
  return ImDrawList_PrimVtx(a0, *a1, *a2, a3);
}
void ImDrawList__PathArcToFastEx_cwrap(struct ImDrawList* a0, struct ImVec2* a1, float a2, int a3, int a4, int a5){
  return ImDrawList__PathArcToFastEx(a0, *a1, a2, a3, a4, a5);
}
void ImDrawList__PathArcToN_cwrap(struct ImDrawList* a0, struct ImVec2* a1, float a2, float a3, float a4, int a5){
  return ImDrawList__PathArcToN(a0, *a1, a2, a3, a4, a5);
}
void ImDrawData_ScaleClipRects_cwrap(struct ImDrawData* a0, struct ImVec2* a1){
  return ImDrawData_ScaleClipRects(a0, *a1);
}
int ImFontAtlas_AddCustomRectFontGlyph_cwrap(struct ImFontAtlas* a0, struct ImFont* a1, short unsigned int a2, int a3, int a4, float a5, struct ImVec2* a6){
  return ImFontAtlas_AddCustomRectFontGlyph(a0, a1, a2, a3, a4, a5, *a6);
}
void ImFont_RenderChar_cwrap(struct ImFont* a0, struct ImDrawList* a1, float a2, struct ImVec2* a3, unsigned int a4, short unsigned int a5){
  return ImFont_RenderChar(a0, a1, a2, *a3, a4, a5);
}
void ImFont_RenderText_cwrap(struct ImFont* a0, struct ImDrawList* a1, float a2, struct ImVec2* a3, unsigned int a4, struct ImVec4* a5, char* a6, char* a7, float a8, bool a9){
  return ImFont_RenderText(a0, a1, a2, *a3, a4, *a5, a6, a7, a8, a9);
}
void igImMin_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2){
  return igImMin(a0, *a1, *a2);
}
void igImMax_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2){
  return igImMax(a0, *a1, *a2);
}
void igImClamp_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3){
  return igImClamp(a0, *a1, *a2, *a3);
}
void igImLerp_Vec2Float_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, float a3){
  return igImLerp_Vec2Float(a0, *a1, *a2, a3);
}
void igImLerp_Vec2Vec2_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3){
  return igImLerp_Vec2Vec2(a0, *a1, *a2, *a3);
}
void igImLerp_Vec4_cwrap(struct ImVec4* a0, struct ImVec4* a1, struct ImVec4* a2, float a3){
  return igImLerp_Vec4(a0, *a1, *a2, a3);
}
float igImLengthSqr_Vec2_cwrap(struct ImVec2* a0){
  return igImLengthSqr_Vec2(*a0);
}
float igImLengthSqr_Vec4_cwrap(struct ImVec4* a0){
  return igImLengthSqr_Vec4(*a0);
}
float igImInvLength_cwrap(struct ImVec2* a0, float a1){
  return igImInvLength(*a0, a1);
}
void igImFloor_Vec2_cwrap(struct ImVec2* a0, struct ImVec2* a1){
  return igImFloor_Vec2(a0, *a1);
}
void igImFloorSigned_Vec2_cwrap(struct ImVec2* a0, struct ImVec2* a1){
  return igImFloorSigned_Vec2(a0, *a1);
}
float igImDot_cwrap(struct ImVec2* a0, struct ImVec2* a1){
  return igImDot(*a0, *a1);
}
void igImRotate_cwrap(struct ImVec2* a0, struct ImVec2* a1, float a2, float a3){
  return igImRotate(a0, *a1, a2, a3);
}
void igImMul_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2){
  return igImMul(a0, *a1, *a2);
}
void igImBezierCubicCalc_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, float a5){
  return igImBezierCubicCalc(a0, *a1, *a2, *a3, *a4, a5);
}
void igImBezierCubicClosestPoint_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, struct ImVec2* a5, int a6){
  return igImBezierCubicClosestPoint(a0, *a1, *a2, *a3, *a4, *a5, a6);
}
void igImBezierCubicClosestPointCasteljau_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, struct ImVec2* a5, float a6){
  return igImBezierCubicClosestPointCasteljau(a0, *a1, *a2, *a3, *a4, *a5, a6);
}
void igImBezierQuadraticCalc_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, float a4){
  return igImBezierQuadraticCalc(a0, *a1, *a2, *a3, a4);
}
void igImLineClosestPoint_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3){
  return igImLineClosestPoint(a0, *a1, *a2, *a3);
}
bool igImTriangleContainsPoint_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3){
  return igImTriangleContainsPoint(*a0, *a1, *a2, *a3);
}
void igImTriangleClosestPoint_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4){
  return igImTriangleClosestPoint(a0, *a1, *a2, *a3, *a4);
}
void igImTriangleBarycentricCoords_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, struct ImVec2* a3, float* a4, float* a5, float* a6){
  return igImTriangleBarycentricCoords(*a0, *a1, *a2, *a3, a4, a5, a6);
}
float igImTriangleArea_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2){
  return igImTriangleArea(*a0, *a1, *a2);
}
struct ImVec2ih* ImVec2ih_ImVec2ih_Vec2_cwrap(struct ImVec2* a0){
  return ImVec2ih_ImVec2ih_Vec2(*a0);
}
struct ImRect* ImRect_ImRect_Vec2_cwrap(struct ImVec2* a0, struct ImVec2* a1){
  return ImRect_ImRect_Vec2(*a0, *a1);
}
struct ImRect* ImRect_ImRect_Vec4_cwrap(struct ImVec4* a0){
  return ImRect_ImRect_Vec4(*a0);
}
bool ImRect_Contains_Vec2_cwrap(struct ImRect* a0, struct ImVec2* a1){
  return ImRect_Contains_Vec2(a0, *a1);
}
bool ImRect_Contains_Rect_cwrap(struct ImRect* a0, struct ImRect* a1){
  return ImRect_Contains_Rect(a0, *a1);
}
bool ImRect_Overlaps_cwrap(struct ImRect* a0, struct ImRect* a1){
  return ImRect_Overlaps(a0, *a1);
}
void ImRect_Add_Vec2_cwrap(struct ImRect* a0, struct ImVec2* a1){
  return ImRect_Add_Vec2(a0, *a1);
}
void ImRect_Add_Rect_cwrap(struct ImRect* a0, struct ImRect* a1){
  return ImRect_Add_Rect(a0, *a1);
}
void ImRect_Expand_Vec2_cwrap(struct ImRect* a0, struct ImVec2* a1){
  return ImRect_Expand_Vec2(a0, *a1);
}
void ImRect_Translate_cwrap(struct ImRect* a0, struct ImVec2* a1){
  return ImRect_Translate(a0, *a1);
}
void ImRect_ClipWith_cwrap(struct ImRect* a0, struct ImRect* a1){
  return ImRect_ClipWith(a0, *a1);
}
void ImRect_ClipWithFull_cwrap(struct ImRect* a0, struct ImRect* a1){
  return ImRect_ClipWithFull(a0, *a1);
}
struct ImGuiStyleMod* ImGuiStyleMod_ImGuiStyleMod_Vec2_cwrap(int a0, struct ImVec2* a1){
  return ImGuiStyleMod_ImGuiStyleMod_Vec2(a0, *a1);
}
void ImGuiListClipperRange_FromIndices_cwrap(struct ImGuiListClipperRange* a0, int a1, int a2){
  *a0 = ImGuiListClipperRange_FromIndices(a1, a2);
}
void ImGuiListClipperRange_FromPositions_cwrap(struct ImGuiListClipperRange* a0, float a1, float a2, int a3, int a4){
  *a0 = ImGuiListClipperRange_FromPositions(a1, a2, a3, a4);
}
void ImGuiViewportP_CalcWorkRectPos_cwrap(struct ImVec2* a0, struct ImGuiViewportP* a1, struct ImVec2* a2){
  return ImGuiViewportP_CalcWorkRectPos(a0, a1, *a2);
}
void ImGuiViewportP_CalcWorkRectSize_cwrap(struct ImVec2* a0, struct ImGuiViewportP* a1, struct ImVec2* a2, struct ImVec2* a3){
  return ImGuiViewportP_CalcWorkRectSize(a0, a1, *a2, *a3);
}
unsigned int ImGuiWindow_GetIDFromRectangle_cwrap(struct ImGuiWindow* a0, struct ImRect* a1){
  return ImGuiWindow_GetIDFromRectangle(a0, *a1);
}
void igSetWindowPos_WindowPtr_cwrap(struct ImGuiWindow* a0, struct ImVec2* a1, int a2){
  return igSetWindowPos_WindowPtr(a0, *a1, a2);
}
void igSetWindowSize_WindowPtr_cwrap(struct ImGuiWindow* a0, struct ImVec2* a1, int a2){
  return igSetWindowSize_WindowPtr(a0, *a1, a2);
}
void igSetWindowHitTestHole_cwrap(struct ImGuiWindow* a0, struct ImVec2* a1, struct ImVec2* a2){
  return igSetWindowHitTestHole(a0, *a1, *a2);
}
void igWindowRectAbsToRel_cwrap(struct ImRect* a0, struct ImGuiWindow* a1, struct ImRect* a2){
  return igWindowRectAbsToRel(a0, a1, *a2);
}
void igWindowRectRelToAbs_cwrap(struct ImRect* a0, struct ImGuiWindow* a1, struct ImRect* a2){
  return igWindowRectRelToAbs(a0, a1, *a2);
}
void igWindowPosRelToAbs_cwrap(struct ImVec2* a0, struct ImGuiWindow* a1, struct ImVec2* a2){
  return igWindowPosRelToAbs(a0, a1, *a2);
}
void igScrollToRect_cwrap(struct ImGuiWindow* a0, struct ImRect* a1, int a2){
  return igScrollToRect(a0, *a1, a2);
}
void igScrollToRectEx_cwrap(struct ImVec2* a0, struct ImGuiWindow* a1, struct ImRect* a2, int a3){
  return igScrollToRectEx(a0, a1, *a2, a3);
}
void igScrollToBringRectIntoView_cwrap(struct ImGuiWindow* a0, struct ImRect* a1){
  return igScrollToBringRectIntoView(a0, *a1);
}
void igItemSize_Vec2_cwrap(struct ImVec2* a0, float a1){
  return igItemSize_Vec2(*a0, a1);
}
void igItemSize_Rect_cwrap(struct ImRect* a0, float a1){
  return igItemSize_Rect(*a0, a1);
}
bool igItemAdd_cwrap(struct ImRect* a0, unsigned int a1, struct ImRect* a2, int a3){
  return igItemAdd(*a0, a1, a2, a3);
}
bool igItemHoverable_cwrap(struct ImRect* a0, unsigned int a1, int a2){
  return igItemHoverable(*a0, a1, a2);
}
bool igIsClippedEx_cwrap(struct ImRect* a0, unsigned int a1){
  return igIsClippedEx(*a0, a1);
}
void igSetLastItemData_cwrap(unsigned int a0, int a1, int a2, struct ImRect* a3){
  return igSetLastItemData(a0, a1, a2, *a3);
}
void igCalcItemSize_cwrap(struct ImVec2* a0, struct ImVec2* a1, float a2, float a3){
  return igCalcItemSize(a0, *a1, a2, a3);
}
float igCalcWrapWidthForPos_cwrap(struct ImVec2* a0, float a1){
  return igCalcWrapWidthForPos(*a0, a1);
}
bool igBeginChildEx_cwrap(char* a0, unsigned int a1, struct ImVec2* a2, bool a3, int a4){
  return igBeginChildEx(a0, a1, *a2, a3, a4);
}
void igFindBestWindowPosForPopupEx_cwrap(struct ImVec2* a0, struct ImVec2* a1, struct ImVec2* a2, int* a3, struct ImRect* a4, struct ImRect* a5, ImGuiPopupPositionPolicy a6){
  return igFindBestWindowPosForPopupEx(a0, *a1, *a2, a3, *a4, *a5, a6);
}
bool igBeginComboPopup_cwrap(unsigned int a0, struct ImRect* a1, int a2){
  return igBeginComboPopup(a0, *a1, a2);
}
void igSetNavID_cwrap(unsigned int a0, ImGuiNavLayer a1, unsigned int a2, struct ImRect* a3){
  return igSetNavID(a0, a1, a2, *a3);
}
bool igBeginDragDropTargetCustom_cwrap(struct ImRect* a0, unsigned int a1){
  return igBeginDragDropTargetCustom(*a0, a1);
}
void igRenderDragDropTargetRect_cwrap(struct ImRect* a0){
  return igRenderDragDropTargetRect(*a0);
}
void igSetWindowClipRectBeforeSetChannel_cwrap(struct ImGuiWindow* a0, struct ImRect* a1){
  return igSetWindowClipRectBeforeSetChannel(a0, *a1);
}
bool igBeginTableEx_cwrap(char* a0, unsigned int a1, int a2, int a3, struct ImVec2* a4, float a5){
  return igBeginTableEx(a0, a1, a2, a3, *a4, a5);
}
bool igBeginTabBarEx_cwrap(struct ImGuiTabBar* a0, struct ImRect* a1, int a2){
  return igBeginTabBarEx(a0, *a1, a2);
}
void igTabBarQueueReorderFromMousePos_cwrap(struct ImGuiTabBar* a0, struct ImGuiTabItem* a1, struct ImVec2* a2){
  return igTabBarQueueReorderFromMousePos(a0, a1, *a2);
}
void igTabItemBackground_cwrap(struct ImDrawList* a0, struct ImRect* a1, int a2, unsigned int a3){
  return igTabItemBackground(a0, *a1, a2, a3);
}
void igTabItemLabelAndCloseButton_cwrap(struct ImDrawList* a0, struct ImRect* a1, int a2, struct ImVec2* a3, char* a4, unsigned int a5, unsigned int a6, bool a7, bool* a8, bool* a9){
  return igTabItemLabelAndCloseButton(a0, *a1, a2, *a3, a4, a5, a6, a7, a8, a9);
}
void igRenderText_cwrap(struct ImVec2* a0, char* a1, char* a2, bool a3){
  return igRenderText(*a0, a1, a2, a3);
}
void igRenderTextWrapped_cwrap(struct ImVec2* a0, char* a1, char* a2, float a3){
  return igRenderTextWrapped(*a0, a1, a2, a3);
}
void igRenderTextClipped_cwrap(struct ImVec2* a0, struct ImVec2* a1, char* a2, char* a3, struct ImVec2* a4, struct ImVec2* a5, struct ImRect* a6){
  return igRenderTextClipped(*a0, *a1, a2, a3, a4, *a5, a6);
}
void igRenderTextClippedEx_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, char* a3, char* a4, struct ImVec2* a5, struct ImVec2* a6, struct ImRect* a7){
  return igRenderTextClippedEx(a0, *a1, *a2, a3, a4, a5, *a6, a7);
}
void igRenderTextEllipsis_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, float a3, float a4, char* a5, char* a6, struct ImVec2* a7){
  return igRenderTextEllipsis(a0, *a1, *a2, a3, a4, a5, a6, a7);
}
void igRenderFrame_cwrap(struct ImVec2* a0, struct ImVec2* a1, unsigned int a2, bool a3, float a4){
  return igRenderFrame(*a0, *a1, a2, a3, a4);
}
void igRenderFrameBorder_cwrap(struct ImVec2* a0, struct ImVec2* a1, float a2){
  return igRenderFrameBorder(*a0, *a1, a2);
}
void igRenderColorRectWithAlphaCheckerboard_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, unsigned int a3, float a4, struct ImVec2* a5, float a6, int a7){
  return igRenderColorRectWithAlphaCheckerboard(a0, *a1, *a2, a3, a4, *a5, a6, a7);
}
void igRenderNavHighlight_cwrap(struct ImRect* a0, unsigned int a1, int a2){
  return igRenderNavHighlight(*a0, a1, a2);
}
void igRenderMouseCursor_cwrap(struct ImVec2* a0, float a1, int a2, unsigned int a3, unsigned int a4, unsigned int a5){
  return igRenderMouseCursor(*a0, a1, a2, a3, a4, a5);
}
void igRenderArrow_cwrap(struct ImDrawList* a0, struct ImVec2* a1, unsigned int a2, int a3, float a4){
  return igRenderArrow(a0, *a1, a2, a3, a4);
}
void igRenderBullet_cwrap(struct ImDrawList* a0, struct ImVec2* a1, unsigned int a2){
  return igRenderBullet(a0, *a1, a2);
}
void igRenderCheckMark_cwrap(struct ImDrawList* a0, struct ImVec2* a1, unsigned int a2, float a3){
  return igRenderCheckMark(a0, *a1, a2, a3);
}
void igRenderArrowPointingAt_cwrap(struct ImDrawList* a0, struct ImVec2* a1, struct ImVec2* a2, int a3, unsigned int a4){
  return igRenderArrowPointingAt(a0, *a1, *a2, a3, a4);
}
void igRenderRectFilledRangeH_cwrap(struct ImDrawList* a0, struct ImRect* a1, unsigned int a2, float a3, float a4, float a5){
  return igRenderRectFilledRangeH(a0, *a1, a2, a3, a4, a5);
}
void igRenderRectFilledWithHole_cwrap(struct ImDrawList* a0, struct ImRect* a1, struct ImRect* a2, unsigned int a3, float a4){
  return igRenderRectFilledWithHole(a0, *a1, *a2, a3, a4);
}
bool igButtonEx_cwrap(char* a0, struct ImVec2* a1, int a2){
  return igButtonEx(a0, *a1, a2);
}
bool igArrowButtonEx_cwrap(char* a0, int a1, struct ImVec2* a2, int a3){
  return igArrowButtonEx(a0, a1, *a2, a3);
}
bool igImageButtonEx_cwrap(unsigned int a0, void* a1, struct ImVec2* a2, struct ImVec2* a3, struct ImVec2* a4, struct ImVec4* a5, struct ImVec4* a6, int a7){
  return igImageButtonEx(a0, a1, *a2, *a3, *a4, *a5, *a6, a7);
}
bool igCloseButton_cwrap(unsigned int a0, struct ImVec2* a1){
  return igCloseButton(a0, *a1);
}
bool igCollapseButton_cwrap(unsigned int a0, struct ImVec2* a1){
  return igCollapseButton(a0, *a1);
}
bool igScrollbarEx_cwrap(struct ImRect* a0, unsigned int a1, ImGuiAxis a2, long long int* a3, long long int a4, long long int a5, int a6){
  return igScrollbarEx(*a0, a1, a2, a3, a4, a5, a6);
}
bool igButtonBehavior_cwrap(struct ImRect* a0, unsigned int a1, bool* a2, bool* a3, int a4){
  return igButtonBehavior(*a0, a1, a2, a3, a4);
}
bool igSliderBehavior_cwrap(struct ImRect* a0, unsigned int a1, int a2, void* a3, void* a4, void* a5, char* a6, int a7, struct ImRect* a8){
  return igSliderBehavior(*a0, a1, a2, a3, a4, a5, a6, a7, a8);
}
bool igSplitterBehavior_cwrap(struct ImRect* a0, unsigned int a1, ImGuiAxis a2, float* a3, float* a4, float a5, float a6, float a7, float a8, unsigned int a9){
  return igSplitterBehavior(*a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
bool igInputTextEx_cwrap(char* a0, char* a1, char* a2, int a3, struct ImVec2* a4, int a5, void* a6, void* a7){
  return igInputTextEx(a0, a1, a2, a3, *a4, a5, a6, a7);
}
bool igTempInputText_cwrap(struct ImRect* a0, unsigned int a1, char* a2, char* a3, int a4, int a5){
  return igTempInputText(*a0, a1, a2, a3, a4, a5);
}
bool igTempInputScalar_cwrap(struct ImRect* a0, unsigned int a1, char* a2, int a3, void* a4, char* a5, void* a6, void* a7){
  return igTempInputScalar(*a0, a1, a2, a3, a4, a5, a6, a7);
}
int igPlotEx_cwrap(ImGuiPlotType a0, char* a1, void* a2, void* a3, int a4, int a5, char* a6, float a7, float a8, struct ImVec2* a9){
  return igPlotEx(a0, a1, a2, a3, a4, a5, a6, a7, a8, *a9);
}
void igShadeVertsLinearColorGradientKeepAlpha_cwrap(struct ImDrawList* a0, int a1, int a2, struct ImVec2* a3, struct ImVec2* a4, unsigned int a5, unsigned int a6){
  return igShadeVertsLinearColorGradientKeepAlpha(a0, a1, a2, *a3, *a4, a5, a6);
}
void igShadeVertsLinearUV_cwrap(struct ImDrawList* a0, int a1, int a2, struct ImVec2* a3, struct ImVec2* a4, struct ImVec2* a5, struct ImVec2* a6, bool a7){
  return igShadeVertsLinearUV(a0, a1, a2, *a3, *a4, *a5, *a6, a7);
}
void igDebugRenderViewportThumbnail_cwrap(struct ImDrawList* a0, struct ImGuiViewportP* a1, struct ImRect* a2){
  return igDebugRenderViewportThumbnail(a0, a1, *a2);
}
void simgui_make_image_cwrap(struct simgui_image_t* a0, struct simgui_image_desc_t* a1){
  *a0 = simgui_make_image(a1);
}
void simgui_destroy_image_cwrap(struct simgui_image_t* a0){
  return simgui_destroy_image(*a0);
}
void simgui_query_image_desc_cwrap(struct simgui_image_desc_t* a0, struct simgui_image_t* a1){
  *a0 = simgui_query_image_desc(*a1);
}
void* simgui_imtextureid_cwrap(struct simgui_image_t* a0){
  return simgui_imtextureid(*a0);
}
void simgui_image_from_imtextureid_cwrap(struct simgui_image_t* a0, void* a1){
  *a0 = simgui_image_from_imtextureid(a1);
}
