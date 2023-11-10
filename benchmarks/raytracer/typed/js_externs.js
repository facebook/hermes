const _ImVec2_ImVec2_Nil = $SHBuiltin.extern_c({}, function ImVec2_ImVec2_Nil(): c_ptr { throw 0; });
const _ImVec2_destroy = $SHBuiltin.extern_c({}, function ImVec2_destroy(_self: c_ptr): void { throw 0; });
const _ImVec2_ImVec2_Float = $SHBuiltin.extern_c({}, function ImVec2_ImVec2_Float(__x: c_float, __y: c_float): c_ptr { throw 0; });
const _ImVec4_ImVec4_Nil = $SHBuiltin.extern_c({}, function ImVec4_ImVec4_Nil(): c_ptr { throw 0; });
const _ImVec4_destroy = $SHBuiltin.extern_c({}, function ImVec4_destroy(_self: c_ptr): void { throw 0; });
const _ImVec4_ImVec4_Float = $SHBuiltin.extern_c({}, function ImVec4_ImVec4_Float(__x: c_float, __y: c_float, __z: c_float, __w: c_float): c_ptr { throw 0; });
const _igCreateContext = $SHBuiltin.extern_c({}, function igCreateContext(_shared_font_atlas: c_ptr): c_ptr { throw 0; });
const _igDestroyContext = $SHBuiltin.extern_c({}, function igDestroyContext(_ctx: c_ptr): void { throw 0; });
const _igGetCurrentContext = $SHBuiltin.extern_c({}, function igGetCurrentContext(): c_ptr { throw 0; });
const _igSetCurrentContext = $SHBuiltin.extern_c({}, function igSetCurrentContext(_ctx: c_ptr): void { throw 0; });
const _igGetIO = $SHBuiltin.extern_c({}, function igGetIO(): c_ptr { throw 0; });
const _igGetStyle = $SHBuiltin.extern_c({}, function igGetStyle(): c_ptr { throw 0; });
const _igNewFrame = $SHBuiltin.extern_c({}, function igNewFrame(): void { throw 0; });
const _igEndFrame = $SHBuiltin.extern_c({}, function igEndFrame(): void { throw 0; });
const _igRender = $SHBuiltin.extern_c({}, function igRender(): void { throw 0; });
const _igGetDrawData = $SHBuiltin.extern_c({}, function igGetDrawData(): c_ptr { throw 0; });
const _igShowDemoWindow = $SHBuiltin.extern_c({}, function igShowDemoWindow(_p_open: c_ptr): void { throw 0; });
const _igShowMetricsWindow = $SHBuiltin.extern_c({}, function igShowMetricsWindow(_p_open: c_ptr): void { throw 0; });
const _igShowDebugLogWindow = $SHBuiltin.extern_c({}, function igShowDebugLogWindow(_p_open: c_ptr): void { throw 0; });
const _igShowStackToolWindow = $SHBuiltin.extern_c({}, function igShowStackToolWindow(_p_open: c_ptr): void { throw 0; });
const _igShowAboutWindow = $SHBuiltin.extern_c({}, function igShowAboutWindow(_p_open: c_ptr): void { throw 0; });
const _igShowStyleEditor = $SHBuiltin.extern_c({}, function igShowStyleEditor(_ref: c_ptr): void { throw 0; });
const _igShowStyleSelector = $SHBuiltin.extern_c({}, function igShowStyleSelector(_label: c_ptr): c_bool { throw 0; });
const _igShowFontSelector = $SHBuiltin.extern_c({}, function igShowFontSelector(_label: c_ptr): void { throw 0; });
const _igShowUserGuide = $SHBuiltin.extern_c({}, function igShowUserGuide(): void { throw 0; });
const _igGetVersion = $SHBuiltin.extern_c({}, function igGetVersion(): c_ptr { throw 0; });
const _igStyleColorsDark = $SHBuiltin.extern_c({}, function igStyleColorsDark(_dst: c_ptr): void { throw 0; });
const _igStyleColorsLight = $SHBuiltin.extern_c({}, function igStyleColorsLight(_dst: c_ptr): void { throw 0; });
const _igStyleColorsClassic = $SHBuiltin.extern_c({}, function igStyleColorsClassic(_dst: c_ptr): void { throw 0; });
const _igBegin = $SHBuiltin.extern_c({}, function igBegin(_name: c_ptr, _p_open: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igEnd = $SHBuiltin.extern_c({}, function igEnd(): void { throw 0; });
const _igBeginChild_Str = $SHBuiltin.extern_c({}, function igBeginChild_Str_cwrap(_str_id: c_ptr, _size: c_ptr, _border: c_bool, _flags: c_int): c_bool { throw 0; });
const _igBeginChild_ID = $SHBuiltin.extern_c({}, function igBeginChild_ID_cwrap(_id: c_uint, _size: c_ptr, _border: c_bool, _flags: c_int): c_bool { throw 0; });
const _igEndChild = $SHBuiltin.extern_c({}, function igEndChild(): void { throw 0; });
const _igIsWindowAppearing = $SHBuiltin.extern_c({}, function igIsWindowAppearing(): c_bool { throw 0; });
const _igIsWindowCollapsed = $SHBuiltin.extern_c({}, function igIsWindowCollapsed(): c_bool { throw 0; });
const _igIsWindowFocused = $SHBuiltin.extern_c({}, function igIsWindowFocused(_flags: c_int): c_bool { throw 0; });
const _igIsWindowHovered = $SHBuiltin.extern_c({}, function igIsWindowHovered(_flags: c_int): c_bool { throw 0; });
const _igGetWindowDrawList = $SHBuiltin.extern_c({}, function igGetWindowDrawList(): c_ptr { throw 0; });
const _igGetWindowPos = $SHBuiltin.extern_c({}, function igGetWindowPos(_pOut: c_ptr): void { throw 0; });
const _igGetWindowSize = $SHBuiltin.extern_c({}, function igGetWindowSize(_pOut: c_ptr): void { throw 0; });
const _igGetWindowWidth = $SHBuiltin.extern_c({}, function igGetWindowWidth(): c_float { throw 0; });
const _igGetWindowHeight = $SHBuiltin.extern_c({}, function igGetWindowHeight(): c_float { throw 0; });
const _igSetNextWindowPos = $SHBuiltin.extern_c({}, function igSetNextWindowPos_cwrap(_pos: c_ptr, _cond: c_int, _pivot: c_ptr): void { throw 0; });
const _igSetNextWindowSize = $SHBuiltin.extern_c({}, function igSetNextWindowSize_cwrap(_size: c_ptr, _cond: c_int): void { throw 0; });
const _igSetNextWindowSizeConstraints = $SHBuiltin.extern_c({}, function igSetNextWindowSizeConstraints_cwrap(_size_min: c_ptr, _size_max: c_ptr, _custom_callback: c_ptr, _custom_callback_data: c_ptr): void { throw 0; });
const _igSetNextWindowContentSize = $SHBuiltin.extern_c({}, function igSetNextWindowContentSize_cwrap(_size: c_ptr): void { throw 0; });
const _igSetNextWindowCollapsed = $SHBuiltin.extern_c({}, function igSetNextWindowCollapsed(_collapsed: c_bool, _cond: c_int): void { throw 0; });
const _igSetNextWindowFocus = $SHBuiltin.extern_c({}, function igSetNextWindowFocus(): void { throw 0; });
const _igSetNextWindowScroll = $SHBuiltin.extern_c({}, function igSetNextWindowScroll_cwrap(_scroll: c_ptr): void { throw 0; });
const _igSetNextWindowBgAlpha = $SHBuiltin.extern_c({}, function igSetNextWindowBgAlpha(_alpha: c_float): void { throw 0; });
const _igSetWindowPos_Vec2 = $SHBuiltin.extern_c({}, function igSetWindowPos_Vec2_cwrap(_pos: c_ptr, _cond: c_int): void { throw 0; });
const _igSetWindowSize_Vec2 = $SHBuiltin.extern_c({}, function igSetWindowSize_Vec2_cwrap(_size: c_ptr, _cond: c_int): void { throw 0; });
const _igSetWindowCollapsed_Bool = $SHBuiltin.extern_c({}, function igSetWindowCollapsed_Bool(_collapsed: c_bool, _cond: c_int): void { throw 0; });
const _igSetWindowFocus_Nil = $SHBuiltin.extern_c({}, function igSetWindowFocus_Nil(): void { throw 0; });
const _igSetWindowFontScale = $SHBuiltin.extern_c({}, function igSetWindowFontScale(_scale: c_float): void { throw 0; });
const _igSetWindowPos_Str = $SHBuiltin.extern_c({}, function igSetWindowPos_Str_cwrap(_name: c_ptr, _pos: c_ptr, _cond: c_int): void { throw 0; });
const _igSetWindowSize_Str = $SHBuiltin.extern_c({}, function igSetWindowSize_Str_cwrap(_name: c_ptr, _size: c_ptr, _cond: c_int): void { throw 0; });
const _igSetWindowCollapsed_Str = $SHBuiltin.extern_c({}, function igSetWindowCollapsed_Str(_name: c_ptr, _collapsed: c_bool, _cond: c_int): void { throw 0; });
const _igSetWindowFocus_Str = $SHBuiltin.extern_c({}, function igSetWindowFocus_Str(_name: c_ptr): void { throw 0; });
const _igGetContentRegionAvail = $SHBuiltin.extern_c({}, function igGetContentRegionAvail(_pOut: c_ptr): void { throw 0; });
const _igGetContentRegionMax = $SHBuiltin.extern_c({}, function igGetContentRegionMax(_pOut: c_ptr): void { throw 0; });
const _igGetWindowContentRegionMin = $SHBuiltin.extern_c({}, function igGetWindowContentRegionMin(_pOut: c_ptr): void { throw 0; });
const _igGetWindowContentRegionMax = $SHBuiltin.extern_c({}, function igGetWindowContentRegionMax(_pOut: c_ptr): void { throw 0; });
const _igGetScrollX = $SHBuiltin.extern_c({}, function igGetScrollX(): c_float { throw 0; });
const _igGetScrollY = $SHBuiltin.extern_c({}, function igGetScrollY(): c_float { throw 0; });
const _igSetScrollX_Float = $SHBuiltin.extern_c({}, function igSetScrollX_Float(_scroll_x: c_float): void { throw 0; });
const _igSetScrollY_Float = $SHBuiltin.extern_c({}, function igSetScrollY_Float(_scroll_y: c_float): void { throw 0; });
const _igGetScrollMaxX = $SHBuiltin.extern_c({}, function igGetScrollMaxX(): c_float { throw 0; });
const _igGetScrollMaxY = $SHBuiltin.extern_c({}, function igGetScrollMaxY(): c_float { throw 0; });
const _igSetScrollHereX = $SHBuiltin.extern_c({}, function igSetScrollHereX(_center_x_ratio: c_float): void { throw 0; });
const _igSetScrollHereY = $SHBuiltin.extern_c({}, function igSetScrollHereY(_center_y_ratio: c_float): void { throw 0; });
const _igSetScrollFromPosX_Float = $SHBuiltin.extern_c({}, function igSetScrollFromPosX_Float(_local_x: c_float, _center_x_ratio: c_float): void { throw 0; });
const _igSetScrollFromPosY_Float = $SHBuiltin.extern_c({}, function igSetScrollFromPosY_Float(_local_y: c_float, _center_y_ratio: c_float): void { throw 0; });
const _igPushFont = $SHBuiltin.extern_c({}, function igPushFont(_font: c_ptr): void { throw 0; });
const _igPopFont = $SHBuiltin.extern_c({}, function igPopFont(): void { throw 0; });
const _igPushStyleColor_U32 = $SHBuiltin.extern_c({}, function igPushStyleColor_U32(_idx: c_int, _col: c_uint): void { throw 0; });
const _igPushStyleColor_Vec4 = $SHBuiltin.extern_c({}, function igPushStyleColor_Vec4_cwrap(_idx: c_int, _col: c_ptr): void { throw 0; });
const _igPopStyleColor = $SHBuiltin.extern_c({}, function igPopStyleColor(_count: c_int): void { throw 0; });
const _igPushStyleVar_Float = $SHBuiltin.extern_c({}, function igPushStyleVar_Float(_idx: c_int, _val: c_float): void { throw 0; });
const _igPushStyleVar_Vec2 = $SHBuiltin.extern_c({}, function igPushStyleVar_Vec2_cwrap(_idx: c_int, _val: c_ptr): void { throw 0; });
const _igPopStyleVar = $SHBuiltin.extern_c({}, function igPopStyleVar(_count: c_int): void { throw 0; });
const _igPushTabStop = $SHBuiltin.extern_c({}, function igPushTabStop(_tab_stop: c_bool): void { throw 0; });
const _igPopTabStop = $SHBuiltin.extern_c({}, function igPopTabStop(): void { throw 0; });
const _igPushButtonRepeat = $SHBuiltin.extern_c({}, function igPushButtonRepeat(_repeat: c_bool): void { throw 0; });
const _igPopButtonRepeat = $SHBuiltin.extern_c({}, function igPopButtonRepeat(): void { throw 0; });
const _igPushItemWidth = $SHBuiltin.extern_c({}, function igPushItemWidth(_item_width: c_float): void { throw 0; });
const _igPopItemWidth = $SHBuiltin.extern_c({}, function igPopItemWidth(): void { throw 0; });
const _igSetNextItemWidth = $SHBuiltin.extern_c({}, function igSetNextItemWidth(_item_width: c_float): void { throw 0; });
const _igCalcItemWidth = $SHBuiltin.extern_c({}, function igCalcItemWidth(): c_float { throw 0; });
const _igPushTextWrapPos = $SHBuiltin.extern_c({}, function igPushTextWrapPos(_wrap_local_pos_x: c_float): void { throw 0; });
const _igPopTextWrapPos = $SHBuiltin.extern_c({}, function igPopTextWrapPos(): void { throw 0; });
const _igGetFont = $SHBuiltin.extern_c({}, function igGetFont(): c_ptr { throw 0; });
const _igGetFontSize = $SHBuiltin.extern_c({}, function igGetFontSize(): c_float { throw 0; });
const _igGetFontTexUvWhitePixel = $SHBuiltin.extern_c({}, function igGetFontTexUvWhitePixel(_pOut: c_ptr): void { throw 0; });
const _igGetColorU32_Col = $SHBuiltin.extern_c({}, function igGetColorU32_Col(_idx: c_int, _alpha_mul: c_float): c_uint { throw 0; });
const _igGetColorU32_Vec4 = $SHBuiltin.extern_c({}, function igGetColorU32_Vec4_cwrap(_col: c_ptr): c_uint { throw 0; });
const _igGetColorU32_U32 = $SHBuiltin.extern_c({}, function igGetColorU32_U32(_col: c_uint): c_uint { throw 0; });
const _igGetStyleColorVec4 = $SHBuiltin.extern_c({}, function igGetStyleColorVec4(_idx: c_int): c_ptr { throw 0; });
const _igSeparator = $SHBuiltin.extern_c({}, function igSeparator(): void { throw 0; });
const _igSameLine = $SHBuiltin.extern_c({}, function igSameLine(_offset_from_start_x: c_float, _spacing: c_float): void { throw 0; });
const _igNewLine = $SHBuiltin.extern_c({}, function igNewLine(): void { throw 0; });
const _igSpacing = $SHBuiltin.extern_c({}, function igSpacing(): void { throw 0; });
const _igDummy = $SHBuiltin.extern_c({}, function igDummy_cwrap(_size: c_ptr): void { throw 0; });
const _igIndent = $SHBuiltin.extern_c({}, function igIndent(_indent_w: c_float): void { throw 0; });
const _igUnindent = $SHBuiltin.extern_c({}, function igUnindent(_indent_w: c_float): void { throw 0; });
const _igBeginGroup = $SHBuiltin.extern_c({}, function igBeginGroup(): void { throw 0; });
const _igEndGroup = $SHBuiltin.extern_c({}, function igEndGroup(): void { throw 0; });
const _igGetCursorPos = $SHBuiltin.extern_c({}, function igGetCursorPos(_pOut: c_ptr): void { throw 0; });
const _igGetCursorPosX = $SHBuiltin.extern_c({}, function igGetCursorPosX(): c_float { throw 0; });
const _igGetCursorPosY = $SHBuiltin.extern_c({}, function igGetCursorPosY(): c_float { throw 0; });
const _igSetCursorPos = $SHBuiltin.extern_c({}, function igSetCursorPos_cwrap(_local_pos: c_ptr): void { throw 0; });
const _igSetCursorPosX = $SHBuiltin.extern_c({}, function igSetCursorPosX(_local_x: c_float): void { throw 0; });
const _igSetCursorPosY = $SHBuiltin.extern_c({}, function igSetCursorPosY(_local_y: c_float): void { throw 0; });
const _igGetCursorStartPos = $SHBuiltin.extern_c({}, function igGetCursorStartPos(_pOut: c_ptr): void { throw 0; });
const _igGetCursorScreenPos = $SHBuiltin.extern_c({}, function igGetCursorScreenPos(_pOut: c_ptr): void { throw 0; });
const _igSetCursorScreenPos = $SHBuiltin.extern_c({}, function igSetCursorScreenPos_cwrap(_pos: c_ptr): void { throw 0; });
const _igAlignTextToFramePadding = $SHBuiltin.extern_c({}, function igAlignTextToFramePadding(): void { throw 0; });
const _igGetTextLineHeight = $SHBuiltin.extern_c({}, function igGetTextLineHeight(): c_float { throw 0; });
const _igGetTextLineHeightWithSpacing = $SHBuiltin.extern_c({}, function igGetTextLineHeightWithSpacing(): c_float { throw 0; });
const _igGetFrameHeight = $SHBuiltin.extern_c({}, function igGetFrameHeight(): c_float { throw 0; });
const _igGetFrameHeightWithSpacing = $SHBuiltin.extern_c({}, function igGetFrameHeightWithSpacing(): c_float { throw 0; });
const _igPushID_Str = $SHBuiltin.extern_c({}, function igPushID_Str(_str_id: c_ptr): void { throw 0; });
const _igPushID_StrStr = $SHBuiltin.extern_c({}, function igPushID_StrStr(_str_id_begin: c_ptr, _str_id_end: c_ptr): void { throw 0; });
const _igPushID_Ptr = $SHBuiltin.extern_c({}, function igPushID_Ptr(_ptr_id: c_ptr): void { throw 0; });
const _igPushID_Int = $SHBuiltin.extern_c({}, function igPushID_Int(_int_id: c_int): void { throw 0; });
const _igPopID = $SHBuiltin.extern_c({}, function igPopID(): void { throw 0; });
const _igGetID_Str = $SHBuiltin.extern_c({}, function igGetID_Str(_str_id: c_ptr): c_uint { throw 0; });
const _igGetID_StrStr = $SHBuiltin.extern_c({}, function igGetID_StrStr(_str_id_begin: c_ptr, _str_id_end: c_ptr): c_uint { throw 0; });
const _igGetID_Ptr = $SHBuiltin.extern_c({}, function igGetID_Ptr(_ptr_id: c_ptr): c_uint { throw 0; });
const _igTextUnformatted = $SHBuiltin.extern_c({}, function igTextUnformatted(_text: c_ptr, _text_end: c_ptr): void { throw 0; });
const _igText = $SHBuiltin.extern_c({}, function igText(_fmt: c_ptr): void { throw 0; });
const _igTextV = $SHBuiltin.extern_c({}, function igTextV(_fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igTextColored = $SHBuiltin.extern_c({}, function igTextColored_cwrap(_col: c_ptr, _fmt: c_ptr): void { throw 0; });
const _igTextColoredV = $SHBuiltin.extern_c({}, function igTextColoredV_cwrap(_col: c_ptr, _fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igTextDisabled = $SHBuiltin.extern_c({}, function igTextDisabled(_fmt: c_ptr): void { throw 0; });
const _igTextDisabledV = $SHBuiltin.extern_c({}, function igTextDisabledV(_fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igTextWrapped = $SHBuiltin.extern_c({}, function igTextWrapped(_fmt: c_ptr): void { throw 0; });
const _igTextWrappedV = $SHBuiltin.extern_c({}, function igTextWrappedV(_fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igLabelText = $SHBuiltin.extern_c({}, function igLabelText(_label: c_ptr, _fmt: c_ptr): void { throw 0; });
const _igLabelTextV = $SHBuiltin.extern_c({}, function igLabelTextV(_label: c_ptr, _fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igBulletText = $SHBuiltin.extern_c({}, function igBulletText(_fmt: c_ptr): void { throw 0; });
const _igBulletTextV = $SHBuiltin.extern_c({}, function igBulletTextV(_fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igSeparatorText = $SHBuiltin.extern_c({}, function igSeparatorText(_label: c_ptr): void { throw 0; });
const _igButton = $SHBuiltin.extern_c({}, function igButton_cwrap(_label: c_ptr, _size: c_ptr): c_bool { throw 0; });
const _igSmallButton = $SHBuiltin.extern_c({}, function igSmallButton(_label: c_ptr): c_bool { throw 0; });
const _igInvisibleButton = $SHBuiltin.extern_c({}, function igInvisibleButton_cwrap(_str_id: c_ptr, _size: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igArrowButton = $SHBuiltin.extern_c({}, function igArrowButton(_str_id: c_ptr, _dir: c_int): c_bool { throw 0; });
const _igCheckbox = $SHBuiltin.extern_c({}, function igCheckbox(_label: c_ptr, _v: c_ptr): c_bool { throw 0; });
const _igCheckboxFlags_IntPtr = $SHBuiltin.extern_c({}, function igCheckboxFlags_IntPtr(_label: c_ptr, _flags: c_ptr, _flags_value: c_int): c_bool { throw 0; });
const _igCheckboxFlags_UintPtr = $SHBuiltin.extern_c({}, function igCheckboxFlags_UintPtr(_label: c_ptr, _flags: c_ptr, _flags_value: c_uint): c_bool { throw 0; });
const _igRadioButton_Bool = $SHBuiltin.extern_c({}, function igRadioButton_Bool(_label: c_ptr, _active: c_bool): c_bool { throw 0; });
const _igRadioButton_IntPtr = $SHBuiltin.extern_c({}, function igRadioButton_IntPtr(_label: c_ptr, _v: c_ptr, _v_button: c_int): c_bool { throw 0; });
const _igProgressBar = $SHBuiltin.extern_c({}, function igProgressBar_cwrap(_fraction: c_float, _size_arg: c_ptr, _overlay: c_ptr): void { throw 0; });
const _igBullet = $SHBuiltin.extern_c({}, function igBullet(): void { throw 0; });
const _igImage = $SHBuiltin.extern_c({}, function igImage_cwrap(_user_texture_id: c_ptr, _size: c_ptr, _uv0: c_ptr, _uv1: c_ptr, _tint_col: c_ptr, _border_col: c_ptr): void { throw 0; });
const _igImageButton = $SHBuiltin.extern_c({}, function igImageButton_cwrap(_str_id: c_ptr, _user_texture_id: c_ptr, _size: c_ptr, _uv0: c_ptr, _uv1: c_ptr, _bg_col: c_ptr, _tint_col: c_ptr): c_bool { throw 0; });
const _igBeginCombo = $SHBuiltin.extern_c({}, function igBeginCombo(_label: c_ptr, _preview_value: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igEndCombo = $SHBuiltin.extern_c({}, function igEndCombo(): void { throw 0; });
const _igCombo_Str_arr = $SHBuiltin.extern_c({}, function igCombo_Str_arr(_label: c_ptr, _current_item: c_ptr, _items: c_ptr, _items_count: c_int, _popup_max_height_in_items: c_int): c_bool { throw 0; });
const _igCombo_Str = $SHBuiltin.extern_c({}, function igCombo_Str(_label: c_ptr, _current_item: c_ptr, _items_separated_by_zeros: c_ptr, _popup_max_height_in_items: c_int): c_bool { throw 0; });
const _igCombo_FnBoolPtr = $SHBuiltin.extern_c({}, function igCombo_FnBoolPtr(_label: c_ptr, _current_item: c_ptr, _items_getter: c_ptr, _data: c_ptr, _items_count: c_int, _popup_max_height_in_items: c_int): c_bool { throw 0; });
const _igDragFloat = $SHBuiltin.extern_c({}, function igDragFloat(_label: c_ptr, _v: c_ptr, _v_speed: c_float, _v_min: c_float, _v_max: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragFloat2 = $SHBuiltin.extern_c({}, function igDragFloat2(_label: c_ptr, _v: c_ptr, _v_speed: c_float, _v_min: c_float, _v_max: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragFloat3 = $SHBuiltin.extern_c({}, function igDragFloat3(_label: c_ptr, _v: c_ptr, _v_speed: c_float, _v_min: c_float, _v_max: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragFloat4 = $SHBuiltin.extern_c({}, function igDragFloat4(_label: c_ptr, _v: c_ptr, _v_speed: c_float, _v_min: c_float, _v_max: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragFloatRange2 = $SHBuiltin.extern_c({}, function igDragFloatRange2(_label: c_ptr, _v_current_min: c_ptr, _v_current_max: c_ptr, _v_speed: c_float, _v_min: c_float, _v_max: c_float, _format: c_ptr, _format_max: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragInt = $SHBuiltin.extern_c({}, function igDragInt(_label: c_ptr, _v: c_ptr, _v_speed: c_float, _v_min: c_int, _v_max: c_int, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragInt2 = $SHBuiltin.extern_c({}, function igDragInt2(_label: c_ptr, _v: c_ptr, _v_speed: c_float, _v_min: c_int, _v_max: c_int, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragInt3 = $SHBuiltin.extern_c({}, function igDragInt3(_label: c_ptr, _v: c_ptr, _v_speed: c_float, _v_min: c_int, _v_max: c_int, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragInt4 = $SHBuiltin.extern_c({}, function igDragInt4(_label: c_ptr, _v: c_ptr, _v_speed: c_float, _v_min: c_int, _v_max: c_int, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragIntRange2 = $SHBuiltin.extern_c({}, function igDragIntRange2(_label: c_ptr, _v_current_min: c_ptr, _v_current_max: c_ptr, _v_speed: c_float, _v_min: c_int, _v_max: c_int, _format: c_ptr, _format_max: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragScalar = $SHBuiltin.extern_c({}, function igDragScalar(_label: c_ptr, _data_type: c_int, _p_data: c_ptr, _v_speed: c_float, _p_min: c_ptr, _p_max: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragScalarN = $SHBuiltin.extern_c({}, function igDragScalarN(_label: c_ptr, _data_type: c_int, _p_data: c_ptr, _components: c_int, _v_speed: c_float, _p_min: c_ptr, _p_max: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderFloat = $SHBuiltin.extern_c({}, function igSliderFloat(_label: c_ptr, _v: c_ptr, _v_min: c_float, _v_max: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderFloat2 = $SHBuiltin.extern_c({}, function igSliderFloat2(_label: c_ptr, _v: c_ptr, _v_min: c_float, _v_max: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderFloat3 = $SHBuiltin.extern_c({}, function igSliderFloat3(_label: c_ptr, _v: c_ptr, _v_min: c_float, _v_max: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderFloat4 = $SHBuiltin.extern_c({}, function igSliderFloat4(_label: c_ptr, _v: c_ptr, _v_min: c_float, _v_max: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderAngle = $SHBuiltin.extern_c({}, function igSliderAngle(_label: c_ptr, _v_rad: c_ptr, _v_degrees_min: c_float, _v_degrees_max: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderInt = $SHBuiltin.extern_c({}, function igSliderInt(_label: c_ptr, _v: c_ptr, _v_min: c_int, _v_max: c_int, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderInt2 = $SHBuiltin.extern_c({}, function igSliderInt2(_label: c_ptr, _v: c_ptr, _v_min: c_int, _v_max: c_int, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderInt3 = $SHBuiltin.extern_c({}, function igSliderInt3(_label: c_ptr, _v: c_ptr, _v_min: c_int, _v_max: c_int, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderInt4 = $SHBuiltin.extern_c({}, function igSliderInt4(_label: c_ptr, _v: c_ptr, _v_min: c_int, _v_max: c_int, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderScalar = $SHBuiltin.extern_c({}, function igSliderScalar(_label: c_ptr, _data_type: c_int, _p_data: c_ptr, _p_min: c_ptr, _p_max: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderScalarN = $SHBuiltin.extern_c({}, function igSliderScalarN(_label: c_ptr, _data_type: c_int, _p_data: c_ptr, _components: c_int, _p_min: c_ptr, _p_max: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igVSliderFloat = $SHBuiltin.extern_c({}, function igVSliderFloat_cwrap(_label: c_ptr, _size: c_ptr, _v: c_ptr, _v_min: c_float, _v_max: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igVSliderInt = $SHBuiltin.extern_c({}, function igVSliderInt_cwrap(_label: c_ptr, _size: c_ptr, _v: c_ptr, _v_min: c_int, _v_max: c_int, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igVSliderScalar = $SHBuiltin.extern_c({}, function igVSliderScalar_cwrap(_label: c_ptr, _size: c_ptr, _data_type: c_int, _p_data: c_ptr, _p_min: c_ptr, _p_max: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igInputText = $SHBuiltin.extern_c({}, function igInputText(_label: c_ptr, _buf: c_ptr, _buf_size: c_ulong, _flags: c_int, _callback: c_ptr, _user_data: c_ptr): c_bool { throw 0; });
const _igInputTextMultiline = $SHBuiltin.extern_c({}, function igInputTextMultiline_cwrap(_label: c_ptr, _buf: c_ptr, _buf_size: c_ulong, _size: c_ptr, _flags: c_int, _callback: c_ptr, _user_data: c_ptr): c_bool { throw 0; });
const _igInputTextWithHint = $SHBuiltin.extern_c({}, function igInputTextWithHint(_label: c_ptr, _hint: c_ptr, _buf: c_ptr, _buf_size: c_ulong, _flags: c_int, _callback: c_ptr, _user_data: c_ptr): c_bool { throw 0; });
const _igInputFloat = $SHBuiltin.extern_c({}, function igInputFloat(_label: c_ptr, _v: c_ptr, _step: c_float, _step_fast: c_float, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igInputFloat2 = $SHBuiltin.extern_c({}, function igInputFloat2(_label: c_ptr, _v: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igInputFloat3 = $SHBuiltin.extern_c({}, function igInputFloat3(_label: c_ptr, _v: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igInputFloat4 = $SHBuiltin.extern_c({}, function igInputFloat4(_label: c_ptr, _v: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igInputInt = $SHBuiltin.extern_c({}, function igInputInt(_label: c_ptr, _v: c_ptr, _step: c_int, _step_fast: c_int, _flags: c_int): c_bool { throw 0; });
const _igInputInt2 = $SHBuiltin.extern_c({}, function igInputInt2(_label: c_ptr, _v: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igInputInt3 = $SHBuiltin.extern_c({}, function igInputInt3(_label: c_ptr, _v: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igInputInt4 = $SHBuiltin.extern_c({}, function igInputInt4(_label: c_ptr, _v: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igInputDouble = $SHBuiltin.extern_c({}, function igInputDouble(_label: c_ptr, _v: c_ptr, _step: c_double, _step_fast: c_double, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igInputScalar = $SHBuiltin.extern_c({}, function igInputScalar(_label: c_ptr, _data_type: c_int, _p_data: c_ptr, _p_step: c_ptr, _p_step_fast: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igInputScalarN = $SHBuiltin.extern_c({}, function igInputScalarN(_label: c_ptr, _data_type: c_int, _p_data: c_ptr, _components: c_int, _p_step: c_ptr, _p_step_fast: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igColorEdit3 = $SHBuiltin.extern_c({}, function igColorEdit3(_label: c_ptr, _col: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igColorEdit4 = $SHBuiltin.extern_c({}, function igColorEdit4(_label: c_ptr, _col: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igColorPicker3 = $SHBuiltin.extern_c({}, function igColorPicker3(_label: c_ptr, _col: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igColorPicker4 = $SHBuiltin.extern_c({}, function igColorPicker4(_label: c_ptr, _col: c_ptr, _flags: c_int, _ref_col: c_ptr): c_bool { throw 0; });
const _igColorButton = $SHBuiltin.extern_c({}, function igColorButton_cwrap(_desc_id: c_ptr, _col: c_ptr, _flags: c_int, _size: c_ptr): c_bool { throw 0; });
const _igSetColorEditOptions = $SHBuiltin.extern_c({}, function igSetColorEditOptions(_flags: c_int): void { throw 0; });
const _igTreeNode_Str = $SHBuiltin.extern_c({}, function igTreeNode_Str(_label: c_ptr): c_bool { throw 0; });
const _igTreeNode_StrStr = $SHBuiltin.extern_c({}, function igTreeNode_StrStr(_str_id: c_ptr, _fmt: c_ptr): c_bool { throw 0; });
const _igTreeNode_Ptr = $SHBuiltin.extern_c({}, function igTreeNode_Ptr(_ptr_id: c_ptr, _fmt: c_ptr): c_bool { throw 0; });
const _igTreeNodeV_Str = $SHBuiltin.extern_c({}, function igTreeNodeV_Str(_str_id: c_ptr, _fmt: c_ptr, _args: c_ptr): c_bool { throw 0; });
const _igTreeNodeV_Ptr = $SHBuiltin.extern_c({}, function igTreeNodeV_Ptr(_ptr_id: c_ptr, _fmt: c_ptr, _args: c_ptr): c_bool { throw 0; });
const _igTreeNodeEx_Str = $SHBuiltin.extern_c({}, function igTreeNodeEx_Str(_label: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igTreeNodeEx_StrStr = $SHBuiltin.extern_c({}, function igTreeNodeEx_StrStr(_str_id: c_ptr, _flags: c_int, _fmt: c_ptr): c_bool { throw 0; });
const _igTreeNodeEx_Ptr = $SHBuiltin.extern_c({}, function igTreeNodeEx_Ptr(_ptr_id: c_ptr, _flags: c_int, _fmt: c_ptr): c_bool { throw 0; });
const _igTreeNodeExV_Str = $SHBuiltin.extern_c({}, function igTreeNodeExV_Str(_str_id: c_ptr, _flags: c_int, _fmt: c_ptr, _args: c_ptr): c_bool { throw 0; });
const _igTreeNodeExV_Ptr = $SHBuiltin.extern_c({}, function igTreeNodeExV_Ptr(_ptr_id: c_ptr, _flags: c_int, _fmt: c_ptr, _args: c_ptr): c_bool { throw 0; });
const _igTreePush_Str = $SHBuiltin.extern_c({}, function igTreePush_Str(_str_id: c_ptr): void { throw 0; });
const _igTreePush_Ptr = $SHBuiltin.extern_c({}, function igTreePush_Ptr(_ptr_id: c_ptr): void { throw 0; });
const _igTreePop = $SHBuiltin.extern_c({}, function igTreePop(): void { throw 0; });
const _igGetTreeNodeToLabelSpacing = $SHBuiltin.extern_c({}, function igGetTreeNodeToLabelSpacing(): c_float { throw 0; });
const _igCollapsingHeader_TreeNodeFlags = $SHBuiltin.extern_c({}, function igCollapsingHeader_TreeNodeFlags(_label: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igCollapsingHeader_BoolPtr = $SHBuiltin.extern_c({}, function igCollapsingHeader_BoolPtr(_label: c_ptr, _p_visible: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSetNextItemOpen = $SHBuiltin.extern_c({}, function igSetNextItemOpen(_is_open: c_bool, _cond: c_int): void { throw 0; });
const _igSelectable_Bool = $SHBuiltin.extern_c({}, function igSelectable_Bool_cwrap(_label: c_ptr, _selected: c_bool, _flags: c_int, _size: c_ptr): c_bool { throw 0; });
const _igSelectable_BoolPtr = $SHBuiltin.extern_c({}, function igSelectable_BoolPtr_cwrap(_label: c_ptr, _p_selected: c_ptr, _flags: c_int, _size: c_ptr): c_bool { throw 0; });
const _igBeginListBox = $SHBuiltin.extern_c({}, function igBeginListBox_cwrap(_label: c_ptr, _size: c_ptr): c_bool { throw 0; });
const _igEndListBox = $SHBuiltin.extern_c({}, function igEndListBox(): void { throw 0; });
const _igListBox_Str_arr = $SHBuiltin.extern_c({}, function igListBox_Str_arr(_label: c_ptr, _current_item: c_ptr, _items: c_ptr, _items_count: c_int, _height_in_items: c_int): c_bool { throw 0; });
const _igListBox_FnBoolPtr = $SHBuiltin.extern_c({}, function igListBox_FnBoolPtr(_label: c_ptr, _current_item: c_ptr, _items_getter: c_ptr, _data: c_ptr, _items_count: c_int, _height_in_items: c_int): c_bool { throw 0; });
const _igPlotLines_FloatPtr = $SHBuiltin.extern_c({}, function igPlotLines_FloatPtr_cwrap(_label: c_ptr, _values: c_ptr, _values_count: c_int, _values_offset: c_int, _overlay_text: c_ptr, _scale_min: c_float, _scale_max: c_float, _graph_size: c_ptr, _stride: c_int): void { throw 0; });
const _igPlotLines_FnFloatPtr = $SHBuiltin.extern_c({}, function igPlotLines_FnFloatPtr_cwrap(_label: c_ptr, _values_getter: c_ptr, _data: c_ptr, _values_count: c_int, _values_offset: c_int, _overlay_text: c_ptr, _scale_min: c_float, _scale_max: c_float, _graph_size: c_ptr): void { throw 0; });
const _igPlotHistogram_FloatPtr = $SHBuiltin.extern_c({}, function igPlotHistogram_FloatPtr_cwrap(_label: c_ptr, _values: c_ptr, _values_count: c_int, _values_offset: c_int, _overlay_text: c_ptr, _scale_min: c_float, _scale_max: c_float, _graph_size: c_ptr, _stride: c_int): void { throw 0; });
const _igPlotHistogram_FnFloatPtr = $SHBuiltin.extern_c({}, function igPlotHistogram_FnFloatPtr_cwrap(_label: c_ptr, _values_getter: c_ptr, _data: c_ptr, _values_count: c_int, _values_offset: c_int, _overlay_text: c_ptr, _scale_min: c_float, _scale_max: c_float, _graph_size: c_ptr): void { throw 0; });
const _igValue_Bool = $SHBuiltin.extern_c({}, function igValue_Bool(_prefix: c_ptr, _b: c_bool): void { throw 0; });
const _igValue_Int = $SHBuiltin.extern_c({}, function igValue_Int(_prefix: c_ptr, _v: c_int): void { throw 0; });
const _igValue_Uint = $SHBuiltin.extern_c({}, function igValue_Uint(_prefix: c_ptr, _v: c_uint): void { throw 0; });
const _igValue_Float = $SHBuiltin.extern_c({}, function igValue_Float(_prefix: c_ptr, _v: c_float, _float_format: c_ptr): void { throw 0; });
const _igBeginMenuBar = $SHBuiltin.extern_c({}, function igBeginMenuBar(): c_bool { throw 0; });
const _igEndMenuBar = $SHBuiltin.extern_c({}, function igEndMenuBar(): void { throw 0; });
const _igBeginMainMenuBar = $SHBuiltin.extern_c({}, function igBeginMainMenuBar(): c_bool { throw 0; });
const _igEndMainMenuBar = $SHBuiltin.extern_c({}, function igEndMainMenuBar(): void { throw 0; });
const _igBeginMenu = $SHBuiltin.extern_c({}, function igBeginMenu(_label: c_ptr, _enabled: c_bool): c_bool { throw 0; });
const _igEndMenu = $SHBuiltin.extern_c({}, function igEndMenu(): void { throw 0; });
const _igMenuItem_Bool = $SHBuiltin.extern_c({}, function igMenuItem_Bool(_label: c_ptr, _shortcut: c_ptr, _selected: c_bool, _enabled: c_bool): c_bool { throw 0; });
const _igMenuItem_BoolPtr = $SHBuiltin.extern_c({}, function igMenuItem_BoolPtr(_label: c_ptr, _shortcut: c_ptr, _p_selected: c_ptr, _enabled: c_bool): c_bool { throw 0; });
const _igBeginTooltip = $SHBuiltin.extern_c({}, function igBeginTooltip(): c_bool { throw 0; });
const _igEndTooltip = $SHBuiltin.extern_c({}, function igEndTooltip(): void { throw 0; });
const _igSetTooltip = $SHBuiltin.extern_c({}, function igSetTooltip(_fmt: c_ptr): void { throw 0; });
const _igSetTooltipV = $SHBuiltin.extern_c({}, function igSetTooltipV(_fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igBeginItemTooltip = $SHBuiltin.extern_c({}, function igBeginItemTooltip(): c_bool { throw 0; });
const _igSetItemTooltip = $SHBuiltin.extern_c({}, function igSetItemTooltip(_fmt: c_ptr): void { throw 0; });
const _igSetItemTooltipV = $SHBuiltin.extern_c({}, function igSetItemTooltipV(_fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igBeginPopup = $SHBuiltin.extern_c({}, function igBeginPopup(_str_id: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igBeginPopupModal = $SHBuiltin.extern_c({}, function igBeginPopupModal(_name: c_ptr, _p_open: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igEndPopup = $SHBuiltin.extern_c({}, function igEndPopup(): void { throw 0; });
const _igOpenPopup_Str = $SHBuiltin.extern_c({}, function igOpenPopup_Str(_str_id: c_ptr, _popup_flags: c_int): void { throw 0; });
const _igOpenPopup_ID = $SHBuiltin.extern_c({}, function igOpenPopup_ID(_id: c_uint, _popup_flags: c_int): void { throw 0; });
const _igOpenPopupOnItemClick = $SHBuiltin.extern_c({}, function igOpenPopupOnItemClick(_str_id: c_ptr, _popup_flags: c_int): void { throw 0; });
const _igCloseCurrentPopup = $SHBuiltin.extern_c({}, function igCloseCurrentPopup(): void { throw 0; });
const _igBeginPopupContextItem = $SHBuiltin.extern_c({}, function igBeginPopupContextItem(_str_id: c_ptr, _popup_flags: c_int): c_bool { throw 0; });
const _igBeginPopupContextWindow = $SHBuiltin.extern_c({}, function igBeginPopupContextWindow(_str_id: c_ptr, _popup_flags: c_int): c_bool { throw 0; });
const _igBeginPopupContextVoid = $SHBuiltin.extern_c({}, function igBeginPopupContextVoid(_str_id: c_ptr, _popup_flags: c_int): c_bool { throw 0; });
const _igIsPopupOpen_Str = $SHBuiltin.extern_c({}, function igIsPopupOpen_Str(_str_id: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igBeginTable = $SHBuiltin.extern_c({}, function igBeginTable_cwrap(_str_id: c_ptr, _column: c_int, _flags: c_int, _outer_size: c_ptr, _inner_width: c_float): c_bool { throw 0; });
const _igEndTable = $SHBuiltin.extern_c({}, function igEndTable(): void { throw 0; });
const _igTableNextRow = $SHBuiltin.extern_c({}, function igTableNextRow(_row_flags: c_int, _min_row_height: c_float): void { throw 0; });
const _igTableNextColumn = $SHBuiltin.extern_c({}, function igTableNextColumn(): c_bool { throw 0; });
const _igTableSetColumnIndex = $SHBuiltin.extern_c({}, function igTableSetColumnIndex(_column_n: c_int): c_bool { throw 0; });
const _igTableSetupColumn = $SHBuiltin.extern_c({}, function igTableSetupColumn(_label: c_ptr, _flags: c_int, _init_width_or_weight: c_float, _user_id: c_uint): void { throw 0; });
const _igTableSetupScrollFreeze = $SHBuiltin.extern_c({}, function igTableSetupScrollFreeze(_cols: c_int, _rows: c_int): void { throw 0; });
const _igTableHeadersRow = $SHBuiltin.extern_c({}, function igTableHeadersRow(): void { throw 0; });
const _igTableHeader = $SHBuiltin.extern_c({}, function igTableHeader(_label: c_ptr): void { throw 0; });
const _igTableGetSortSpecs = $SHBuiltin.extern_c({}, function igTableGetSortSpecs(): c_ptr { throw 0; });
const _igTableGetColumnCount = $SHBuiltin.extern_c({}, function igTableGetColumnCount(): c_int { throw 0; });
const _igTableGetColumnIndex = $SHBuiltin.extern_c({}, function igTableGetColumnIndex(): c_int { throw 0; });
const _igTableGetRowIndex = $SHBuiltin.extern_c({}, function igTableGetRowIndex(): c_int { throw 0; });
const _igTableGetColumnName_Int = $SHBuiltin.extern_c({}, function igTableGetColumnName_Int(_column_n: c_int): c_ptr { throw 0; });
const _igTableGetColumnFlags = $SHBuiltin.extern_c({}, function igTableGetColumnFlags(_column_n: c_int): c_int { throw 0; });
const _igTableSetColumnEnabled = $SHBuiltin.extern_c({}, function igTableSetColumnEnabled(_column_n: c_int, _v: c_bool): void { throw 0; });
const _igTableSetBgColor = $SHBuiltin.extern_c({}, function igTableSetBgColor(_target: c_int, _color: c_uint, _column_n: c_int): void { throw 0; });
const _igColumns = $SHBuiltin.extern_c({}, function igColumns(_count: c_int, _id: c_ptr, _border: c_bool): void { throw 0; });
const _igNextColumn = $SHBuiltin.extern_c({}, function igNextColumn(): void { throw 0; });
const _igGetColumnIndex = $SHBuiltin.extern_c({}, function igGetColumnIndex(): c_int { throw 0; });
const _igGetColumnWidth = $SHBuiltin.extern_c({}, function igGetColumnWidth(_column_index: c_int): c_float { throw 0; });
const _igSetColumnWidth = $SHBuiltin.extern_c({}, function igSetColumnWidth(_column_index: c_int, _width: c_float): void { throw 0; });
const _igGetColumnOffset = $SHBuiltin.extern_c({}, function igGetColumnOffset(_column_index: c_int): c_float { throw 0; });
const _igSetColumnOffset = $SHBuiltin.extern_c({}, function igSetColumnOffset(_column_index: c_int, _offset_x: c_float): void { throw 0; });
const _igGetColumnsCount = $SHBuiltin.extern_c({}, function igGetColumnsCount(): c_int { throw 0; });
const _igBeginTabBar = $SHBuiltin.extern_c({}, function igBeginTabBar(_str_id: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igEndTabBar = $SHBuiltin.extern_c({}, function igEndTabBar(): void { throw 0; });
const _igBeginTabItem = $SHBuiltin.extern_c({}, function igBeginTabItem(_label: c_ptr, _p_open: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igEndTabItem = $SHBuiltin.extern_c({}, function igEndTabItem(): void { throw 0; });
const _igTabItemButton = $SHBuiltin.extern_c({}, function igTabItemButton(_label: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSetTabItemClosed = $SHBuiltin.extern_c({}, function igSetTabItemClosed(_tab_or_docked_window_label: c_ptr): void { throw 0; });
const _igLogToTTY = $SHBuiltin.extern_c({}, function igLogToTTY(_auto_open_depth: c_int): void { throw 0; });
const _igLogToFile = $SHBuiltin.extern_c({}, function igLogToFile(_auto_open_depth: c_int, _filename: c_ptr): void { throw 0; });
const _igLogToClipboard = $SHBuiltin.extern_c({}, function igLogToClipboard(_auto_open_depth: c_int): void { throw 0; });
const _igLogFinish = $SHBuiltin.extern_c({}, function igLogFinish(): void { throw 0; });
const _igLogButtons = $SHBuiltin.extern_c({}, function igLogButtons(): void { throw 0; });
const _igLogTextV = $SHBuiltin.extern_c({}, function igLogTextV(_fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igBeginDragDropSource = $SHBuiltin.extern_c({}, function igBeginDragDropSource(_flags: c_int): c_bool { throw 0; });
const _igSetDragDropPayload = $SHBuiltin.extern_c({}, function igSetDragDropPayload(_type: c_ptr, _data: c_ptr, _sz: c_ulong, _cond: c_int): c_bool { throw 0; });
const _igEndDragDropSource = $SHBuiltin.extern_c({}, function igEndDragDropSource(): void { throw 0; });
const _igBeginDragDropTarget = $SHBuiltin.extern_c({}, function igBeginDragDropTarget(): c_bool { throw 0; });
const _igAcceptDragDropPayload = $SHBuiltin.extern_c({}, function igAcceptDragDropPayload(_type: c_ptr, _flags: c_int): c_ptr { throw 0; });
const _igEndDragDropTarget = $SHBuiltin.extern_c({}, function igEndDragDropTarget(): void { throw 0; });
const _igGetDragDropPayload = $SHBuiltin.extern_c({}, function igGetDragDropPayload(): c_ptr { throw 0; });
const _igBeginDisabled = $SHBuiltin.extern_c({}, function igBeginDisabled(_disabled: c_bool): void { throw 0; });
const _igEndDisabled = $SHBuiltin.extern_c({}, function igEndDisabled(): void { throw 0; });
const _igPushClipRect = $SHBuiltin.extern_c({}, function igPushClipRect_cwrap(_clip_rect_min: c_ptr, _clip_rect_max: c_ptr, _intersect_with_current_clip_rect: c_bool): void { throw 0; });
const _igPopClipRect = $SHBuiltin.extern_c({}, function igPopClipRect(): void { throw 0; });
const _igSetItemDefaultFocus = $SHBuiltin.extern_c({}, function igSetItemDefaultFocus(): void { throw 0; });
const _igSetKeyboardFocusHere = $SHBuiltin.extern_c({}, function igSetKeyboardFocusHere(_offset: c_int): void { throw 0; });
const _igSetNextItemAllowOverlap = $SHBuiltin.extern_c({}, function igSetNextItemAllowOverlap(): void { throw 0; });
const _igIsItemHovered = $SHBuiltin.extern_c({}, function igIsItemHovered(_flags: c_int): c_bool { throw 0; });
const _igIsItemActive = $SHBuiltin.extern_c({}, function igIsItemActive(): c_bool { throw 0; });
const _igIsItemFocused = $SHBuiltin.extern_c({}, function igIsItemFocused(): c_bool { throw 0; });
const _igIsItemClicked = $SHBuiltin.extern_c({}, function igIsItemClicked(_mouse_button: c_int): c_bool { throw 0; });
const _igIsItemVisible = $SHBuiltin.extern_c({}, function igIsItemVisible(): c_bool { throw 0; });
const _igIsItemEdited = $SHBuiltin.extern_c({}, function igIsItemEdited(): c_bool { throw 0; });
const _igIsItemActivated = $SHBuiltin.extern_c({}, function igIsItemActivated(): c_bool { throw 0; });
const _igIsItemDeactivated = $SHBuiltin.extern_c({}, function igIsItemDeactivated(): c_bool { throw 0; });
const _igIsItemDeactivatedAfterEdit = $SHBuiltin.extern_c({}, function igIsItemDeactivatedAfterEdit(): c_bool { throw 0; });
const _igIsItemToggledOpen = $SHBuiltin.extern_c({}, function igIsItemToggledOpen(): c_bool { throw 0; });
const _igIsAnyItemHovered = $SHBuiltin.extern_c({}, function igIsAnyItemHovered(): c_bool { throw 0; });
const _igIsAnyItemActive = $SHBuiltin.extern_c({}, function igIsAnyItemActive(): c_bool { throw 0; });
const _igIsAnyItemFocused = $SHBuiltin.extern_c({}, function igIsAnyItemFocused(): c_bool { throw 0; });
const _igGetItemID = $SHBuiltin.extern_c({}, function igGetItemID(): c_uint { throw 0; });
const _igGetItemRectMin = $SHBuiltin.extern_c({}, function igGetItemRectMin(_pOut: c_ptr): void { throw 0; });
const _igGetItemRectMax = $SHBuiltin.extern_c({}, function igGetItemRectMax(_pOut: c_ptr): void { throw 0; });
const _igGetItemRectSize = $SHBuiltin.extern_c({}, function igGetItemRectSize(_pOut: c_ptr): void { throw 0; });
const _igGetMainViewport = $SHBuiltin.extern_c({}, function igGetMainViewport(): c_ptr { throw 0; });
const _igGetBackgroundDrawList_Nil = $SHBuiltin.extern_c({}, function igGetBackgroundDrawList_Nil(): c_ptr { throw 0; });
const _igGetForegroundDrawList_Nil = $SHBuiltin.extern_c({}, function igGetForegroundDrawList_Nil(): c_ptr { throw 0; });
const _igIsRectVisible_Nil = $SHBuiltin.extern_c({}, function igIsRectVisible_Nil_cwrap(_size: c_ptr): c_bool { throw 0; });
const _igIsRectVisible_Vec2 = $SHBuiltin.extern_c({}, function igIsRectVisible_Vec2_cwrap(_rect_min: c_ptr, _rect_max: c_ptr): c_bool { throw 0; });
const _igGetTime = $SHBuiltin.extern_c({}, function igGetTime(): c_double { throw 0; });
const _igGetFrameCount = $SHBuiltin.extern_c({}, function igGetFrameCount(): c_int { throw 0; });
const _igGetDrawListSharedData = $SHBuiltin.extern_c({}, function igGetDrawListSharedData(): c_ptr { throw 0; });
const _igGetStyleColorName = $SHBuiltin.extern_c({}, function igGetStyleColorName(_idx: c_int): c_ptr { throw 0; });
const _igSetStateStorage = $SHBuiltin.extern_c({}, function igSetStateStorage(_storage: c_ptr): void { throw 0; });
const _igGetStateStorage = $SHBuiltin.extern_c({}, function igGetStateStorage(): c_ptr { throw 0; });
const _igBeginChildFrame = $SHBuiltin.extern_c({}, function igBeginChildFrame_cwrap(_id: c_uint, _size: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igEndChildFrame = $SHBuiltin.extern_c({}, function igEndChildFrame(): void { throw 0; });
const _igCalcTextSize = $SHBuiltin.extern_c({}, function igCalcTextSize(_pOut: c_ptr, _text: c_ptr, _text_end: c_ptr, _hide_text_after_double_hash: c_bool, _wrap_width: c_float): void { throw 0; });
const _igColorConvertU32ToFloat4 = $SHBuiltin.extern_c({}, function igColorConvertU32ToFloat4(_pOut: c_ptr, _in: c_uint): void { throw 0; });
const _igColorConvertFloat4ToU32 = $SHBuiltin.extern_c({}, function igColorConvertFloat4ToU32_cwrap(_in: c_ptr): c_uint { throw 0; });
const _igColorConvertRGBtoHSV = $SHBuiltin.extern_c({}, function igColorConvertRGBtoHSV(_r: c_float, _g: c_float, _b: c_float, _out_h: c_ptr, _out_s: c_ptr, _out_v: c_ptr): void { throw 0; });
const _igColorConvertHSVtoRGB = $SHBuiltin.extern_c({}, function igColorConvertHSVtoRGB(_h: c_float, _s: c_float, _v: c_float, _out_r: c_ptr, _out_g: c_ptr, _out_b: c_ptr): void { throw 0; });
const _igIsKeyDown_Nil = $SHBuiltin.extern_c({}, function igIsKeyDown_Nil(_key: c_int): c_bool { throw 0; });
const _igIsKeyPressed_Bool = $SHBuiltin.extern_c({}, function igIsKeyPressed_Bool(_key: c_int, _repeat: c_bool): c_bool { throw 0; });
const _igIsKeyReleased_Nil = $SHBuiltin.extern_c({}, function igIsKeyReleased_Nil(_key: c_int): c_bool { throw 0; });
const _igGetKeyPressedAmount = $SHBuiltin.extern_c({}, function igGetKeyPressedAmount(_key: c_int, _repeat_delay: c_float, _rate: c_float): c_int { throw 0; });
const _igGetKeyName = $SHBuiltin.extern_c({}, function igGetKeyName(_key: c_int): c_ptr { throw 0; });
const _igSetNextFrameWantCaptureKeyboard = $SHBuiltin.extern_c({}, function igSetNextFrameWantCaptureKeyboard(_want_capture_keyboard: c_bool): void { throw 0; });
const _igIsMouseDown_Nil = $SHBuiltin.extern_c({}, function igIsMouseDown_Nil(_button: c_int): c_bool { throw 0; });
const _igIsMouseClicked_Bool = $SHBuiltin.extern_c({}, function igIsMouseClicked_Bool(_button: c_int, _repeat: c_bool): c_bool { throw 0; });
const _igIsMouseReleased_Nil = $SHBuiltin.extern_c({}, function igIsMouseReleased_Nil(_button: c_int): c_bool { throw 0; });
const _igIsMouseDoubleClicked = $SHBuiltin.extern_c({}, function igIsMouseDoubleClicked(_button: c_int): c_bool { throw 0; });
const _igGetMouseClickedCount = $SHBuiltin.extern_c({}, function igGetMouseClickedCount(_button: c_int): c_int { throw 0; });
const _igIsMouseHoveringRect = $SHBuiltin.extern_c({}, function igIsMouseHoveringRect_cwrap(_r_min: c_ptr, _r_max: c_ptr, _clip: c_bool): c_bool { throw 0; });
const _igIsMousePosValid = $SHBuiltin.extern_c({}, function igIsMousePosValid(_mouse_pos: c_ptr): c_bool { throw 0; });
const _igIsAnyMouseDown = $SHBuiltin.extern_c({}, function igIsAnyMouseDown(): c_bool { throw 0; });
const _igGetMousePos = $SHBuiltin.extern_c({}, function igGetMousePos(_pOut: c_ptr): void { throw 0; });
const _igGetMousePosOnOpeningCurrentPopup = $SHBuiltin.extern_c({}, function igGetMousePosOnOpeningCurrentPopup(_pOut: c_ptr): void { throw 0; });
const _igIsMouseDragging = $SHBuiltin.extern_c({}, function igIsMouseDragging(_button: c_int, _lock_threshold: c_float): c_bool { throw 0; });
const _igGetMouseDragDelta = $SHBuiltin.extern_c({}, function igGetMouseDragDelta(_pOut: c_ptr, _button: c_int, _lock_threshold: c_float): void { throw 0; });
const _igResetMouseDragDelta = $SHBuiltin.extern_c({}, function igResetMouseDragDelta(_button: c_int): void { throw 0; });
const _igGetMouseCursor = $SHBuiltin.extern_c({}, function igGetMouseCursor(): c_int { throw 0; });
const _igSetMouseCursor = $SHBuiltin.extern_c({}, function igSetMouseCursor(_cursor_type: c_int): void { throw 0; });
const _igSetNextFrameWantCaptureMouse = $SHBuiltin.extern_c({}, function igSetNextFrameWantCaptureMouse(_want_capture_mouse: c_bool): void { throw 0; });
const _igGetClipboardText = $SHBuiltin.extern_c({}, function igGetClipboardText(): c_ptr { throw 0; });
const _igSetClipboardText = $SHBuiltin.extern_c({}, function igSetClipboardText(_text: c_ptr): void { throw 0; });
const _igLoadIniSettingsFromDisk = $SHBuiltin.extern_c({}, function igLoadIniSettingsFromDisk(_ini_filename: c_ptr): void { throw 0; });
const _igLoadIniSettingsFromMemory = $SHBuiltin.extern_c({}, function igLoadIniSettingsFromMemory(_ini_data: c_ptr, _ini_size: c_ulong): void { throw 0; });
const _igSaveIniSettingsToDisk = $SHBuiltin.extern_c({}, function igSaveIniSettingsToDisk(_ini_filename: c_ptr): void { throw 0; });
const _igSaveIniSettingsToMemory = $SHBuiltin.extern_c({}, function igSaveIniSettingsToMemory(_out_ini_size: c_ptr): c_ptr { throw 0; });
const _igDebugTextEncoding = $SHBuiltin.extern_c({}, function igDebugTextEncoding(_text: c_ptr): void { throw 0; });
const _igDebugCheckVersionAndDataLayout = $SHBuiltin.extern_c({}, function igDebugCheckVersionAndDataLayout(_version_str: c_ptr, _sz_io: c_ulong, _sz_style: c_ulong, _sz_vec2: c_ulong, _sz_vec4: c_ulong, _sz_drawvert: c_ulong, _sz_drawidx: c_ulong): c_bool { throw 0; });
const _igSetAllocatorFunctions = $SHBuiltin.extern_c({}, function igSetAllocatorFunctions(_alloc_func: c_ptr, _free_func: c_ptr, _user_data: c_ptr): void { throw 0; });
const _igGetAllocatorFunctions = $SHBuiltin.extern_c({}, function igGetAllocatorFunctions(_p_alloc_func: c_ptr, _p_free_func: c_ptr, _p_user_data: c_ptr): void { throw 0; });
const _igMemAlloc = $SHBuiltin.extern_c({}, function igMemAlloc(_size: c_ulong): c_ptr { throw 0; });
const _igMemFree = $SHBuiltin.extern_c({}, function igMemFree(_ptr: c_ptr): void { throw 0; });
const _ImGuiStyle_ImGuiStyle = $SHBuiltin.extern_c({}, function ImGuiStyle_ImGuiStyle(): c_ptr { throw 0; });
const _ImGuiStyle_destroy = $SHBuiltin.extern_c({}, function ImGuiStyle_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiStyle_ScaleAllSizes = $SHBuiltin.extern_c({}, function ImGuiStyle_ScaleAllSizes(_self: c_ptr, _scale_factor: c_float): void { throw 0; });
const _ImGuiIO_AddKeyEvent = $SHBuiltin.extern_c({}, function ImGuiIO_AddKeyEvent(_self: c_ptr, _key: c_int, _down: c_bool): void { throw 0; });
const _ImGuiIO_AddKeyAnalogEvent = $SHBuiltin.extern_c({}, function ImGuiIO_AddKeyAnalogEvent(_self: c_ptr, _key: c_int, _down: c_bool, _v: c_float): void { throw 0; });
const _ImGuiIO_AddMousePosEvent = $SHBuiltin.extern_c({}, function ImGuiIO_AddMousePosEvent(_self: c_ptr, _x: c_float, _y: c_float): void { throw 0; });
const _ImGuiIO_AddMouseButtonEvent = $SHBuiltin.extern_c({}, function ImGuiIO_AddMouseButtonEvent(_self: c_ptr, _button: c_int, _down: c_bool): void { throw 0; });
const _ImGuiIO_AddMouseWheelEvent = $SHBuiltin.extern_c({}, function ImGuiIO_AddMouseWheelEvent(_self: c_ptr, _wheel_x: c_float, _wheel_y: c_float): void { throw 0; });
const _ImGuiIO_AddMouseSourceEvent = $SHBuiltin.extern_c({}, function ImGuiIO_AddMouseSourceEvent(_self: c_ptr, _source: c_int): void { throw 0; });
const _ImGuiIO_AddFocusEvent = $SHBuiltin.extern_c({}, function ImGuiIO_AddFocusEvent(_self: c_ptr, _focused: c_bool): void { throw 0; });
const _ImGuiIO_AddInputCharacter = $SHBuiltin.extern_c({}, function ImGuiIO_AddInputCharacter(_self: c_ptr, _c: c_uint): void { throw 0; });
const _ImGuiIO_AddInputCharacterUTF16 = $SHBuiltin.extern_c({}, function ImGuiIO_AddInputCharacterUTF16(_self: c_ptr, _c: c_ushort): void { throw 0; });
const _ImGuiIO_AddInputCharactersUTF8 = $SHBuiltin.extern_c({}, function ImGuiIO_AddInputCharactersUTF8(_self: c_ptr, _str: c_ptr): void { throw 0; });
const _ImGuiIO_SetKeyEventNativeData = $SHBuiltin.extern_c({}, function ImGuiIO_SetKeyEventNativeData(_self: c_ptr, _key: c_int, _native_keycode: c_int, _native_scancode: c_int, _native_legacy_index: c_int): void { throw 0; });
const _ImGuiIO_SetAppAcceptingEvents = $SHBuiltin.extern_c({}, function ImGuiIO_SetAppAcceptingEvents(_self: c_ptr, _accepting_events: c_bool): void { throw 0; });
const _ImGuiIO_ClearEventsQueue = $SHBuiltin.extern_c({}, function ImGuiIO_ClearEventsQueue(_self: c_ptr): void { throw 0; });
const _ImGuiIO_ClearInputKeys = $SHBuiltin.extern_c({}, function ImGuiIO_ClearInputKeys(_self: c_ptr): void { throw 0; });
const _ImGuiIO_ImGuiIO = $SHBuiltin.extern_c({}, function ImGuiIO_ImGuiIO(): c_ptr { throw 0; });
const _ImGuiIO_destroy = $SHBuiltin.extern_c({}, function ImGuiIO_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextCallbackData_ImGuiInputTextCallbackData = $SHBuiltin.extern_c({}, function ImGuiInputTextCallbackData_ImGuiInputTextCallbackData(): c_ptr { throw 0; });
const _ImGuiInputTextCallbackData_destroy = $SHBuiltin.extern_c({}, function ImGuiInputTextCallbackData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextCallbackData_DeleteChars = $SHBuiltin.extern_c({}, function ImGuiInputTextCallbackData_DeleteChars(_self: c_ptr, _pos: c_int, _bytes_count: c_int): void { throw 0; });
const _ImGuiInputTextCallbackData_InsertChars = $SHBuiltin.extern_c({}, function ImGuiInputTextCallbackData_InsertChars(_self: c_ptr, _pos: c_int, _text: c_ptr, _text_end: c_ptr): void { throw 0; });
const _ImGuiInputTextCallbackData_SelectAll = $SHBuiltin.extern_c({}, function ImGuiInputTextCallbackData_SelectAll(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextCallbackData_ClearSelection = $SHBuiltin.extern_c({}, function ImGuiInputTextCallbackData_ClearSelection(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextCallbackData_HasSelection = $SHBuiltin.extern_c({}, function ImGuiInputTextCallbackData_HasSelection(_self: c_ptr): c_bool { throw 0; });
const _ImGuiPayload_ImGuiPayload = $SHBuiltin.extern_c({}, function ImGuiPayload_ImGuiPayload(): c_ptr { throw 0; });
const _ImGuiPayload_destroy = $SHBuiltin.extern_c({}, function ImGuiPayload_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiPayload_Clear = $SHBuiltin.extern_c({}, function ImGuiPayload_Clear(_self: c_ptr): void { throw 0; });
const _ImGuiPayload_IsDataType = $SHBuiltin.extern_c({}, function ImGuiPayload_IsDataType(_self: c_ptr, _type: c_ptr): c_bool { throw 0; });
const _ImGuiPayload_IsPreview = $SHBuiltin.extern_c({}, function ImGuiPayload_IsPreview(_self: c_ptr): c_bool { throw 0; });
const _ImGuiPayload_IsDelivery = $SHBuiltin.extern_c({}, function ImGuiPayload_IsDelivery(_self: c_ptr): c_bool { throw 0; });
const _ImGuiTableColumnSortSpecs_ImGuiTableColumnSortSpecs = $SHBuiltin.extern_c({}, function ImGuiTableColumnSortSpecs_ImGuiTableColumnSortSpecs(): c_ptr { throw 0; });
const _ImGuiTableColumnSortSpecs_destroy = $SHBuiltin.extern_c({}, function ImGuiTableColumnSortSpecs_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTableSortSpecs_ImGuiTableSortSpecs = $SHBuiltin.extern_c({}, function ImGuiTableSortSpecs_ImGuiTableSortSpecs(): c_ptr { throw 0; });
const _ImGuiTableSortSpecs_destroy = $SHBuiltin.extern_c({}, function ImGuiTableSortSpecs_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiOnceUponAFrame_ImGuiOnceUponAFrame = $SHBuiltin.extern_c({}, function ImGuiOnceUponAFrame_ImGuiOnceUponAFrame(): c_ptr { throw 0; });
const _ImGuiOnceUponAFrame_destroy = $SHBuiltin.extern_c({}, function ImGuiOnceUponAFrame_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTextFilter_ImGuiTextFilter = $SHBuiltin.extern_c({}, function ImGuiTextFilter_ImGuiTextFilter(_default_filter: c_ptr): c_ptr { throw 0; });
const _ImGuiTextFilter_destroy = $SHBuiltin.extern_c({}, function ImGuiTextFilter_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTextFilter_Draw = $SHBuiltin.extern_c({}, function ImGuiTextFilter_Draw(_self: c_ptr, _label: c_ptr, _width: c_float): c_bool { throw 0; });
const _ImGuiTextFilter_PassFilter = $SHBuiltin.extern_c({}, function ImGuiTextFilter_PassFilter(_self: c_ptr, _text: c_ptr, _text_end: c_ptr): c_bool { throw 0; });
const _ImGuiTextFilter_Build = $SHBuiltin.extern_c({}, function ImGuiTextFilter_Build(_self: c_ptr): void { throw 0; });
const _ImGuiTextFilter_Clear = $SHBuiltin.extern_c({}, function ImGuiTextFilter_Clear(_self: c_ptr): void { throw 0; });
const _ImGuiTextFilter_IsActive = $SHBuiltin.extern_c({}, function ImGuiTextFilter_IsActive(_self: c_ptr): c_bool { throw 0; });
const _ImGuiTextRange_ImGuiTextRange_Nil = $SHBuiltin.extern_c({}, function ImGuiTextRange_ImGuiTextRange_Nil(): c_ptr { throw 0; });
const _ImGuiTextRange_destroy = $SHBuiltin.extern_c({}, function ImGuiTextRange_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTextRange_ImGuiTextRange_Str = $SHBuiltin.extern_c({}, function ImGuiTextRange_ImGuiTextRange_Str(__b: c_ptr, __e: c_ptr): c_ptr { throw 0; });
const _ImGuiTextRange_empty = $SHBuiltin.extern_c({}, function ImGuiTextRange_empty(_self: c_ptr): c_bool { throw 0; });
const _ImGuiTextRange_split = $SHBuiltin.extern_c({}, function ImGuiTextRange_split(_self: c_ptr, _separator: c_char, _out: c_ptr): void { throw 0; });
const _ImGuiTextBuffer_ImGuiTextBuffer = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_ImGuiTextBuffer(): c_ptr { throw 0; });
const _ImGuiTextBuffer_destroy = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTextBuffer_begin = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_begin(_self: c_ptr): c_ptr { throw 0; });
const _ImGuiTextBuffer_end = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_end(_self: c_ptr): c_ptr { throw 0; });
const _ImGuiTextBuffer_size = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_size(_self: c_ptr): c_int { throw 0; });
const _ImGuiTextBuffer_empty = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_empty(_self: c_ptr): c_bool { throw 0; });
const _ImGuiTextBuffer_clear = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_clear(_self: c_ptr): void { throw 0; });
const _ImGuiTextBuffer_reserve = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_reserve(_self: c_ptr, _capacity: c_int): void { throw 0; });
const _ImGuiTextBuffer_c_str = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_c_str(_self: c_ptr): c_ptr { throw 0; });
const _ImGuiTextBuffer_append = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_append(_self: c_ptr, _str: c_ptr, _str_end: c_ptr): void { throw 0; });
const _ImGuiTextBuffer_appendfv = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_appendfv(_self: c_ptr, _fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _ImGuiStoragePair_ImGuiStoragePair_Int = $SHBuiltin.extern_c({}, function ImGuiStoragePair_ImGuiStoragePair_Int(__key: c_uint, __val_i: c_int): c_ptr { throw 0; });
const _ImGuiStoragePair_destroy = $SHBuiltin.extern_c({}, function ImGuiStoragePair_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiStoragePair_ImGuiStoragePair_Float = $SHBuiltin.extern_c({}, function ImGuiStoragePair_ImGuiStoragePair_Float(__key: c_uint, __val_f: c_float): c_ptr { throw 0; });
const _ImGuiStoragePair_ImGuiStoragePair_Ptr = $SHBuiltin.extern_c({}, function ImGuiStoragePair_ImGuiStoragePair_Ptr(__key: c_uint, __val_p: c_ptr): c_ptr { throw 0; });
const _ImGuiStorage_Clear = $SHBuiltin.extern_c({}, function ImGuiStorage_Clear(_self: c_ptr): void { throw 0; });
const _ImGuiStorage_GetInt = $SHBuiltin.extern_c({}, function ImGuiStorage_GetInt(_self: c_ptr, _key: c_uint, _default_val: c_int): c_int { throw 0; });
const _ImGuiStorage_SetInt = $SHBuiltin.extern_c({}, function ImGuiStorage_SetInt(_self: c_ptr, _key: c_uint, _val: c_int): void { throw 0; });
const _ImGuiStorage_GetBool = $SHBuiltin.extern_c({}, function ImGuiStorage_GetBool(_self: c_ptr, _key: c_uint, _default_val: c_bool): c_bool { throw 0; });
const _ImGuiStorage_SetBool = $SHBuiltin.extern_c({}, function ImGuiStorage_SetBool(_self: c_ptr, _key: c_uint, _val: c_bool): void { throw 0; });
const _ImGuiStorage_GetFloat = $SHBuiltin.extern_c({}, function ImGuiStorage_GetFloat(_self: c_ptr, _key: c_uint, _default_val: c_float): c_float { throw 0; });
const _ImGuiStorage_SetFloat = $SHBuiltin.extern_c({}, function ImGuiStorage_SetFloat(_self: c_ptr, _key: c_uint, _val: c_float): void { throw 0; });
const _ImGuiStorage_GetVoidPtr = $SHBuiltin.extern_c({}, function ImGuiStorage_GetVoidPtr(_self: c_ptr, _key: c_uint): c_ptr { throw 0; });
const _ImGuiStorage_SetVoidPtr = $SHBuiltin.extern_c({}, function ImGuiStorage_SetVoidPtr(_self: c_ptr, _key: c_uint, _val: c_ptr): void { throw 0; });
const _ImGuiStorage_GetIntRef = $SHBuiltin.extern_c({}, function ImGuiStorage_GetIntRef(_self: c_ptr, _key: c_uint, _default_val: c_int): c_ptr { throw 0; });
const _ImGuiStorage_GetBoolRef = $SHBuiltin.extern_c({}, function ImGuiStorage_GetBoolRef(_self: c_ptr, _key: c_uint, _default_val: c_bool): c_ptr { throw 0; });
const _ImGuiStorage_GetFloatRef = $SHBuiltin.extern_c({}, function ImGuiStorage_GetFloatRef(_self: c_ptr, _key: c_uint, _default_val: c_float): c_ptr { throw 0; });
const _ImGuiStorage_GetVoidPtrRef = $SHBuiltin.extern_c({}, function ImGuiStorage_GetVoidPtrRef(_self: c_ptr, _key: c_uint, _default_val: c_ptr): c_ptr { throw 0; });
const _ImGuiStorage_SetAllInt = $SHBuiltin.extern_c({}, function ImGuiStorage_SetAllInt(_self: c_ptr, _val: c_int): void { throw 0; });
const _ImGuiStorage_BuildSortByKey = $SHBuiltin.extern_c({}, function ImGuiStorage_BuildSortByKey(_self: c_ptr): void { throw 0; });
const _ImGuiListClipper_ImGuiListClipper = $SHBuiltin.extern_c({}, function ImGuiListClipper_ImGuiListClipper(): c_ptr { throw 0; });
const _ImGuiListClipper_destroy = $SHBuiltin.extern_c({}, function ImGuiListClipper_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiListClipper_Begin = $SHBuiltin.extern_c({}, function ImGuiListClipper_Begin(_self: c_ptr, _items_count: c_int, _items_height: c_float): void { throw 0; });
const _ImGuiListClipper_End = $SHBuiltin.extern_c({}, function ImGuiListClipper_End(_self: c_ptr): void { throw 0; });
const _ImGuiListClipper_Step = $SHBuiltin.extern_c({}, function ImGuiListClipper_Step(_self: c_ptr): c_bool { throw 0; });
const _ImGuiListClipper_IncludeItemByIndex = $SHBuiltin.extern_c({}, function ImGuiListClipper_IncludeItemByIndex(_self: c_ptr, _item_index: c_int): void { throw 0; });
const _ImGuiListClipper_IncludeItemsByIndex = $SHBuiltin.extern_c({}, function ImGuiListClipper_IncludeItemsByIndex(_self: c_ptr, _item_begin: c_int, _item_end: c_int): void { throw 0; });
const _ImColor_ImColor_Nil = $SHBuiltin.extern_c({}, function ImColor_ImColor_Nil(): c_ptr { throw 0; });
const _ImColor_destroy = $SHBuiltin.extern_c({}, function ImColor_destroy(_self: c_ptr): void { throw 0; });
const _ImColor_ImColor_Float = $SHBuiltin.extern_c({}, function ImColor_ImColor_Float(_r: c_float, _g: c_float, _b: c_float, _a: c_float): c_ptr { throw 0; });
const _ImColor_ImColor_Vec4 = $SHBuiltin.extern_c({}, function ImColor_ImColor_Vec4_cwrap(_col: c_ptr): c_ptr { throw 0; });
const _ImColor_ImColor_Int = $SHBuiltin.extern_c({}, function ImColor_ImColor_Int(_r: c_int, _g: c_int, _b: c_int, _a: c_int): c_ptr { throw 0; });
const _ImColor_ImColor_U32 = $SHBuiltin.extern_c({}, function ImColor_ImColor_U32(_rgba: c_uint): c_ptr { throw 0; });
const _ImColor_SetHSV = $SHBuiltin.extern_c({}, function ImColor_SetHSV(_self: c_ptr, _h: c_float, _s: c_float, _v: c_float, _a: c_float): void { throw 0; });
const _ImColor_HSV = $SHBuiltin.extern_c({}, function ImColor_HSV(_pOut: c_ptr, _h: c_float, _s: c_float, _v: c_float, _a: c_float): void { throw 0; });
const _ImDrawCmd_ImDrawCmd = $SHBuiltin.extern_c({}, function ImDrawCmd_ImDrawCmd(): c_ptr { throw 0; });
const _ImDrawCmd_destroy = $SHBuiltin.extern_c({}, function ImDrawCmd_destroy(_self: c_ptr): void { throw 0; });
const _ImDrawCmd_GetTexID = $SHBuiltin.extern_c({}, function ImDrawCmd_GetTexID(_self: c_ptr): c_ptr { throw 0; });
const _ImDrawListSplitter_ImDrawListSplitter = $SHBuiltin.extern_c({}, function ImDrawListSplitter_ImDrawListSplitter(): c_ptr { throw 0; });
const _ImDrawListSplitter_destroy = $SHBuiltin.extern_c({}, function ImDrawListSplitter_destroy(_self: c_ptr): void { throw 0; });
const _ImDrawListSplitter_Clear = $SHBuiltin.extern_c({}, function ImDrawListSplitter_Clear(_self: c_ptr): void { throw 0; });
const _ImDrawListSplitter_ClearFreeMemory = $SHBuiltin.extern_c({}, function ImDrawListSplitter_ClearFreeMemory(_self: c_ptr): void { throw 0; });
const _ImDrawListSplitter_Split = $SHBuiltin.extern_c({}, function ImDrawListSplitter_Split(_self: c_ptr, _draw_list: c_ptr, _count: c_int): void { throw 0; });
const _ImDrawListSplitter_Merge = $SHBuiltin.extern_c({}, function ImDrawListSplitter_Merge(_self: c_ptr, _draw_list: c_ptr): void { throw 0; });
const _ImDrawListSplitter_SetCurrentChannel = $SHBuiltin.extern_c({}, function ImDrawListSplitter_SetCurrentChannel(_self: c_ptr, _draw_list: c_ptr, _channel_idx: c_int): void { throw 0; });
const _ImDrawList_ImDrawList = $SHBuiltin.extern_c({}, function ImDrawList_ImDrawList(_shared_data: c_ptr): c_ptr { throw 0; });
const _ImDrawList_destroy = $SHBuiltin.extern_c({}, function ImDrawList_destroy(_self: c_ptr): void { throw 0; });
const _ImDrawList_PushClipRect = $SHBuiltin.extern_c({}, function ImDrawList_PushClipRect_cwrap(_self: c_ptr, _clip_rect_min: c_ptr, _clip_rect_max: c_ptr, _intersect_with_current_clip_rect: c_bool): void { throw 0; });
const _ImDrawList_PushClipRectFullScreen = $SHBuiltin.extern_c({}, function ImDrawList_PushClipRectFullScreen(_self: c_ptr): void { throw 0; });
const _ImDrawList_PopClipRect = $SHBuiltin.extern_c({}, function ImDrawList_PopClipRect(_self: c_ptr): void { throw 0; });
const _ImDrawList_PushTextureID = $SHBuiltin.extern_c({}, function ImDrawList_PushTextureID(_self: c_ptr, _texture_id: c_ptr): void { throw 0; });
const _ImDrawList_PopTextureID = $SHBuiltin.extern_c({}, function ImDrawList_PopTextureID(_self: c_ptr): void { throw 0; });
const _ImDrawList_GetClipRectMin = $SHBuiltin.extern_c({}, function ImDrawList_GetClipRectMin(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImDrawList_GetClipRectMax = $SHBuiltin.extern_c({}, function ImDrawList_GetClipRectMax(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImDrawList_AddLine = $SHBuiltin.extern_c({}, function ImDrawList_AddLine_cwrap(_self: c_ptr, _p1: c_ptr, _p2: c_ptr, _col: c_uint, _thickness: c_float): void { throw 0; });
const _ImDrawList_AddRect = $SHBuiltin.extern_c({}, function ImDrawList_AddRect_cwrap(_self: c_ptr, _p_min: c_ptr, _p_max: c_ptr, _col: c_uint, _rounding: c_float, _flags: c_int, _thickness: c_float): void { throw 0; });
const _ImDrawList_AddRectFilled = $SHBuiltin.extern_c({}, function ImDrawList_AddRectFilled_cwrap(_self: c_ptr, _p_min: c_ptr, _p_max: c_ptr, _col: c_uint, _rounding: c_float, _flags: c_int): void { throw 0; });
const _ImDrawList_AddRectFilledMultiColor = $SHBuiltin.extern_c({}, function ImDrawList_AddRectFilledMultiColor_cwrap(_self: c_ptr, _p_min: c_ptr, _p_max: c_ptr, _col_upr_left: c_uint, _col_upr_right: c_uint, _col_bot_right: c_uint, _col_bot_left: c_uint): void { throw 0; });
const _ImDrawList_AddQuad = $SHBuiltin.extern_c({}, function ImDrawList_AddQuad_cwrap(_self: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _p4: c_ptr, _col: c_uint, _thickness: c_float): void { throw 0; });
const _ImDrawList_AddQuadFilled = $SHBuiltin.extern_c({}, function ImDrawList_AddQuadFilled_cwrap(_self: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _p4: c_ptr, _col: c_uint): void { throw 0; });
const _ImDrawList_AddTriangle = $SHBuiltin.extern_c({}, function ImDrawList_AddTriangle_cwrap(_self: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _col: c_uint, _thickness: c_float): void { throw 0; });
const _ImDrawList_AddTriangleFilled = $SHBuiltin.extern_c({}, function ImDrawList_AddTriangleFilled_cwrap(_self: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _col: c_uint): void { throw 0; });
const _ImDrawList_AddCircle = $SHBuiltin.extern_c({}, function ImDrawList_AddCircle_cwrap(_self: c_ptr, _center: c_ptr, _radius: c_float, _col: c_uint, _num_segments: c_int, _thickness: c_float): void { throw 0; });
const _ImDrawList_AddCircleFilled = $SHBuiltin.extern_c({}, function ImDrawList_AddCircleFilled_cwrap(_self: c_ptr, _center: c_ptr, _radius: c_float, _col: c_uint, _num_segments: c_int): void { throw 0; });
const _ImDrawList_AddNgon = $SHBuiltin.extern_c({}, function ImDrawList_AddNgon_cwrap(_self: c_ptr, _center: c_ptr, _radius: c_float, _col: c_uint, _num_segments: c_int, _thickness: c_float): void { throw 0; });
const _ImDrawList_AddNgonFilled = $SHBuiltin.extern_c({}, function ImDrawList_AddNgonFilled_cwrap(_self: c_ptr, _center: c_ptr, _radius: c_float, _col: c_uint, _num_segments: c_int): void { throw 0; });
const _ImDrawList_AddText_Vec2 = $SHBuiltin.extern_c({}, function ImDrawList_AddText_Vec2_cwrap(_self: c_ptr, _pos: c_ptr, _col: c_uint, _text_begin: c_ptr, _text_end: c_ptr): void { throw 0; });
const _ImDrawList_AddText_FontPtr = $SHBuiltin.extern_c({}, function ImDrawList_AddText_FontPtr_cwrap(_self: c_ptr, _font: c_ptr, _font_size: c_float, _pos: c_ptr, _col: c_uint, _text_begin: c_ptr, _text_end: c_ptr, _wrap_width: c_float, _cpu_fine_clip_rect: c_ptr): void { throw 0; });
const _ImDrawList_AddPolyline = $SHBuiltin.extern_c({}, function ImDrawList_AddPolyline(_self: c_ptr, _points: c_ptr, _num_points: c_int, _col: c_uint, _flags: c_int, _thickness: c_float): void { throw 0; });
const _ImDrawList_AddConvexPolyFilled = $SHBuiltin.extern_c({}, function ImDrawList_AddConvexPolyFilled(_self: c_ptr, _points: c_ptr, _num_points: c_int, _col: c_uint): void { throw 0; });
const _ImDrawList_AddBezierCubic = $SHBuiltin.extern_c({}, function ImDrawList_AddBezierCubic_cwrap(_self: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _p4: c_ptr, _col: c_uint, _thickness: c_float, _num_segments: c_int): void { throw 0; });
const _ImDrawList_AddBezierQuadratic = $SHBuiltin.extern_c({}, function ImDrawList_AddBezierQuadratic_cwrap(_self: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _col: c_uint, _thickness: c_float, _num_segments: c_int): void { throw 0; });
const _ImDrawList_AddImage = $SHBuiltin.extern_c({}, function ImDrawList_AddImage_cwrap(_self: c_ptr, _user_texture_id: c_ptr, _p_min: c_ptr, _p_max: c_ptr, _uv_min: c_ptr, _uv_max: c_ptr, _col: c_uint): void { throw 0; });
const _ImDrawList_AddImageQuad = $SHBuiltin.extern_c({}, function ImDrawList_AddImageQuad_cwrap(_self: c_ptr, _user_texture_id: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _p4: c_ptr, _uv1: c_ptr, _uv2: c_ptr, _uv3: c_ptr, _uv4: c_ptr, _col: c_uint): void { throw 0; });
const _ImDrawList_AddImageRounded = $SHBuiltin.extern_c({}, function ImDrawList_AddImageRounded_cwrap(_self: c_ptr, _user_texture_id: c_ptr, _p_min: c_ptr, _p_max: c_ptr, _uv_min: c_ptr, _uv_max: c_ptr, _col: c_uint, _rounding: c_float, _flags: c_int): void { throw 0; });
const _ImDrawList_PathClear = $SHBuiltin.extern_c({}, function ImDrawList_PathClear(_self: c_ptr): void { throw 0; });
const _ImDrawList_PathLineTo = $SHBuiltin.extern_c({}, function ImDrawList_PathLineTo_cwrap(_self: c_ptr, _pos: c_ptr): void { throw 0; });
const _ImDrawList_PathLineToMergeDuplicate = $SHBuiltin.extern_c({}, function ImDrawList_PathLineToMergeDuplicate_cwrap(_self: c_ptr, _pos: c_ptr): void { throw 0; });
const _ImDrawList_PathFillConvex = $SHBuiltin.extern_c({}, function ImDrawList_PathFillConvex(_self: c_ptr, _col: c_uint): void { throw 0; });
const _ImDrawList_PathStroke = $SHBuiltin.extern_c({}, function ImDrawList_PathStroke(_self: c_ptr, _col: c_uint, _flags: c_int, _thickness: c_float): void { throw 0; });
const _ImDrawList_PathArcTo = $SHBuiltin.extern_c({}, function ImDrawList_PathArcTo_cwrap(_self: c_ptr, _center: c_ptr, _radius: c_float, _a_min: c_float, _a_max: c_float, _num_segments: c_int): void { throw 0; });
const _ImDrawList_PathArcToFast = $SHBuiltin.extern_c({}, function ImDrawList_PathArcToFast_cwrap(_self: c_ptr, _center: c_ptr, _radius: c_float, _a_min_of_12: c_int, _a_max_of_12: c_int): void { throw 0; });
const _ImDrawList_PathBezierCubicCurveTo = $SHBuiltin.extern_c({}, function ImDrawList_PathBezierCubicCurveTo_cwrap(_self: c_ptr, _p2: c_ptr, _p3: c_ptr, _p4: c_ptr, _num_segments: c_int): void { throw 0; });
const _ImDrawList_PathBezierQuadraticCurveTo = $SHBuiltin.extern_c({}, function ImDrawList_PathBezierQuadraticCurveTo_cwrap(_self: c_ptr, _p2: c_ptr, _p3: c_ptr, _num_segments: c_int): void { throw 0; });
const _ImDrawList_PathRect = $SHBuiltin.extern_c({}, function ImDrawList_PathRect_cwrap(_self: c_ptr, _rect_min: c_ptr, _rect_max: c_ptr, _rounding: c_float, _flags: c_int): void { throw 0; });
const _ImDrawList_AddCallback = $SHBuiltin.extern_c({}, function ImDrawList_AddCallback(_self: c_ptr, _callback: c_ptr, _callback_data: c_ptr): void { throw 0; });
const _ImDrawList_AddDrawCmd = $SHBuiltin.extern_c({}, function ImDrawList_AddDrawCmd(_self: c_ptr): void { throw 0; });
const _ImDrawList_CloneOutput = $SHBuiltin.extern_c({}, function ImDrawList_CloneOutput(_self: c_ptr): c_ptr { throw 0; });
const _ImDrawList_ChannelsSplit = $SHBuiltin.extern_c({}, function ImDrawList_ChannelsSplit(_self: c_ptr, _count: c_int): void { throw 0; });
const _ImDrawList_ChannelsMerge = $SHBuiltin.extern_c({}, function ImDrawList_ChannelsMerge(_self: c_ptr): void { throw 0; });
const _ImDrawList_ChannelsSetCurrent = $SHBuiltin.extern_c({}, function ImDrawList_ChannelsSetCurrent(_self: c_ptr, _n: c_int): void { throw 0; });
const _ImDrawList_PrimReserve = $SHBuiltin.extern_c({}, function ImDrawList_PrimReserve(_self: c_ptr, _idx_count: c_int, _vtx_count: c_int): void { throw 0; });
const _ImDrawList_PrimUnreserve = $SHBuiltin.extern_c({}, function ImDrawList_PrimUnreserve(_self: c_ptr, _idx_count: c_int, _vtx_count: c_int): void { throw 0; });
const _ImDrawList_PrimRect = $SHBuiltin.extern_c({}, function ImDrawList_PrimRect_cwrap(_self: c_ptr, _a: c_ptr, _b: c_ptr, _col: c_uint): void { throw 0; });
const _ImDrawList_PrimRectUV = $SHBuiltin.extern_c({}, function ImDrawList_PrimRectUV_cwrap(_self: c_ptr, _a: c_ptr, _b: c_ptr, _uv_a: c_ptr, _uv_b: c_ptr, _col: c_uint): void { throw 0; });
const _ImDrawList_PrimQuadUV = $SHBuiltin.extern_c({}, function ImDrawList_PrimQuadUV_cwrap(_self: c_ptr, _a: c_ptr, _b: c_ptr, _c: c_ptr, _d: c_ptr, _uv_a: c_ptr, _uv_b: c_ptr, _uv_c: c_ptr, _uv_d: c_ptr, _col: c_uint): void { throw 0; });
const _ImDrawList_PrimWriteVtx = $SHBuiltin.extern_c({}, function ImDrawList_PrimWriteVtx_cwrap(_self: c_ptr, _pos: c_ptr, _uv: c_ptr, _col: c_uint): void { throw 0; });
const _ImDrawList_PrimWriteIdx = $SHBuiltin.extern_c({}, function ImDrawList_PrimWriteIdx(_self: c_ptr, _idx: c_ushort): void { throw 0; });
const _ImDrawList_PrimVtx = $SHBuiltin.extern_c({}, function ImDrawList_PrimVtx_cwrap(_self: c_ptr, _pos: c_ptr, _uv: c_ptr, _col: c_uint): void { throw 0; });
const _ImDrawList__ResetForNewFrame = $SHBuiltin.extern_c({}, function ImDrawList__ResetForNewFrame(_self: c_ptr): void { throw 0; });
const _ImDrawList__ClearFreeMemory = $SHBuiltin.extern_c({}, function ImDrawList__ClearFreeMemory(_self: c_ptr): void { throw 0; });
const _ImDrawList__PopUnusedDrawCmd = $SHBuiltin.extern_c({}, function ImDrawList__PopUnusedDrawCmd(_self: c_ptr): void { throw 0; });
const _ImDrawList__TryMergeDrawCmds = $SHBuiltin.extern_c({}, function ImDrawList__TryMergeDrawCmds(_self: c_ptr): void { throw 0; });
const _ImDrawList__OnChangedClipRect = $SHBuiltin.extern_c({}, function ImDrawList__OnChangedClipRect(_self: c_ptr): void { throw 0; });
const _ImDrawList__OnChangedTextureID = $SHBuiltin.extern_c({}, function ImDrawList__OnChangedTextureID(_self: c_ptr): void { throw 0; });
const _ImDrawList__OnChangedVtxOffset = $SHBuiltin.extern_c({}, function ImDrawList__OnChangedVtxOffset(_self: c_ptr): void { throw 0; });
const _ImDrawList__CalcCircleAutoSegmentCount = $SHBuiltin.extern_c({}, function ImDrawList__CalcCircleAutoSegmentCount(_self: c_ptr, _radius: c_float): c_int { throw 0; });
const _ImDrawList__PathArcToFastEx = $SHBuiltin.extern_c({}, function ImDrawList__PathArcToFastEx_cwrap(_self: c_ptr, _center: c_ptr, _radius: c_float, _a_min_sample: c_int, _a_max_sample: c_int, _a_step: c_int): void { throw 0; });
const _ImDrawList__PathArcToN = $SHBuiltin.extern_c({}, function ImDrawList__PathArcToN_cwrap(_self: c_ptr, _center: c_ptr, _radius: c_float, _a_min: c_float, _a_max: c_float, _num_segments: c_int): void { throw 0; });
const _ImDrawData_ImDrawData = $SHBuiltin.extern_c({}, function ImDrawData_ImDrawData(): c_ptr { throw 0; });
const _ImDrawData_destroy = $SHBuiltin.extern_c({}, function ImDrawData_destroy(_self: c_ptr): void { throw 0; });
const _ImDrawData_Clear = $SHBuiltin.extern_c({}, function ImDrawData_Clear(_self: c_ptr): void { throw 0; });
const _ImDrawData_AddDrawList = $SHBuiltin.extern_c({}, function ImDrawData_AddDrawList(_self: c_ptr, _draw_list: c_ptr): void { throw 0; });
const _ImDrawData_DeIndexAllBuffers = $SHBuiltin.extern_c({}, function ImDrawData_DeIndexAllBuffers(_self: c_ptr): void { throw 0; });
const _ImDrawData_ScaleClipRects = $SHBuiltin.extern_c({}, function ImDrawData_ScaleClipRects_cwrap(_self: c_ptr, _fb_scale: c_ptr): void { throw 0; });
const _ImFontConfig_ImFontConfig = $SHBuiltin.extern_c({}, function ImFontConfig_ImFontConfig(): c_ptr { throw 0; });
const _ImFontConfig_destroy = $SHBuiltin.extern_c({}, function ImFontConfig_destroy(_self: c_ptr): void { throw 0; });
const _ImFontGlyphRangesBuilder_ImFontGlyphRangesBuilder = $SHBuiltin.extern_c({}, function ImFontGlyphRangesBuilder_ImFontGlyphRangesBuilder(): c_ptr { throw 0; });
const _ImFontGlyphRangesBuilder_destroy = $SHBuiltin.extern_c({}, function ImFontGlyphRangesBuilder_destroy(_self: c_ptr): void { throw 0; });
const _ImFontGlyphRangesBuilder_Clear = $SHBuiltin.extern_c({}, function ImFontGlyphRangesBuilder_Clear(_self: c_ptr): void { throw 0; });
const _ImFontGlyphRangesBuilder_GetBit = $SHBuiltin.extern_c({}, function ImFontGlyphRangesBuilder_GetBit(_self: c_ptr, _n: c_ulong): c_bool { throw 0; });
const _ImFontGlyphRangesBuilder_SetBit = $SHBuiltin.extern_c({}, function ImFontGlyphRangesBuilder_SetBit(_self: c_ptr, _n: c_ulong): void { throw 0; });
const _ImFontGlyphRangesBuilder_AddChar = $SHBuiltin.extern_c({}, function ImFontGlyphRangesBuilder_AddChar(_self: c_ptr, _c: c_ushort): void { throw 0; });
const _ImFontGlyphRangesBuilder_AddText = $SHBuiltin.extern_c({}, function ImFontGlyphRangesBuilder_AddText(_self: c_ptr, _text: c_ptr, _text_end: c_ptr): void { throw 0; });
const _ImFontGlyphRangesBuilder_AddRanges = $SHBuiltin.extern_c({}, function ImFontGlyphRangesBuilder_AddRanges(_self: c_ptr, _ranges: c_ptr): void { throw 0; });
const _ImFontGlyphRangesBuilder_BuildRanges = $SHBuiltin.extern_c({}, function ImFontGlyphRangesBuilder_BuildRanges(_self: c_ptr, _out_ranges: c_ptr): void { throw 0; });
const _ImFontAtlasCustomRect_ImFontAtlasCustomRect = $SHBuiltin.extern_c({}, function ImFontAtlasCustomRect_ImFontAtlasCustomRect(): c_ptr { throw 0; });
const _ImFontAtlasCustomRect_destroy = $SHBuiltin.extern_c({}, function ImFontAtlasCustomRect_destroy(_self: c_ptr): void { throw 0; });
const _ImFontAtlasCustomRect_IsPacked = $SHBuiltin.extern_c({}, function ImFontAtlasCustomRect_IsPacked(_self: c_ptr): c_bool { throw 0; });
const _ImFontAtlas_ImFontAtlas = $SHBuiltin.extern_c({}, function ImFontAtlas_ImFontAtlas(): c_ptr { throw 0; });
const _ImFontAtlas_destroy = $SHBuiltin.extern_c({}, function ImFontAtlas_destroy(_self: c_ptr): void { throw 0; });
const _ImFontAtlas_AddFont = $SHBuiltin.extern_c({}, function ImFontAtlas_AddFont(_self: c_ptr, _font_cfg: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_AddFontDefault = $SHBuiltin.extern_c({}, function ImFontAtlas_AddFontDefault(_self: c_ptr, _font_cfg: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_AddFontFromFileTTF = $SHBuiltin.extern_c({}, function ImFontAtlas_AddFontFromFileTTF(_self: c_ptr, _filename: c_ptr, _size_pixels: c_float, _font_cfg: c_ptr, _glyph_ranges: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_AddFontFromMemoryTTF = $SHBuiltin.extern_c({}, function ImFontAtlas_AddFontFromMemoryTTF(_self: c_ptr, _font_data: c_ptr, _font_size: c_int, _size_pixels: c_float, _font_cfg: c_ptr, _glyph_ranges: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_AddFontFromMemoryCompressedTTF = $SHBuiltin.extern_c({}, function ImFontAtlas_AddFontFromMemoryCompressedTTF(_self: c_ptr, _compressed_font_data: c_ptr, _compressed_font_size: c_int, _size_pixels: c_float, _font_cfg: c_ptr, _glyph_ranges: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_AddFontFromMemoryCompressedBase85TTF = $SHBuiltin.extern_c({}, function ImFontAtlas_AddFontFromMemoryCompressedBase85TTF(_self: c_ptr, _compressed_font_data_base85: c_ptr, _size_pixels: c_float, _font_cfg: c_ptr, _glyph_ranges: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_ClearInputData = $SHBuiltin.extern_c({}, function ImFontAtlas_ClearInputData(_self: c_ptr): void { throw 0; });
const _ImFontAtlas_ClearTexData = $SHBuiltin.extern_c({}, function ImFontAtlas_ClearTexData(_self: c_ptr): void { throw 0; });
const _ImFontAtlas_ClearFonts = $SHBuiltin.extern_c({}, function ImFontAtlas_ClearFonts(_self: c_ptr): void { throw 0; });
const _ImFontAtlas_Clear = $SHBuiltin.extern_c({}, function ImFontAtlas_Clear(_self: c_ptr): void { throw 0; });
const _ImFontAtlas_Build = $SHBuiltin.extern_c({}, function ImFontAtlas_Build(_self: c_ptr): c_bool { throw 0; });
const _ImFontAtlas_GetTexDataAsAlpha8 = $SHBuiltin.extern_c({}, function ImFontAtlas_GetTexDataAsAlpha8(_self: c_ptr, _out_pixels: c_ptr, _out_width: c_ptr, _out_height: c_ptr, _out_bytes_per_pixel: c_ptr): void { throw 0; });
const _ImFontAtlas_GetTexDataAsRGBA32 = $SHBuiltin.extern_c({}, function ImFontAtlas_GetTexDataAsRGBA32(_self: c_ptr, _out_pixels: c_ptr, _out_width: c_ptr, _out_height: c_ptr, _out_bytes_per_pixel: c_ptr): void { throw 0; });
const _ImFontAtlas_IsBuilt = $SHBuiltin.extern_c({}, function ImFontAtlas_IsBuilt(_self: c_ptr): c_bool { throw 0; });
const _ImFontAtlas_SetTexID = $SHBuiltin.extern_c({}, function ImFontAtlas_SetTexID(_self: c_ptr, _id: c_ptr): void { throw 0; });
const _ImFontAtlas_GetGlyphRangesDefault = $SHBuiltin.extern_c({}, function ImFontAtlas_GetGlyphRangesDefault(_self: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_GetGlyphRangesGreek = $SHBuiltin.extern_c({}, function ImFontAtlas_GetGlyphRangesGreek(_self: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_GetGlyphRangesKorean = $SHBuiltin.extern_c({}, function ImFontAtlas_GetGlyphRangesKorean(_self: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_GetGlyphRangesJapanese = $SHBuiltin.extern_c({}, function ImFontAtlas_GetGlyphRangesJapanese(_self: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_GetGlyphRangesChineseFull = $SHBuiltin.extern_c({}, function ImFontAtlas_GetGlyphRangesChineseFull(_self: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_GetGlyphRangesChineseSimplifiedCommon = $SHBuiltin.extern_c({}, function ImFontAtlas_GetGlyphRangesChineseSimplifiedCommon(_self: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_GetGlyphRangesCyrillic = $SHBuiltin.extern_c({}, function ImFontAtlas_GetGlyphRangesCyrillic(_self: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_GetGlyphRangesThai = $SHBuiltin.extern_c({}, function ImFontAtlas_GetGlyphRangesThai(_self: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_GetGlyphRangesVietnamese = $SHBuiltin.extern_c({}, function ImFontAtlas_GetGlyphRangesVietnamese(_self: c_ptr): c_ptr { throw 0; });
const _ImFontAtlas_AddCustomRectRegular = $SHBuiltin.extern_c({}, function ImFontAtlas_AddCustomRectRegular(_self: c_ptr, _width: c_int, _height: c_int): c_int { throw 0; });
const _ImFontAtlas_AddCustomRectFontGlyph = $SHBuiltin.extern_c({}, function ImFontAtlas_AddCustomRectFontGlyph_cwrap(_self: c_ptr, _font: c_ptr, _id: c_ushort, _width: c_int, _height: c_int, _advance_x: c_float, _offset: c_ptr): c_int { throw 0; });
const _ImFontAtlas_GetCustomRectByIndex = $SHBuiltin.extern_c({}, function ImFontAtlas_GetCustomRectByIndex(_self: c_ptr, _index: c_int): c_ptr { throw 0; });
const _ImFontAtlas_CalcCustomRectUV = $SHBuiltin.extern_c({}, function ImFontAtlas_CalcCustomRectUV(_self: c_ptr, _rect: c_ptr, _out_uv_min: c_ptr, _out_uv_max: c_ptr): void { throw 0; });
const _ImFontAtlas_GetMouseCursorTexData = $SHBuiltin.extern_c({}, function ImFontAtlas_GetMouseCursorTexData(_self: c_ptr, _cursor: c_int, _out_offset: c_ptr, _out_size: c_ptr, _out_uv_border: c_ptr, _out_uv_fill: c_ptr): c_bool { throw 0; });
const _ImFont_ImFont = $SHBuiltin.extern_c({}, function ImFont_ImFont(): c_ptr { throw 0; });
const _ImFont_destroy = $SHBuiltin.extern_c({}, function ImFont_destroy(_self: c_ptr): void { throw 0; });
const _ImFont_FindGlyph = $SHBuiltin.extern_c({}, function ImFont_FindGlyph(_self: c_ptr, _c: c_ushort): c_ptr { throw 0; });
const _ImFont_FindGlyphNoFallback = $SHBuiltin.extern_c({}, function ImFont_FindGlyphNoFallback(_self: c_ptr, _c: c_ushort): c_ptr { throw 0; });
const _ImFont_GetCharAdvance = $SHBuiltin.extern_c({}, function ImFont_GetCharAdvance(_self: c_ptr, _c: c_ushort): c_float { throw 0; });
const _ImFont_IsLoaded = $SHBuiltin.extern_c({}, function ImFont_IsLoaded(_self: c_ptr): c_bool { throw 0; });
const _ImFont_GetDebugName = $SHBuiltin.extern_c({}, function ImFont_GetDebugName(_self: c_ptr): c_ptr { throw 0; });
const _ImFont_CalcTextSizeA = $SHBuiltin.extern_c({}, function ImFont_CalcTextSizeA(_pOut: c_ptr, _self: c_ptr, _size: c_float, _max_width: c_float, _wrap_width: c_float, _text_begin: c_ptr, _text_end: c_ptr, _remaining: c_ptr): void { throw 0; });
const _ImFont_CalcWordWrapPositionA = $SHBuiltin.extern_c({}, function ImFont_CalcWordWrapPositionA(_self: c_ptr, _scale: c_float, _text: c_ptr, _text_end: c_ptr, _wrap_width: c_float): c_ptr { throw 0; });
const _ImFont_RenderChar = $SHBuiltin.extern_c({}, function ImFont_RenderChar_cwrap(_self: c_ptr, _draw_list: c_ptr, _size: c_float, _pos: c_ptr, _col: c_uint, _c: c_ushort): void { throw 0; });
const _ImFont_RenderText = $SHBuiltin.extern_c({}, function ImFont_RenderText_cwrap(_self: c_ptr, _draw_list: c_ptr, _size: c_float, _pos: c_ptr, _col: c_uint, _clip_rect: c_ptr, _text_begin: c_ptr, _text_end: c_ptr, _wrap_width: c_float, _cpu_fine_clip: c_bool): void { throw 0; });
const _ImFont_BuildLookupTable = $SHBuiltin.extern_c({}, function ImFont_BuildLookupTable(_self: c_ptr): void { throw 0; });
const _ImFont_ClearOutputData = $SHBuiltin.extern_c({}, function ImFont_ClearOutputData(_self: c_ptr): void { throw 0; });
const _ImFont_GrowIndex = $SHBuiltin.extern_c({}, function ImFont_GrowIndex(_self: c_ptr, _new_size: c_int): void { throw 0; });
const _ImFont_AddGlyph = $SHBuiltin.extern_c({}, function ImFont_AddGlyph(_self: c_ptr, _src_cfg: c_ptr, _c: c_ushort, _x0: c_float, _y0: c_float, _x1: c_float, _y1: c_float, _u0: c_float, _v0: c_float, _u1: c_float, _v1: c_float, _advance_x: c_float): void { throw 0; });
const _ImFont_AddRemapChar = $SHBuiltin.extern_c({}, function ImFont_AddRemapChar(_self: c_ptr, _dst: c_ushort, _src: c_ushort, _overwrite_dst: c_bool): void { throw 0; });
const _ImFont_SetGlyphVisible = $SHBuiltin.extern_c({}, function ImFont_SetGlyphVisible(_self: c_ptr, _c: c_ushort, _visible: c_bool): void { throw 0; });
const _ImFont_IsGlyphRangeUnused = $SHBuiltin.extern_c({}, function ImFont_IsGlyphRangeUnused(_self: c_ptr, _c_begin: c_uint, _c_last: c_uint): c_bool { throw 0; });
const _ImGuiViewport_ImGuiViewport = $SHBuiltin.extern_c({}, function ImGuiViewport_ImGuiViewport(): c_ptr { throw 0; });
const _ImGuiViewport_destroy = $SHBuiltin.extern_c({}, function ImGuiViewport_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiViewport_GetCenter = $SHBuiltin.extern_c({}, function ImGuiViewport_GetCenter(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImGuiViewport_GetWorkCenter = $SHBuiltin.extern_c({}, function ImGuiViewport_GetWorkCenter(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImGuiPlatformImeData_ImGuiPlatformImeData = $SHBuiltin.extern_c({}, function ImGuiPlatformImeData_ImGuiPlatformImeData(): c_ptr { throw 0; });
const _ImGuiPlatformImeData_destroy = $SHBuiltin.extern_c({}, function ImGuiPlatformImeData_destroy(_self: c_ptr): void { throw 0; });
const _igGetKeyIndex = $SHBuiltin.extern_c({}, function igGetKeyIndex(_key: c_int): c_int { throw 0; });
const _igImHashData = $SHBuiltin.extern_c({}, function igImHashData(_data: c_ptr, _data_size: c_ulong, _seed: c_uint): c_uint { throw 0; });
const _igImHashStr = $SHBuiltin.extern_c({}, function igImHashStr(_data: c_ptr, _data_size: c_ulong, _seed: c_uint): c_uint { throw 0; });
const _igImQsort = $SHBuiltin.extern_c({}, function igImQsort(_base: c_ptr, _count: c_ulong, _size_of_element: c_ulong, _compare_func: c_ptr): void { throw 0; });
const _igImAlphaBlendColors = $SHBuiltin.extern_c({}, function igImAlphaBlendColors(_col_a: c_uint, _col_b: c_uint): c_uint { throw 0; });
const _igImIsPowerOfTwo_Int = $SHBuiltin.extern_c({}, function igImIsPowerOfTwo_Int(_v: c_int): c_bool { throw 0; });
const _igImIsPowerOfTwo_U64 = $SHBuiltin.extern_c({}, function igImIsPowerOfTwo_U64(_v: c_ulonglong): c_bool { throw 0; });
const _igImUpperPowerOfTwo = $SHBuiltin.extern_c({}, function igImUpperPowerOfTwo(_v: c_int): c_int { throw 0; });
const _igImStricmp = $SHBuiltin.extern_c({}, function igImStricmp(_str1: c_ptr, _str2: c_ptr): c_int { throw 0; });
const _igImStrnicmp = $SHBuiltin.extern_c({}, function igImStrnicmp(_str1: c_ptr, _str2: c_ptr, _count: c_ulong): c_int { throw 0; });
const _igImStrncpy = $SHBuiltin.extern_c({}, function igImStrncpy(_dst: c_ptr, _src: c_ptr, _count: c_ulong): void { throw 0; });
const _igImStrdup = $SHBuiltin.extern_c({}, function igImStrdup(_str: c_ptr): c_ptr { throw 0; });
const _igImStrdupcpy = $SHBuiltin.extern_c({}, function igImStrdupcpy(_dst: c_ptr, _p_dst_size: c_ptr, _str: c_ptr): c_ptr { throw 0; });
const _igImStrchrRange = $SHBuiltin.extern_c({}, function igImStrchrRange(_str_begin: c_ptr, _str_end: c_ptr, _c: c_char): c_ptr { throw 0; });
const _igImStrlenW = $SHBuiltin.extern_c({}, function igImStrlenW(_str: c_ptr): c_int { throw 0; });
const _igImStreolRange = $SHBuiltin.extern_c({}, function igImStreolRange(_str: c_ptr, _str_end: c_ptr): c_ptr { throw 0; });
const _igImStrbolW = $SHBuiltin.extern_c({}, function igImStrbolW(_buf_mid_line: c_ptr, _buf_begin: c_ptr): c_ptr { throw 0; });
const _igImStristr = $SHBuiltin.extern_c({}, function igImStristr(_haystack: c_ptr, _haystack_end: c_ptr, _needle: c_ptr, _needle_end: c_ptr): c_ptr { throw 0; });
const _igImStrTrimBlanks = $SHBuiltin.extern_c({}, function igImStrTrimBlanks(_str: c_ptr): void { throw 0; });
const _igImStrSkipBlank = $SHBuiltin.extern_c({}, function igImStrSkipBlank(_str: c_ptr): c_ptr { throw 0; });
const _igImToUpper = $SHBuiltin.extern_c({}, function igImToUpper(_c: c_char): c_char { throw 0; });
const _igImCharIsBlankA = $SHBuiltin.extern_c({}, function igImCharIsBlankA(_c: c_char): c_bool { throw 0; });
const _igImCharIsBlankW = $SHBuiltin.extern_c({}, function igImCharIsBlankW(_c: c_uint): c_bool { throw 0; });
const _igImFormatString = $SHBuiltin.extern_c({}, function igImFormatString(_buf: c_ptr, _buf_size: c_ulong, _fmt: c_ptr): c_int { throw 0; });
const _igImFormatStringV = $SHBuiltin.extern_c({}, function igImFormatStringV(_buf: c_ptr, _buf_size: c_ulong, _fmt: c_ptr, _args: c_ptr): c_int { throw 0; });
const _igImFormatStringToTempBuffer = $SHBuiltin.extern_c({}, function igImFormatStringToTempBuffer(_out_buf: c_ptr, _out_buf_end: c_ptr, _fmt: c_ptr): void { throw 0; });
const _igImFormatStringToTempBufferV = $SHBuiltin.extern_c({}, function igImFormatStringToTempBufferV(_out_buf: c_ptr, _out_buf_end: c_ptr, _fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igImParseFormatFindStart = $SHBuiltin.extern_c({}, function igImParseFormatFindStart(_format: c_ptr): c_ptr { throw 0; });
const _igImParseFormatFindEnd = $SHBuiltin.extern_c({}, function igImParseFormatFindEnd(_format: c_ptr): c_ptr { throw 0; });
const _igImParseFormatTrimDecorations = $SHBuiltin.extern_c({}, function igImParseFormatTrimDecorations(_format: c_ptr, _buf: c_ptr, _buf_size: c_ulong): c_ptr { throw 0; });
const _igImParseFormatSanitizeForPrinting = $SHBuiltin.extern_c({}, function igImParseFormatSanitizeForPrinting(_fmt_in: c_ptr, _fmt_out: c_ptr, _fmt_out_size: c_ulong): void { throw 0; });
const _igImParseFormatSanitizeForScanning = $SHBuiltin.extern_c({}, function igImParseFormatSanitizeForScanning(_fmt_in: c_ptr, _fmt_out: c_ptr, _fmt_out_size: c_ulong): c_ptr { throw 0; });
const _igImParseFormatPrecision = $SHBuiltin.extern_c({}, function igImParseFormatPrecision(_format: c_ptr, _default_value: c_int): c_int { throw 0; });
const _igImTextCharToUtf8 = $SHBuiltin.extern_c({}, function igImTextCharToUtf8(_out_buf: c_ptr, _c: c_uint): c_ptr { throw 0; });
const _igImTextStrToUtf8 = $SHBuiltin.extern_c({}, function igImTextStrToUtf8(_out_buf: c_ptr, _out_buf_size: c_int, _in_text: c_ptr, _in_text_end: c_ptr): c_int { throw 0; });
const _igImTextCharFromUtf8 = $SHBuiltin.extern_c({}, function igImTextCharFromUtf8(_out_char: c_ptr, _in_text: c_ptr, _in_text_end: c_ptr): c_int { throw 0; });
const _igImTextStrFromUtf8 = $SHBuiltin.extern_c({}, function igImTextStrFromUtf8(_out_buf: c_ptr, _out_buf_size: c_int, _in_text: c_ptr, _in_text_end: c_ptr, _in_remaining: c_ptr): c_int { throw 0; });
const _igImTextCountCharsFromUtf8 = $SHBuiltin.extern_c({}, function igImTextCountCharsFromUtf8(_in_text: c_ptr, _in_text_end: c_ptr): c_int { throw 0; });
const _igImTextCountUtf8BytesFromChar = $SHBuiltin.extern_c({}, function igImTextCountUtf8BytesFromChar(_in_text: c_ptr, _in_text_end: c_ptr): c_int { throw 0; });
const _igImTextCountUtf8BytesFromStr = $SHBuiltin.extern_c({}, function igImTextCountUtf8BytesFromStr(_in_text: c_ptr, _in_text_end: c_ptr): c_int { throw 0; });
const _igImFileOpen = $SHBuiltin.extern_c({}, function igImFileOpen(_filename: c_ptr, _mode: c_ptr): c_ptr { throw 0; });
const _igImFileClose = $SHBuiltin.extern_c({}, function igImFileClose(_file: c_ptr): c_bool { throw 0; });
const _igImFileGetSize = $SHBuiltin.extern_c({}, function igImFileGetSize(_file: c_ptr): c_ulonglong { throw 0; });
const _igImFileRead = $SHBuiltin.extern_c({}, function igImFileRead(_data: c_ptr, _size: c_ulonglong, _count: c_ulonglong, _file: c_ptr): c_ulonglong { throw 0; });
const _igImFileWrite = $SHBuiltin.extern_c({}, function igImFileWrite(_data: c_ptr, _size: c_ulonglong, _count: c_ulonglong, _file: c_ptr): c_ulonglong { throw 0; });
const _igImFileLoadToMemory = $SHBuiltin.extern_c({}, function igImFileLoadToMemory(_filename: c_ptr, _mode: c_ptr, _out_file_size: c_ptr, _padding_bytes: c_int): c_ptr { throw 0; });
const _igImPow_Float = $SHBuiltin.extern_c({}, function igImPow_Float(_x: c_float, _y: c_float): c_float { throw 0; });
const _igImPow_double = $SHBuiltin.extern_c({}, function igImPow_double(_x: c_double, _y: c_double): c_double { throw 0; });
const _igImLog_Float = $SHBuiltin.extern_c({}, function igImLog_Float(_x: c_float): c_float { throw 0; });
const _igImLog_double = $SHBuiltin.extern_c({}, function igImLog_double(_x: c_double): c_double { throw 0; });
const _igImAbs_Int = $SHBuiltin.extern_c({}, function igImAbs_Int(_x: c_int): c_int { throw 0; });
const _igImAbs_Float = $SHBuiltin.extern_c({}, function igImAbs_Float(_x: c_float): c_float { throw 0; });
const _igImAbs_double = $SHBuiltin.extern_c({}, function igImAbs_double(_x: c_double): c_double { throw 0; });
const _igImSign_Float = $SHBuiltin.extern_c({}, function igImSign_Float(_x: c_float): c_float { throw 0; });
const _igImSign_double = $SHBuiltin.extern_c({}, function igImSign_double(_x: c_double): c_double { throw 0; });
const _igImRsqrt_Float = $SHBuiltin.extern_c({}, function igImRsqrt_Float(_x: c_float): c_float { throw 0; });
const _igImRsqrt_double = $SHBuiltin.extern_c({}, function igImRsqrt_double(_x: c_double): c_double { throw 0; });
const _igImMin = $SHBuiltin.extern_c({}, function igImMin_cwrap(_pOut: c_ptr, _lhs: c_ptr, _rhs: c_ptr): void { throw 0; });
const _igImMax = $SHBuiltin.extern_c({}, function igImMax_cwrap(_pOut: c_ptr, _lhs: c_ptr, _rhs: c_ptr): void { throw 0; });
const _igImClamp = $SHBuiltin.extern_c({}, function igImClamp_cwrap(_pOut: c_ptr, _v: c_ptr, _mn: c_ptr, _mx: c_ptr): void { throw 0; });
const _igImLerp_Vec2Float = $SHBuiltin.extern_c({}, function igImLerp_Vec2Float_cwrap(_pOut: c_ptr, _a: c_ptr, _b: c_ptr, _t: c_float): void { throw 0; });
const _igImLerp_Vec2Vec2 = $SHBuiltin.extern_c({}, function igImLerp_Vec2Vec2_cwrap(_pOut: c_ptr, _a: c_ptr, _b: c_ptr, _t: c_ptr): void { throw 0; });
const _igImLerp_Vec4 = $SHBuiltin.extern_c({}, function igImLerp_Vec4_cwrap(_pOut: c_ptr, _a: c_ptr, _b: c_ptr, _t: c_float): void { throw 0; });
const _igImSaturate = $SHBuiltin.extern_c({}, function igImSaturate(_f: c_float): c_float { throw 0; });
const _igImLengthSqr_Vec2 = $SHBuiltin.extern_c({}, function igImLengthSqr_Vec2_cwrap(_lhs: c_ptr): c_float { throw 0; });
const _igImLengthSqr_Vec4 = $SHBuiltin.extern_c({}, function igImLengthSqr_Vec4_cwrap(_lhs: c_ptr): c_float { throw 0; });
const _igImInvLength = $SHBuiltin.extern_c({}, function igImInvLength_cwrap(_lhs: c_ptr, _fail_value: c_float): c_float { throw 0; });
const _igImFloor_Float = $SHBuiltin.extern_c({}, function igImFloor_Float(_f: c_float): c_float { throw 0; });
const _igImFloorSigned_Float = $SHBuiltin.extern_c({}, function igImFloorSigned_Float(_f: c_float): c_float { throw 0; });
const _igImFloor_Vec2 = $SHBuiltin.extern_c({}, function igImFloor_Vec2_cwrap(_pOut: c_ptr, _v: c_ptr): void { throw 0; });
const _igImFloorSigned_Vec2 = $SHBuiltin.extern_c({}, function igImFloorSigned_Vec2_cwrap(_pOut: c_ptr, _v: c_ptr): void { throw 0; });
const _igImModPositive = $SHBuiltin.extern_c({}, function igImModPositive(_a: c_int, _b: c_int): c_int { throw 0; });
const _igImDot = $SHBuiltin.extern_c({}, function igImDot_cwrap(_a: c_ptr, _b: c_ptr): c_float { throw 0; });
const _igImRotate = $SHBuiltin.extern_c({}, function igImRotate_cwrap(_pOut: c_ptr, _v: c_ptr, _cos_a: c_float, _sin_a: c_float): void { throw 0; });
const _igImLinearSweep = $SHBuiltin.extern_c({}, function igImLinearSweep(_current: c_float, _target: c_float, _speed: c_float): c_float { throw 0; });
const _igImMul = $SHBuiltin.extern_c({}, function igImMul_cwrap(_pOut: c_ptr, _lhs: c_ptr, _rhs: c_ptr): void { throw 0; });
const _igImIsFloatAboveGuaranteedIntegerPrecision = $SHBuiltin.extern_c({}, function igImIsFloatAboveGuaranteedIntegerPrecision(_f: c_float): c_bool { throw 0; });
const _igImExponentialMovingAverage = $SHBuiltin.extern_c({}, function igImExponentialMovingAverage(_avg: c_float, _sample: c_float, _n: c_int): c_float { throw 0; });
const _igImBezierCubicCalc = $SHBuiltin.extern_c({}, function igImBezierCubicCalc_cwrap(_pOut: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _p4: c_ptr, _t: c_float): void { throw 0; });
const _igImBezierCubicClosestPoint = $SHBuiltin.extern_c({}, function igImBezierCubicClosestPoint_cwrap(_pOut: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _p4: c_ptr, _p: c_ptr, _num_segments: c_int): void { throw 0; });
const _igImBezierCubicClosestPointCasteljau = $SHBuiltin.extern_c({}, function igImBezierCubicClosestPointCasteljau_cwrap(_pOut: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _p4: c_ptr, _p: c_ptr, _tess_tol: c_float): void { throw 0; });
const _igImBezierQuadraticCalc = $SHBuiltin.extern_c({}, function igImBezierQuadraticCalc_cwrap(_pOut: c_ptr, _p1: c_ptr, _p2: c_ptr, _p3: c_ptr, _t: c_float): void { throw 0; });
const _igImLineClosestPoint = $SHBuiltin.extern_c({}, function igImLineClosestPoint_cwrap(_pOut: c_ptr, _a: c_ptr, _b: c_ptr, _p: c_ptr): void { throw 0; });
const _igImTriangleContainsPoint = $SHBuiltin.extern_c({}, function igImTriangleContainsPoint_cwrap(_a: c_ptr, _b: c_ptr, _c: c_ptr, _p: c_ptr): c_bool { throw 0; });
const _igImTriangleClosestPoint = $SHBuiltin.extern_c({}, function igImTriangleClosestPoint_cwrap(_pOut: c_ptr, _a: c_ptr, _b: c_ptr, _c: c_ptr, _p: c_ptr): void { throw 0; });
const _igImTriangleBarycentricCoords = $SHBuiltin.extern_c({}, function igImTriangleBarycentricCoords_cwrap(_a: c_ptr, _b: c_ptr, _c: c_ptr, _p: c_ptr, _out_u: c_ptr, _out_v: c_ptr, _out_w: c_ptr): void { throw 0; });
const _igImTriangleArea = $SHBuiltin.extern_c({}, function igImTriangleArea_cwrap(_a: c_ptr, _b: c_ptr, _c: c_ptr): c_float { throw 0; });
const _ImVec1_ImVec1_Nil = $SHBuiltin.extern_c({}, function ImVec1_ImVec1_Nil(): c_ptr { throw 0; });
const _ImVec1_destroy = $SHBuiltin.extern_c({}, function ImVec1_destroy(_self: c_ptr): void { throw 0; });
const _ImVec1_ImVec1_Float = $SHBuiltin.extern_c({}, function ImVec1_ImVec1_Float(__x: c_float): c_ptr { throw 0; });
const _ImVec2ih_ImVec2ih_Nil = $SHBuiltin.extern_c({}, function ImVec2ih_ImVec2ih_Nil(): c_ptr { throw 0; });
const _ImVec2ih_destroy = $SHBuiltin.extern_c({}, function ImVec2ih_destroy(_self: c_ptr): void { throw 0; });
const _ImVec2ih_ImVec2ih_short = $SHBuiltin.extern_c({}, function ImVec2ih_ImVec2ih_short(__x: c_short, __y: c_short): c_ptr { throw 0; });
const _ImVec2ih_ImVec2ih_Vec2 = $SHBuiltin.extern_c({}, function ImVec2ih_ImVec2ih_Vec2_cwrap(_rhs: c_ptr): c_ptr { throw 0; });
const _ImRect_ImRect_Nil = $SHBuiltin.extern_c({}, function ImRect_ImRect_Nil(): c_ptr { throw 0; });
const _ImRect_destroy = $SHBuiltin.extern_c({}, function ImRect_destroy(_self: c_ptr): void { throw 0; });
const _ImRect_ImRect_Vec2 = $SHBuiltin.extern_c({}, function ImRect_ImRect_Vec2_cwrap(_min: c_ptr, _max: c_ptr): c_ptr { throw 0; });
const _ImRect_ImRect_Vec4 = $SHBuiltin.extern_c({}, function ImRect_ImRect_Vec4_cwrap(_v: c_ptr): c_ptr { throw 0; });
const _ImRect_ImRect_Float = $SHBuiltin.extern_c({}, function ImRect_ImRect_Float(_x1: c_float, _y1: c_float, _x2: c_float, _y2: c_float): c_ptr { throw 0; });
const _ImRect_GetCenter = $SHBuiltin.extern_c({}, function ImRect_GetCenter(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImRect_GetSize = $SHBuiltin.extern_c({}, function ImRect_GetSize(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImRect_GetWidth = $SHBuiltin.extern_c({}, function ImRect_GetWidth(_self: c_ptr): c_float { throw 0; });
const _ImRect_GetHeight = $SHBuiltin.extern_c({}, function ImRect_GetHeight(_self: c_ptr): c_float { throw 0; });
const _ImRect_GetArea = $SHBuiltin.extern_c({}, function ImRect_GetArea(_self: c_ptr): c_float { throw 0; });
const _ImRect_GetTL = $SHBuiltin.extern_c({}, function ImRect_GetTL(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImRect_GetTR = $SHBuiltin.extern_c({}, function ImRect_GetTR(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImRect_GetBL = $SHBuiltin.extern_c({}, function ImRect_GetBL(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImRect_GetBR = $SHBuiltin.extern_c({}, function ImRect_GetBR(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImRect_Contains_Vec2 = $SHBuiltin.extern_c({}, function ImRect_Contains_Vec2_cwrap(_self: c_ptr, _p: c_ptr): c_bool { throw 0; });
const _ImRect_Contains_Rect = $SHBuiltin.extern_c({}, function ImRect_Contains_Rect_cwrap(_self: c_ptr, _r: c_ptr): c_bool { throw 0; });
const _ImRect_Overlaps = $SHBuiltin.extern_c({}, function ImRect_Overlaps_cwrap(_self: c_ptr, _r: c_ptr): c_bool { throw 0; });
const _ImRect_Add_Vec2 = $SHBuiltin.extern_c({}, function ImRect_Add_Vec2_cwrap(_self: c_ptr, _p: c_ptr): void { throw 0; });
const _ImRect_Add_Rect = $SHBuiltin.extern_c({}, function ImRect_Add_Rect_cwrap(_self: c_ptr, _r: c_ptr): void { throw 0; });
const _ImRect_Expand_Float = $SHBuiltin.extern_c({}, function ImRect_Expand_Float(_self: c_ptr, _amount: c_float): void { throw 0; });
const _ImRect_Expand_Vec2 = $SHBuiltin.extern_c({}, function ImRect_Expand_Vec2_cwrap(_self: c_ptr, _amount: c_ptr): void { throw 0; });
const _ImRect_Translate = $SHBuiltin.extern_c({}, function ImRect_Translate_cwrap(_self: c_ptr, _d: c_ptr): void { throw 0; });
const _ImRect_TranslateX = $SHBuiltin.extern_c({}, function ImRect_TranslateX(_self: c_ptr, _dx: c_float): void { throw 0; });
const _ImRect_TranslateY = $SHBuiltin.extern_c({}, function ImRect_TranslateY(_self: c_ptr, _dy: c_float): void { throw 0; });
const _ImRect_ClipWith = $SHBuiltin.extern_c({}, function ImRect_ClipWith_cwrap(_self: c_ptr, _r: c_ptr): void { throw 0; });
const _ImRect_ClipWithFull = $SHBuiltin.extern_c({}, function ImRect_ClipWithFull_cwrap(_self: c_ptr, _r: c_ptr): void { throw 0; });
const _ImRect_Floor = $SHBuiltin.extern_c({}, function ImRect_Floor(_self: c_ptr): void { throw 0; });
const _ImRect_IsInverted = $SHBuiltin.extern_c({}, function ImRect_IsInverted(_self: c_ptr): c_bool { throw 0; });
const _ImRect_ToVec4 = $SHBuiltin.extern_c({}, function ImRect_ToVec4(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _igImBitArrayGetStorageSizeInBytes = $SHBuiltin.extern_c({}, function igImBitArrayGetStorageSizeInBytes(_bitcount: c_int): c_ulong { throw 0; });
const _igImBitArrayClearAllBits = $SHBuiltin.extern_c({}, function igImBitArrayClearAllBits(_arr: c_ptr, _bitcount: c_int): void { throw 0; });
const _igImBitArrayTestBit = $SHBuiltin.extern_c({}, function igImBitArrayTestBit(_arr: c_ptr, _n: c_int): c_bool { throw 0; });
const _igImBitArrayClearBit = $SHBuiltin.extern_c({}, function igImBitArrayClearBit(_arr: c_ptr, _n: c_int): void { throw 0; });
const _igImBitArraySetBit = $SHBuiltin.extern_c({}, function igImBitArraySetBit(_arr: c_ptr, _n: c_int): void { throw 0; });
const _igImBitArraySetBitRange = $SHBuiltin.extern_c({}, function igImBitArraySetBitRange(_arr: c_ptr, _n: c_int, _n2: c_int): void { throw 0; });
const _ImBitVector_Create = $SHBuiltin.extern_c({}, function ImBitVector_Create(_self: c_ptr, _sz: c_int): void { throw 0; });
const _ImBitVector_Clear = $SHBuiltin.extern_c({}, function ImBitVector_Clear(_self: c_ptr): void { throw 0; });
const _ImBitVector_TestBit = $SHBuiltin.extern_c({}, function ImBitVector_TestBit(_self: c_ptr, _n: c_int): c_bool { throw 0; });
const _ImBitVector_SetBit = $SHBuiltin.extern_c({}, function ImBitVector_SetBit(_self: c_ptr, _n: c_int): void { throw 0; });
const _ImBitVector_ClearBit = $SHBuiltin.extern_c({}, function ImBitVector_ClearBit(_self: c_ptr, _n: c_int): void { throw 0; });
const _ImGuiTextIndex_clear = $SHBuiltin.extern_c({}, function ImGuiTextIndex_clear(_self: c_ptr): void { throw 0; });
const _ImGuiTextIndex_size = $SHBuiltin.extern_c({}, function ImGuiTextIndex_size(_self: c_ptr): c_int { throw 0; });
const _ImGuiTextIndex_get_line_begin = $SHBuiltin.extern_c({}, function ImGuiTextIndex_get_line_begin(_self: c_ptr, _base: c_ptr, _n: c_int): c_ptr { throw 0; });
const _ImGuiTextIndex_get_line_end = $SHBuiltin.extern_c({}, function ImGuiTextIndex_get_line_end(_self: c_ptr, _base: c_ptr, _n: c_int): c_ptr { throw 0; });
const _ImGuiTextIndex_append = $SHBuiltin.extern_c({}, function ImGuiTextIndex_append(_self: c_ptr, _base: c_ptr, _old_size: c_int, _new_size: c_int): void { throw 0; });
const _ImDrawListSharedData_ImDrawListSharedData = $SHBuiltin.extern_c({}, function ImDrawListSharedData_ImDrawListSharedData(): c_ptr { throw 0; });
const _ImDrawListSharedData_destroy = $SHBuiltin.extern_c({}, function ImDrawListSharedData_destroy(_self: c_ptr): void { throw 0; });
const _ImDrawListSharedData_SetCircleTessellationMaxError = $SHBuiltin.extern_c({}, function ImDrawListSharedData_SetCircleTessellationMaxError(_self: c_ptr, _max_error: c_float): void { throw 0; });
const _ImDrawDataBuilder_ImDrawDataBuilder = $SHBuiltin.extern_c({}, function ImDrawDataBuilder_ImDrawDataBuilder(): c_ptr { throw 0; });
const _ImDrawDataBuilder_destroy = $SHBuiltin.extern_c({}, function ImDrawDataBuilder_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiDataVarInfo_GetVarPtr = $SHBuiltin.extern_c({}, function ImGuiDataVarInfo_GetVarPtr(_self: c_ptr, _parent: c_ptr): c_ptr { throw 0; });
const _ImGuiStyleMod_ImGuiStyleMod_Int = $SHBuiltin.extern_c({}, function ImGuiStyleMod_ImGuiStyleMod_Int(_idx: c_int, _v: c_int): c_ptr { throw 0; });
const _ImGuiStyleMod_destroy = $SHBuiltin.extern_c({}, function ImGuiStyleMod_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiStyleMod_ImGuiStyleMod_Float = $SHBuiltin.extern_c({}, function ImGuiStyleMod_ImGuiStyleMod_Float(_idx: c_int, _v: c_float): c_ptr { throw 0; });
const _ImGuiStyleMod_ImGuiStyleMod_Vec2 = $SHBuiltin.extern_c({}, function ImGuiStyleMod_ImGuiStyleMod_Vec2_cwrap(_idx: c_int, _v: c_ptr): c_ptr { throw 0; });
const _ImGuiComboPreviewData_ImGuiComboPreviewData = $SHBuiltin.extern_c({}, function ImGuiComboPreviewData_ImGuiComboPreviewData(): c_ptr { throw 0; });
const _ImGuiComboPreviewData_destroy = $SHBuiltin.extern_c({}, function ImGuiComboPreviewData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiMenuColumns_ImGuiMenuColumns = $SHBuiltin.extern_c({}, function ImGuiMenuColumns_ImGuiMenuColumns(): c_ptr { throw 0; });
const _ImGuiMenuColumns_destroy = $SHBuiltin.extern_c({}, function ImGuiMenuColumns_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiMenuColumns_Update = $SHBuiltin.extern_c({}, function ImGuiMenuColumns_Update(_self: c_ptr, _spacing: c_float, _window_reappearing: c_bool): void { throw 0; });
const _ImGuiMenuColumns_DeclColumns = $SHBuiltin.extern_c({}, function ImGuiMenuColumns_DeclColumns(_self: c_ptr, _w_icon: c_float, _w_label: c_float, _w_shortcut: c_float, _w_mark: c_float): c_float { throw 0; });
const _ImGuiMenuColumns_CalcNextTotalWidth = $SHBuiltin.extern_c({}, function ImGuiMenuColumns_CalcNextTotalWidth(_self: c_ptr, _update_offsets: c_bool): void { throw 0; });
const _ImGuiInputTextDeactivatedState_ImGuiInputTextDeactivatedState = $SHBuiltin.extern_c({}, function ImGuiInputTextDeactivatedState_ImGuiInputTextDeactivatedState(): c_ptr { throw 0; });
const _ImGuiInputTextDeactivatedState_destroy = $SHBuiltin.extern_c({}, function ImGuiInputTextDeactivatedState_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextDeactivatedState_ClearFreeMemory = $SHBuiltin.extern_c({}, function ImGuiInputTextDeactivatedState_ClearFreeMemory(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextState_ImGuiInputTextState = $SHBuiltin.extern_c({}, function ImGuiInputTextState_ImGuiInputTextState(): c_ptr { throw 0; });
const _ImGuiInputTextState_destroy = $SHBuiltin.extern_c({}, function ImGuiInputTextState_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextState_ClearText = $SHBuiltin.extern_c({}, function ImGuiInputTextState_ClearText(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextState_ClearFreeMemory = $SHBuiltin.extern_c({}, function ImGuiInputTextState_ClearFreeMemory(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextState_GetUndoAvailCount = $SHBuiltin.extern_c({}, function ImGuiInputTextState_GetUndoAvailCount(_self: c_ptr): c_int { throw 0; });
const _ImGuiInputTextState_GetRedoAvailCount = $SHBuiltin.extern_c({}, function ImGuiInputTextState_GetRedoAvailCount(_self: c_ptr): c_int { throw 0; });
const _ImGuiInputTextState_OnKeyPressed = $SHBuiltin.extern_c({}, function ImGuiInputTextState_OnKeyPressed(_self: c_ptr, _key: c_int): void { throw 0; });
const _ImGuiInputTextState_CursorAnimReset = $SHBuiltin.extern_c({}, function ImGuiInputTextState_CursorAnimReset(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextState_CursorClamp = $SHBuiltin.extern_c({}, function ImGuiInputTextState_CursorClamp(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextState_HasSelection = $SHBuiltin.extern_c({}, function ImGuiInputTextState_HasSelection(_self: c_ptr): c_bool { throw 0; });
const _ImGuiInputTextState_ClearSelection = $SHBuiltin.extern_c({}, function ImGuiInputTextState_ClearSelection(_self: c_ptr): void { throw 0; });
const _ImGuiInputTextState_GetCursorPos = $SHBuiltin.extern_c({}, function ImGuiInputTextState_GetCursorPos(_self: c_ptr): c_int { throw 0; });
const _ImGuiInputTextState_GetSelectionStart = $SHBuiltin.extern_c({}, function ImGuiInputTextState_GetSelectionStart(_self: c_ptr): c_int { throw 0; });
const _ImGuiInputTextState_GetSelectionEnd = $SHBuiltin.extern_c({}, function ImGuiInputTextState_GetSelectionEnd(_self: c_ptr): c_int { throw 0; });
const _ImGuiInputTextState_SelectAll = $SHBuiltin.extern_c({}, function ImGuiInputTextState_SelectAll(_self: c_ptr): void { throw 0; });
const _ImGuiPopupData_ImGuiPopupData = $SHBuiltin.extern_c({}, function ImGuiPopupData_ImGuiPopupData(): c_ptr { throw 0; });
const _ImGuiPopupData_destroy = $SHBuiltin.extern_c({}, function ImGuiPopupData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiNextWindowData_ImGuiNextWindowData = $SHBuiltin.extern_c({}, function ImGuiNextWindowData_ImGuiNextWindowData(): c_ptr { throw 0; });
const _ImGuiNextWindowData_destroy = $SHBuiltin.extern_c({}, function ImGuiNextWindowData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiNextWindowData_ClearFlags = $SHBuiltin.extern_c({}, function ImGuiNextWindowData_ClearFlags(_self: c_ptr): void { throw 0; });
const _ImGuiNextItemData_ImGuiNextItemData = $SHBuiltin.extern_c({}, function ImGuiNextItemData_ImGuiNextItemData(): c_ptr { throw 0; });
const _ImGuiNextItemData_destroy = $SHBuiltin.extern_c({}, function ImGuiNextItemData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiNextItemData_ClearFlags = $SHBuiltin.extern_c({}, function ImGuiNextItemData_ClearFlags(_self: c_ptr): void { throw 0; });
const _ImGuiLastItemData_ImGuiLastItemData = $SHBuiltin.extern_c({}, function ImGuiLastItemData_ImGuiLastItemData(): c_ptr { throw 0; });
const _ImGuiLastItemData_destroy = $SHBuiltin.extern_c({}, function ImGuiLastItemData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiStackSizes_ImGuiStackSizes = $SHBuiltin.extern_c({}, function ImGuiStackSizes_ImGuiStackSizes(): c_ptr { throw 0; });
const _ImGuiStackSizes_destroy = $SHBuiltin.extern_c({}, function ImGuiStackSizes_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiStackSizes_SetToContextState = $SHBuiltin.extern_c({}, function ImGuiStackSizes_SetToContextState(_self: c_ptr, _ctx: c_ptr): void { throw 0; });
const _ImGuiStackSizes_CompareWithContextState = $SHBuiltin.extern_c({}, function ImGuiStackSizes_CompareWithContextState(_self: c_ptr, _ctx: c_ptr): void { throw 0; });
const _ImGuiPtrOrIndex_ImGuiPtrOrIndex_Ptr = $SHBuiltin.extern_c({}, function ImGuiPtrOrIndex_ImGuiPtrOrIndex_Ptr(_ptr: c_ptr): c_ptr { throw 0; });
const _ImGuiPtrOrIndex_destroy = $SHBuiltin.extern_c({}, function ImGuiPtrOrIndex_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiPtrOrIndex_ImGuiPtrOrIndex_Int = $SHBuiltin.extern_c({}, function ImGuiPtrOrIndex_ImGuiPtrOrIndex_Int(_index: c_int): c_ptr { throw 0; });
const _ImGuiInputEvent_ImGuiInputEvent = $SHBuiltin.extern_c({}, function ImGuiInputEvent_ImGuiInputEvent(): c_ptr { throw 0; });
const _ImGuiInputEvent_destroy = $SHBuiltin.extern_c({}, function ImGuiInputEvent_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiKeyRoutingData_ImGuiKeyRoutingData = $SHBuiltin.extern_c({}, function ImGuiKeyRoutingData_ImGuiKeyRoutingData(): c_ptr { throw 0; });
const _ImGuiKeyRoutingData_destroy = $SHBuiltin.extern_c({}, function ImGuiKeyRoutingData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiKeyRoutingTable_ImGuiKeyRoutingTable = $SHBuiltin.extern_c({}, function ImGuiKeyRoutingTable_ImGuiKeyRoutingTable(): c_ptr { throw 0; });
const _ImGuiKeyRoutingTable_destroy = $SHBuiltin.extern_c({}, function ImGuiKeyRoutingTable_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiKeyRoutingTable_Clear = $SHBuiltin.extern_c({}, function ImGuiKeyRoutingTable_Clear(_self: c_ptr): void { throw 0; });
const _ImGuiKeyOwnerData_ImGuiKeyOwnerData = $SHBuiltin.extern_c({}, function ImGuiKeyOwnerData_ImGuiKeyOwnerData(): c_ptr { throw 0; });
const _ImGuiKeyOwnerData_destroy = $SHBuiltin.extern_c({}, function ImGuiKeyOwnerData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiListClipperRange_FromIndices = $SHBuiltin.extern_c({}, function ImGuiListClipperRange_FromIndices_cwrap(_out: c_ptr, _min: c_int, _max: c_int): void { throw 0; });
const _ImGuiListClipperRange_FromPositions = $SHBuiltin.extern_c({}, function ImGuiListClipperRange_FromPositions_cwrap(_out: c_ptr, _y1: c_float, _y2: c_float, _off_min: c_int, _off_max: c_int): void { throw 0; });
const _ImGuiListClipperData_ImGuiListClipperData = $SHBuiltin.extern_c({}, function ImGuiListClipperData_ImGuiListClipperData(): c_ptr { throw 0; });
const _ImGuiListClipperData_destroy = $SHBuiltin.extern_c({}, function ImGuiListClipperData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiListClipperData_Reset = $SHBuiltin.extern_c({}, function ImGuiListClipperData_Reset(_self: c_ptr, _clipper: c_ptr): void { throw 0; });
const _ImGuiNavItemData_ImGuiNavItemData = $SHBuiltin.extern_c({}, function ImGuiNavItemData_ImGuiNavItemData(): c_ptr { throw 0; });
const _ImGuiNavItemData_destroy = $SHBuiltin.extern_c({}, function ImGuiNavItemData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiNavItemData_Clear = $SHBuiltin.extern_c({}, function ImGuiNavItemData_Clear(_self: c_ptr): void { throw 0; });
const _ImGuiOldColumnData_ImGuiOldColumnData = $SHBuiltin.extern_c({}, function ImGuiOldColumnData_ImGuiOldColumnData(): c_ptr { throw 0; });
const _ImGuiOldColumnData_destroy = $SHBuiltin.extern_c({}, function ImGuiOldColumnData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiOldColumns_ImGuiOldColumns = $SHBuiltin.extern_c({}, function ImGuiOldColumns_ImGuiOldColumns(): c_ptr { throw 0; });
const _ImGuiOldColumns_destroy = $SHBuiltin.extern_c({}, function ImGuiOldColumns_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiViewportP_ImGuiViewportP = $SHBuiltin.extern_c({}, function ImGuiViewportP_ImGuiViewportP(): c_ptr { throw 0; });
const _ImGuiViewportP_destroy = $SHBuiltin.extern_c({}, function ImGuiViewportP_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiViewportP_CalcWorkRectPos = $SHBuiltin.extern_c({}, function ImGuiViewportP_CalcWorkRectPos_cwrap(_pOut: c_ptr, _self: c_ptr, _off_min: c_ptr): void { throw 0; });
const _ImGuiViewportP_CalcWorkRectSize = $SHBuiltin.extern_c({}, function ImGuiViewportP_CalcWorkRectSize_cwrap(_pOut: c_ptr, _self: c_ptr, _off_min: c_ptr, _off_max: c_ptr): void { throw 0; });
const _ImGuiViewportP_UpdateWorkRect = $SHBuiltin.extern_c({}, function ImGuiViewportP_UpdateWorkRect(_self: c_ptr): void { throw 0; });
const _ImGuiViewportP_GetMainRect = $SHBuiltin.extern_c({}, function ImGuiViewportP_GetMainRect(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImGuiViewportP_GetWorkRect = $SHBuiltin.extern_c({}, function ImGuiViewportP_GetWorkRect(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImGuiViewportP_GetBuildWorkRect = $SHBuiltin.extern_c({}, function ImGuiViewportP_GetBuildWorkRect(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImGuiWindowSettings_ImGuiWindowSettings = $SHBuiltin.extern_c({}, function ImGuiWindowSettings_ImGuiWindowSettings(): c_ptr { throw 0; });
const _ImGuiWindowSettings_destroy = $SHBuiltin.extern_c({}, function ImGuiWindowSettings_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiWindowSettings_GetName = $SHBuiltin.extern_c({}, function ImGuiWindowSettings_GetName(_self: c_ptr): c_ptr { throw 0; });
const _ImGuiSettingsHandler_ImGuiSettingsHandler = $SHBuiltin.extern_c({}, function ImGuiSettingsHandler_ImGuiSettingsHandler(): c_ptr { throw 0; });
const _ImGuiSettingsHandler_destroy = $SHBuiltin.extern_c({}, function ImGuiSettingsHandler_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiStackLevelInfo_ImGuiStackLevelInfo = $SHBuiltin.extern_c({}, function ImGuiStackLevelInfo_ImGuiStackLevelInfo(): c_ptr { throw 0; });
const _ImGuiStackLevelInfo_destroy = $SHBuiltin.extern_c({}, function ImGuiStackLevelInfo_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiStackTool_ImGuiStackTool = $SHBuiltin.extern_c({}, function ImGuiStackTool_ImGuiStackTool(): c_ptr { throw 0; });
const _ImGuiStackTool_destroy = $SHBuiltin.extern_c({}, function ImGuiStackTool_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiContextHook_ImGuiContextHook = $SHBuiltin.extern_c({}, function ImGuiContextHook_ImGuiContextHook(): c_ptr { throw 0; });
const _ImGuiContextHook_destroy = $SHBuiltin.extern_c({}, function ImGuiContextHook_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiContext_ImGuiContext = $SHBuiltin.extern_c({}, function ImGuiContext_ImGuiContext(_shared_font_atlas: c_ptr): c_ptr { throw 0; });
const _ImGuiContext_destroy = $SHBuiltin.extern_c({}, function ImGuiContext_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiWindow_ImGuiWindow = $SHBuiltin.extern_c({}, function ImGuiWindow_ImGuiWindow(_context: c_ptr, _name: c_ptr): c_ptr { throw 0; });
const _ImGuiWindow_destroy = $SHBuiltin.extern_c({}, function ImGuiWindow_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiWindow_GetID_Str = $SHBuiltin.extern_c({}, function ImGuiWindow_GetID_Str(_self: c_ptr, _str: c_ptr, _str_end: c_ptr): c_uint { throw 0; });
const _ImGuiWindow_GetID_Ptr = $SHBuiltin.extern_c({}, function ImGuiWindow_GetID_Ptr(_self: c_ptr, _ptr: c_ptr): c_uint { throw 0; });
const _ImGuiWindow_GetID_Int = $SHBuiltin.extern_c({}, function ImGuiWindow_GetID_Int(_self: c_ptr, _n: c_int): c_uint { throw 0; });
const _ImGuiWindow_GetIDFromRectangle = $SHBuiltin.extern_c({}, function ImGuiWindow_GetIDFromRectangle_cwrap(_self: c_ptr, _r_abs: c_ptr): c_uint { throw 0; });
const _ImGuiWindow_Rect = $SHBuiltin.extern_c({}, function ImGuiWindow_Rect(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImGuiWindow_CalcFontSize = $SHBuiltin.extern_c({}, function ImGuiWindow_CalcFontSize(_self: c_ptr): c_float { throw 0; });
const _ImGuiWindow_TitleBarHeight = $SHBuiltin.extern_c({}, function ImGuiWindow_TitleBarHeight(_self: c_ptr): c_float { throw 0; });
const _ImGuiWindow_TitleBarRect = $SHBuiltin.extern_c({}, function ImGuiWindow_TitleBarRect(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImGuiWindow_MenuBarHeight = $SHBuiltin.extern_c({}, function ImGuiWindow_MenuBarHeight(_self: c_ptr): c_float { throw 0; });
const _ImGuiWindow_MenuBarRect = $SHBuiltin.extern_c({}, function ImGuiWindow_MenuBarRect(_pOut: c_ptr, _self: c_ptr): void { throw 0; });
const _ImGuiTabItem_ImGuiTabItem = $SHBuiltin.extern_c({}, function ImGuiTabItem_ImGuiTabItem(): c_ptr { throw 0; });
const _ImGuiTabItem_destroy = $SHBuiltin.extern_c({}, function ImGuiTabItem_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTabBar_ImGuiTabBar = $SHBuiltin.extern_c({}, function ImGuiTabBar_ImGuiTabBar(): c_ptr { throw 0; });
const _ImGuiTabBar_destroy = $SHBuiltin.extern_c({}, function ImGuiTabBar_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTableColumn_ImGuiTableColumn = $SHBuiltin.extern_c({}, function ImGuiTableColumn_ImGuiTableColumn(): c_ptr { throw 0; });
const _ImGuiTableColumn_destroy = $SHBuiltin.extern_c({}, function ImGuiTableColumn_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTableInstanceData_ImGuiTableInstanceData = $SHBuiltin.extern_c({}, function ImGuiTableInstanceData_ImGuiTableInstanceData(): c_ptr { throw 0; });
const _ImGuiTableInstanceData_destroy = $SHBuiltin.extern_c({}, function ImGuiTableInstanceData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTable_ImGuiTable = $SHBuiltin.extern_c({}, function ImGuiTable_ImGuiTable(): c_ptr { throw 0; });
const _ImGuiTable_destroy = $SHBuiltin.extern_c({}, function ImGuiTable_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTableTempData_ImGuiTableTempData = $SHBuiltin.extern_c({}, function ImGuiTableTempData_ImGuiTableTempData(): c_ptr { throw 0; });
const _ImGuiTableTempData_destroy = $SHBuiltin.extern_c({}, function ImGuiTableTempData_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTableColumnSettings_ImGuiTableColumnSettings = $SHBuiltin.extern_c({}, function ImGuiTableColumnSettings_ImGuiTableColumnSettings(): c_ptr { throw 0; });
const _ImGuiTableColumnSettings_destroy = $SHBuiltin.extern_c({}, function ImGuiTableColumnSettings_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTableSettings_ImGuiTableSettings = $SHBuiltin.extern_c({}, function ImGuiTableSettings_ImGuiTableSettings(): c_ptr { throw 0; });
const _ImGuiTableSettings_destroy = $SHBuiltin.extern_c({}, function ImGuiTableSettings_destroy(_self: c_ptr): void { throw 0; });
const _ImGuiTableSettings_GetColumnSettings = $SHBuiltin.extern_c({}, function ImGuiTableSettings_GetColumnSettings(_self: c_ptr): c_ptr { throw 0; });
const _igGetCurrentWindowRead = $SHBuiltin.extern_c({}, function igGetCurrentWindowRead(): c_ptr { throw 0; });
const _igGetCurrentWindow = $SHBuiltin.extern_c({}, function igGetCurrentWindow(): c_ptr { throw 0; });
const _igFindWindowByID = $SHBuiltin.extern_c({}, function igFindWindowByID(_id: c_uint): c_ptr { throw 0; });
const _igFindWindowByName = $SHBuiltin.extern_c({}, function igFindWindowByName(_name: c_ptr): c_ptr { throw 0; });
const _igUpdateWindowParentAndRootLinks = $SHBuiltin.extern_c({}, function igUpdateWindowParentAndRootLinks(_window: c_ptr, _flags: c_int, _parent_window: c_ptr): void { throw 0; });
const _igCalcWindowNextAutoFitSize = $SHBuiltin.extern_c({}, function igCalcWindowNextAutoFitSize(_pOut: c_ptr, _window: c_ptr): void { throw 0; });
const _igIsWindowChildOf = $SHBuiltin.extern_c({}, function igIsWindowChildOf(_window: c_ptr, _potential_parent: c_ptr, _popup_hierarchy: c_bool): c_bool { throw 0; });
const _igIsWindowWithinBeginStackOf = $SHBuiltin.extern_c({}, function igIsWindowWithinBeginStackOf(_window: c_ptr, _potential_parent: c_ptr): c_bool { throw 0; });
const _igIsWindowAbove = $SHBuiltin.extern_c({}, function igIsWindowAbove(_potential_above: c_ptr, _potential_below: c_ptr): c_bool { throw 0; });
const _igIsWindowNavFocusable = $SHBuiltin.extern_c({}, function igIsWindowNavFocusable(_window: c_ptr): c_bool { throw 0; });
const _igSetWindowPos_WindowPtr = $SHBuiltin.extern_c({}, function igSetWindowPos_WindowPtr_cwrap(_window: c_ptr, _pos: c_ptr, _cond: c_int): void { throw 0; });
const _igSetWindowSize_WindowPtr = $SHBuiltin.extern_c({}, function igSetWindowSize_WindowPtr_cwrap(_window: c_ptr, _size: c_ptr, _cond: c_int): void { throw 0; });
const _igSetWindowCollapsed_WindowPtr = $SHBuiltin.extern_c({}, function igSetWindowCollapsed_WindowPtr(_window: c_ptr, _collapsed: c_bool, _cond: c_int): void { throw 0; });
const _igSetWindowHitTestHole = $SHBuiltin.extern_c({}, function igSetWindowHitTestHole_cwrap(_window: c_ptr, _pos: c_ptr, _size: c_ptr): void { throw 0; });
const _igSetWindowHiddendAndSkipItemsForCurrentFrame = $SHBuiltin.extern_c({}, function igSetWindowHiddendAndSkipItemsForCurrentFrame(_window: c_ptr): void { throw 0; });
const _igWindowRectAbsToRel = $SHBuiltin.extern_c({}, function igWindowRectAbsToRel_cwrap(_pOut: c_ptr, _window: c_ptr, _r: c_ptr): void { throw 0; });
const _igWindowRectRelToAbs = $SHBuiltin.extern_c({}, function igWindowRectRelToAbs_cwrap(_pOut: c_ptr, _window: c_ptr, _r: c_ptr): void { throw 0; });
const _igWindowPosRelToAbs = $SHBuiltin.extern_c({}, function igWindowPosRelToAbs_cwrap(_pOut: c_ptr, _window: c_ptr, _p: c_ptr): void { throw 0; });
const _igFocusWindow = $SHBuiltin.extern_c({}, function igFocusWindow(_window: c_ptr, _flags: c_int): void { throw 0; });
const _igFocusTopMostWindowUnderOne = $SHBuiltin.extern_c({}, function igFocusTopMostWindowUnderOne(_under_this_window: c_ptr, _ignore_window: c_ptr, _filter_viewport: c_ptr, _flags: c_int): void { throw 0; });
const _igBringWindowToFocusFront = $SHBuiltin.extern_c({}, function igBringWindowToFocusFront(_window: c_ptr): void { throw 0; });
const _igBringWindowToDisplayFront = $SHBuiltin.extern_c({}, function igBringWindowToDisplayFront(_window: c_ptr): void { throw 0; });
const _igBringWindowToDisplayBack = $SHBuiltin.extern_c({}, function igBringWindowToDisplayBack(_window: c_ptr): void { throw 0; });
const _igBringWindowToDisplayBehind = $SHBuiltin.extern_c({}, function igBringWindowToDisplayBehind(_window: c_ptr, _above_window: c_ptr): void { throw 0; });
const _igFindWindowDisplayIndex = $SHBuiltin.extern_c({}, function igFindWindowDisplayIndex(_window: c_ptr): c_int { throw 0; });
const _igFindBottomMostVisibleWindowWithinBeginStack = $SHBuiltin.extern_c({}, function igFindBottomMostVisibleWindowWithinBeginStack(_window: c_ptr): c_ptr { throw 0; });
const _igSetCurrentFont = $SHBuiltin.extern_c({}, function igSetCurrentFont(_font: c_ptr): void { throw 0; });
const _igGetDefaultFont = $SHBuiltin.extern_c({}, function igGetDefaultFont(): c_ptr { throw 0; });
const _igGetForegroundDrawList_WindowPtr = $SHBuiltin.extern_c({}, function igGetForegroundDrawList_WindowPtr(_window: c_ptr): c_ptr { throw 0; });
const _igGetBackgroundDrawList_ViewportPtr = $SHBuiltin.extern_c({}, function igGetBackgroundDrawList_ViewportPtr(_viewport: c_ptr): c_ptr { throw 0; });
const _igGetForegroundDrawList_ViewportPtr = $SHBuiltin.extern_c({}, function igGetForegroundDrawList_ViewportPtr(_viewport: c_ptr): c_ptr { throw 0; });
const _igAddDrawListToDrawDataEx = $SHBuiltin.extern_c({}, function igAddDrawListToDrawDataEx(_draw_data: c_ptr, _out_list: c_ptr, _draw_list: c_ptr): void { throw 0; });
const _igInitialize = $SHBuiltin.extern_c({}, function igInitialize(): void { throw 0; });
const _igShutdown = $SHBuiltin.extern_c({}, function igShutdown(): void { throw 0; });
const _igUpdateInputEvents = $SHBuiltin.extern_c({}, function igUpdateInputEvents(_trickle_fast_inputs: c_bool): void { throw 0; });
const _igUpdateHoveredWindowAndCaptureFlags = $SHBuiltin.extern_c({}, function igUpdateHoveredWindowAndCaptureFlags(): void { throw 0; });
const _igStartMouseMovingWindow = $SHBuiltin.extern_c({}, function igStartMouseMovingWindow(_window: c_ptr): void { throw 0; });
const _igUpdateMouseMovingWindowNewFrame = $SHBuiltin.extern_c({}, function igUpdateMouseMovingWindowNewFrame(): void { throw 0; });
const _igUpdateMouseMovingWindowEndFrame = $SHBuiltin.extern_c({}, function igUpdateMouseMovingWindowEndFrame(): void { throw 0; });
const _igAddContextHook = $SHBuiltin.extern_c({}, function igAddContextHook(_context: c_ptr, _hook: c_ptr): c_uint { throw 0; });
const _igRemoveContextHook = $SHBuiltin.extern_c({}, function igRemoveContextHook(_context: c_ptr, _hook_to_remove: c_uint): void { throw 0; });
const _igCallContextHooks = $SHBuiltin.extern_c({}, function igCallContextHooks(_context: c_ptr, _type: c_int): void { throw 0; });
const _igSetWindowViewport = $SHBuiltin.extern_c({}, function igSetWindowViewport(_window: c_ptr, _viewport: c_ptr): void { throw 0; });
const _igMarkIniSettingsDirty_Nil = $SHBuiltin.extern_c({}, function igMarkIniSettingsDirty_Nil(): void { throw 0; });
const _igMarkIniSettingsDirty_WindowPtr = $SHBuiltin.extern_c({}, function igMarkIniSettingsDirty_WindowPtr(_window: c_ptr): void { throw 0; });
const _igClearIniSettings = $SHBuiltin.extern_c({}, function igClearIniSettings(): void { throw 0; });
const _igAddSettingsHandler = $SHBuiltin.extern_c({}, function igAddSettingsHandler(_handler: c_ptr): void { throw 0; });
const _igRemoveSettingsHandler = $SHBuiltin.extern_c({}, function igRemoveSettingsHandler(_type_name: c_ptr): void { throw 0; });
const _igFindSettingsHandler = $SHBuiltin.extern_c({}, function igFindSettingsHandler(_type_name: c_ptr): c_ptr { throw 0; });
const _igCreateNewWindowSettings = $SHBuiltin.extern_c({}, function igCreateNewWindowSettings(_name: c_ptr): c_ptr { throw 0; });
const _igFindWindowSettingsByID = $SHBuiltin.extern_c({}, function igFindWindowSettingsByID(_id: c_uint): c_ptr { throw 0; });
const _igFindWindowSettingsByWindow = $SHBuiltin.extern_c({}, function igFindWindowSettingsByWindow(_window: c_ptr): c_ptr { throw 0; });
const _igClearWindowSettings = $SHBuiltin.extern_c({}, function igClearWindowSettings(_name: c_ptr): void { throw 0; });
const _igLocalizeRegisterEntries = $SHBuiltin.extern_c({}, function igLocalizeRegisterEntries(_entries: c_ptr, _count: c_int): void { throw 0; });
const _igLocalizeGetMsg = $SHBuiltin.extern_c({}, function igLocalizeGetMsg(_key: c_int): c_ptr { throw 0; });
const _igSetScrollX_WindowPtr = $SHBuiltin.extern_c({}, function igSetScrollX_WindowPtr(_window: c_ptr, _scroll_x: c_float): void { throw 0; });
const _igSetScrollY_WindowPtr = $SHBuiltin.extern_c({}, function igSetScrollY_WindowPtr(_window: c_ptr, _scroll_y: c_float): void { throw 0; });
const _igSetScrollFromPosX_WindowPtr = $SHBuiltin.extern_c({}, function igSetScrollFromPosX_WindowPtr(_window: c_ptr, _local_x: c_float, _center_x_ratio: c_float): void { throw 0; });
const _igSetScrollFromPosY_WindowPtr = $SHBuiltin.extern_c({}, function igSetScrollFromPosY_WindowPtr(_window: c_ptr, _local_y: c_float, _center_y_ratio: c_float): void { throw 0; });
const _igScrollToItem = $SHBuiltin.extern_c({}, function igScrollToItem(_flags: c_int): void { throw 0; });
const _igScrollToRect = $SHBuiltin.extern_c({}, function igScrollToRect_cwrap(_window: c_ptr, _rect: c_ptr, _flags: c_int): void { throw 0; });
const _igScrollToRectEx = $SHBuiltin.extern_c({}, function igScrollToRectEx_cwrap(_pOut: c_ptr, _window: c_ptr, _rect: c_ptr, _flags: c_int): void { throw 0; });
const _igScrollToBringRectIntoView = $SHBuiltin.extern_c({}, function igScrollToBringRectIntoView_cwrap(_window: c_ptr, _rect: c_ptr): void { throw 0; });
const _igGetItemStatusFlags = $SHBuiltin.extern_c({}, function igGetItemStatusFlags(): c_int { throw 0; });
const _igGetItemFlags = $SHBuiltin.extern_c({}, function igGetItemFlags(): c_int { throw 0; });
const _igGetActiveID = $SHBuiltin.extern_c({}, function igGetActiveID(): c_uint { throw 0; });
const _igGetFocusID = $SHBuiltin.extern_c({}, function igGetFocusID(): c_uint { throw 0; });
const _igSetActiveID = $SHBuiltin.extern_c({}, function igSetActiveID(_id: c_uint, _window: c_ptr): void { throw 0; });
const _igSetFocusID = $SHBuiltin.extern_c({}, function igSetFocusID(_id: c_uint, _window: c_ptr): void { throw 0; });
const _igClearActiveID = $SHBuiltin.extern_c({}, function igClearActiveID(): void { throw 0; });
const _igGetHoveredID = $SHBuiltin.extern_c({}, function igGetHoveredID(): c_uint { throw 0; });
const _igSetHoveredID = $SHBuiltin.extern_c({}, function igSetHoveredID(_id: c_uint): void { throw 0; });
const _igKeepAliveID = $SHBuiltin.extern_c({}, function igKeepAliveID(_id: c_uint): void { throw 0; });
const _igMarkItemEdited = $SHBuiltin.extern_c({}, function igMarkItemEdited(_id: c_uint): void { throw 0; });
const _igPushOverrideID = $SHBuiltin.extern_c({}, function igPushOverrideID(_id: c_uint): void { throw 0; });
const _igGetIDWithSeed_Str = $SHBuiltin.extern_c({}, function igGetIDWithSeed_Str(_str_id_begin: c_ptr, _str_id_end: c_ptr, _seed: c_uint): c_uint { throw 0; });
const _igGetIDWithSeed_Int = $SHBuiltin.extern_c({}, function igGetIDWithSeed_Int(_n: c_int, _seed: c_uint): c_uint { throw 0; });
const _igItemSize_Vec2 = $SHBuiltin.extern_c({}, function igItemSize_Vec2_cwrap(_size: c_ptr, _text_baseline_y: c_float): void { throw 0; });
const _igItemSize_Rect = $SHBuiltin.extern_c({}, function igItemSize_Rect_cwrap(_bb: c_ptr, _text_baseline_y: c_float): void { throw 0; });
const _igItemAdd = $SHBuiltin.extern_c({}, function igItemAdd_cwrap(_bb: c_ptr, _id: c_uint, _nav_bb: c_ptr, _extra_flags: c_int): c_bool { throw 0; });
const _igItemHoverable = $SHBuiltin.extern_c({}, function igItemHoverable_cwrap(_bb: c_ptr, _id: c_uint, _item_flags: c_int): c_bool { throw 0; });
const _igIsWindowContentHoverable = $SHBuiltin.extern_c({}, function igIsWindowContentHoverable(_window: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igIsClippedEx = $SHBuiltin.extern_c({}, function igIsClippedEx_cwrap(_bb: c_ptr, _id: c_uint): c_bool { throw 0; });
const _igSetLastItemData = $SHBuiltin.extern_c({}, function igSetLastItemData_cwrap(_item_id: c_uint, _in_flags: c_int, _status_flags: c_int, _item_rect: c_ptr): void { throw 0; });
const _igCalcItemSize = $SHBuiltin.extern_c({}, function igCalcItemSize_cwrap(_pOut: c_ptr, _size: c_ptr, _default_w: c_float, _default_h: c_float): void { throw 0; });
const _igCalcWrapWidthForPos = $SHBuiltin.extern_c({}, function igCalcWrapWidthForPos_cwrap(_pos: c_ptr, _wrap_pos_x: c_float): c_float { throw 0; });
const _igPushMultiItemsWidths = $SHBuiltin.extern_c({}, function igPushMultiItemsWidths(_components: c_int, _width_full: c_float): void { throw 0; });
const _igIsItemToggledSelection = $SHBuiltin.extern_c({}, function igIsItemToggledSelection(): c_bool { throw 0; });
const _igGetContentRegionMaxAbs = $SHBuiltin.extern_c({}, function igGetContentRegionMaxAbs(_pOut: c_ptr): void { throw 0; });
const _igShrinkWidths = $SHBuiltin.extern_c({}, function igShrinkWidths(_items: c_ptr, _count: c_int, _width_excess: c_float): void { throw 0; });
const _igPushItemFlag = $SHBuiltin.extern_c({}, function igPushItemFlag(_option: c_int, _enabled: c_bool): void { throw 0; });
const _igPopItemFlag = $SHBuiltin.extern_c({}, function igPopItemFlag(): void { throw 0; });
const _igGetStyleVarInfo = $SHBuiltin.extern_c({}, function igGetStyleVarInfo(_idx: c_int): c_ptr { throw 0; });
const _igLogBegin = $SHBuiltin.extern_c({}, function igLogBegin(_type: c_int, _auto_open_depth: c_int): void { throw 0; });
const _igLogToBuffer = $SHBuiltin.extern_c({}, function igLogToBuffer(_auto_open_depth: c_int): void { throw 0; });
const _igLogRenderedText = $SHBuiltin.extern_c({}, function igLogRenderedText(_ref_pos: c_ptr, _text: c_ptr, _text_end: c_ptr): void { throw 0; });
const _igLogSetNextTextDecoration = $SHBuiltin.extern_c({}, function igLogSetNextTextDecoration(_prefix: c_ptr, _suffix: c_ptr): void { throw 0; });
const _igBeginChildEx = $SHBuiltin.extern_c({}, function igBeginChildEx_cwrap(_name: c_ptr, _id: c_uint, _size_arg: c_ptr, _border: c_bool, _flags: c_int): c_bool { throw 0; });
const _igOpenPopupEx = $SHBuiltin.extern_c({}, function igOpenPopupEx(_id: c_uint, _popup_flags: c_int): void { throw 0; });
const _igClosePopupToLevel = $SHBuiltin.extern_c({}, function igClosePopupToLevel(_remaining: c_int, _restore_focus_to_window_under_popup: c_bool): void { throw 0; });
const _igClosePopupsOverWindow = $SHBuiltin.extern_c({}, function igClosePopupsOverWindow(_ref_window: c_ptr, _restore_focus_to_window_under_popup: c_bool): void { throw 0; });
const _igClosePopupsExceptModals = $SHBuiltin.extern_c({}, function igClosePopupsExceptModals(): void { throw 0; });
const _igIsPopupOpen_ID = $SHBuiltin.extern_c({}, function igIsPopupOpen_ID(_id: c_uint, _popup_flags: c_int): c_bool { throw 0; });
const _igBeginPopupEx = $SHBuiltin.extern_c({}, function igBeginPopupEx(_id: c_uint, _extra_flags: c_int): c_bool { throw 0; });
const _igBeginTooltipEx = $SHBuiltin.extern_c({}, function igBeginTooltipEx(_tooltip_flags: c_int, _extra_window_flags: c_int): c_bool { throw 0; });
const _igGetPopupAllowedExtentRect = $SHBuiltin.extern_c({}, function igGetPopupAllowedExtentRect(_pOut: c_ptr, _window: c_ptr): void { throw 0; });
const _igGetTopMostPopupModal = $SHBuiltin.extern_c({}, function igGetTopMostPopupModal(): c_ptr { throw 0; });
const _igGetTopMostAndVisiblePopupModal = $SHBuiltin.extern_c({}, function igGetTopMostAndVisiblePopupModal(): c_ptr { throw 0; });
const _igFindBlockingModal = $SHBuiltin.extern_c({}, function igFindBlockingModal(_window: c_ptr): c_ptr { throw 0; });
const _igFindBestWindowPosForPopup = $SHBuiltin.extern_c({}, function igFindBestWindowPosForPopup(_pOut: c_ptr, _window: c_ptr): void { throw 0; });
const _igFindBestWindowPosForPopupEx = $SHBuiltin.extern_c({}, function igFindBestWindowPosForPopupEx_cwrap(_pOut: c_ptr, _ref_pos: c_ptr, _size: c_ptr, _last_dir: c_ptr, _r_outer: c_ptr, _r_avoid: c_ptr, _policy: c_int): void { throw 0; });
const _igBeginViewportSideBar = $SHBuiltin.extern_c({}, function igBeginViewportSideBar(_name: c_ptr, _viewport: c_ptr, _dir: c_int, _size: c_float, _window_flags: c_int): c_bool { throw 0; });
const _igBeginMenuEx = $SHBuiltin.extern_c({}, function igBeginMenuEx(_label: c_ptr, _icon: c_ptr, _enabled: c_bool): c_bool { throw 0; });
const _igMenuItemEx = $SHBuiltin.extern_c({}, function igMenuItemEx(_label: c_ptr, _icon: c_ptr, _shortcut: c_ptr, _selected: c_bool, _enabled: c_bool): c_bool { throw 0; });
const _igBeginComboPopup = $SHBuiltin.extern_c({}, function igBeginComboPopup_cwrap(_popup_id: c_uint, _bb: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igBeginComboPreview = $SHBuiltin.extern_c({}, function igBeginComboPreview(): c_bool { throw 0; });
const _igEndComboPreview = $SHBuiltin.extern_c({}, function igEndComboPreview(): void { throw 0; });
const _igNavInitWindow = $SHBuiltin.extern_c({}, function igNavInitWindow(_window: c_ptr, _force_reinit: c_bool): void { throw 0; });
const _igNavInitRequestApplyResult = $SHBuiltin.extern_c({}, function igNavInitRequestApplyResult(): void { throw 0; });
const _igNavMoveRequestButNoResultYet = $SHBuiltin.extern_c({}, function igNavMoveRequestButNoResultYet(): c_bool { throw 0; });
const _igNavMoveRequestSubmit = $SHBuiltin.extern_c({}, function igNavMoveRequestSubmit(_move_dir: c_int, _clip_dir: c_int, _move_flags: c_int, _scroll_flags: c_int): void { throw 0; });
const _igNavMoveRequestForward = $SHBuiltin.extern_c({}, function igNavMoveRequestForward(_move_dir: c_int, _clip_dir: c_int, _move_flags: c_int, _scroll_flags: c_int): void { throw 0; });
const _igNavMoveRequestResolveWithLastItem = $SHBuiltin.extern_c({}, function igNavMoveRequestResolveWithLastItem(_result: c_ptr): void { throw 0; });
const _igNavMoveRequestResolveWithPastTreeNode = $SHBuiltin.extern_c({}, function igNavMoveRequestResolveWithPastTreeNode(_result: c_ptr, _tree_node_data: c_ptr): void { throw 0; });
const _igNavMoveRequestCancel = $SHBuiltin.extern_c({}, function igNavMoveRequestCancel(): void { throw 0; });
const _igNavMoveRequestApplyResult = $SHBuiltin.extern_c({}, function igNavMoveRequestApplyResult(): void { throw 0; });
const _igNavMoveRequestTryWrapping = $SHBuiltin.extern_c({}, function igNavMoveRequestTryWrapping(_window: c_ptr, _move_flags: c_int): void { throw 0; });
const _igNavClearPreferredPosForAxis = $SHBuiltin.extern_c({}, function igNavClearPreferredPosForAxis(_axis: c_int): void { throw 0; });
const _igNavUpdateCurrentWindowIsScrollPushableX = $SHBuiltin.extern_c({}, function igNavUpdateCurrentWindowIsScrollPushableX(): void { throw 0; });
const _igSetNavWindow = $SHBuiltin.extern_c({}, function igSetNavWindow(_window: c_ptr): void { throw 0; });
const _igSetNavID = $SHBuiltin.extern_c({}, function igSetNavID_cwrap(_id: c_uint, _nav_layer: c_int, _focus_scope_id: c_uint, _rect_rel: c_ptr): void { throw 0; });
const _igFocusItem = $SHBuiltin.extern_c({}, function igFocusItem(): void { throw 0; });
const _igActivateItemByID = $SHBuiltin.extern_c({}, function igActivateItemByID(_id: c_uint): void { throw 0; });
const _igIsNamedKey = $SHBuiltin.extern_c({}, function igIsNamedKey(_key: c_int): c_bool { throw 0; });
const _igIsNamedKeyOrModKey = $SHBuiltin.extern_c({}, function igIsNamedKeyOrModKey(_key: c_int): c_bool { throw 0; });
const _igIsLegacyKey = $SHBuiltin.extern_c({}, function igIsLegacyKey(_key: c_int): c_bool { throw 0; });
const _igIsKeyboardKey = $SHBuiltin.extern_c({}, function igIsKeyboardKey(_key: c_int): c_bool { throw 0; });
const _igIsGamepadKey = $SHBuiltin.extern_c({}, function igIsGamepadKey(_key: c_int): c_bool { throw 0; });
const _igIsMouseKey = $SHBuiltin.extern_c({}, function igIsMouseKey(_key: c_int): c_bool { throw 0; });
const _igIsAliasKey = $SHBuiltin.extern_c({}, function igIsAliasKey(_key: c_int): c_bool { throw 0; });
const _igConvertShortcutMod = $SHBuiltin.extern_c({}, function igConvertShortcutMod(_key_chord: c_int): c_int { throw 0; });
const _igConvertSingleModFlagToKey = $SHBuiltin.extern_c({}, function igConvertSingleModFlagToKey(_ctx: c_ptr, _key: c_int): c_int { throw 0; });
const _igGetKeyData_ContextPtr = $SHBuiltin.extern_c({}, function igGetKeyData_ContextPtr(_ctx: c_ptr, _key: c_int): c_ptr { throw 0; });
const _igGetKeyData_Key = $SHBuiltin.extern_c({}, function igGetKeyData_Key(_key: c_int): c_ptr { throw 0; });
const _igGetKeyChordName = $SHBuiltin.extern_c({}, function igGetKeyChordName(_key_chord: c_int, _out_buf: c_ptr, _out_buf_size: c_int): void { throw 0; });
const _igMouseButtonToKey = $SHBuiltin.extern_c({}, function igMouseButtonToKey(_button: c_int): c_int { throw 0; });
const _igIsMouseDragPastThreshold = $SHBuiltin.extern_c({}, function igIsMouseDragPastThreshold(_button: c_int, _lock_threshold: c_float): c_bool { throw 0; });
const _igGetKeyMagnitude2d = $SHBuiltin.extern_c({}, function igGetKeyMagnitude2d(_pOut: c_ptr, _key_left: c_int, _key_right: c_int, _key_up: c_int, _key_down: c_int): void { throw 0; });
const _igGetNavTweakPressedAmount = $SHBuiltin.extern_c({}, function igGetNavTweakPressedAmount(_axis: c_int): c_float { throw 0; });
const _igCalcTypematicRepeatAmount = $SHBuiltin.extern_c({}, function igCalcTypematicRepeatAmount(_t0: c_float, _t1: c_float, _repeat_delay: c_float, _repeat_rate: c_float): c_int { throw 0; });
const _igGetTypematicRepeatRate = $SHBuiltin.extern_c({}, function igGetTypematicRepeatRate(_flags: c_int, _repeat_delay: c_ptr, _repeat_rate: c_ptr): void { throw 0; });
const _igSetActiveIdUsingAllKeyboardKeys = $SHBuiltin.extern_c({}, function igSetActiveIdUsingAllKeyboardKeys(): void { throw 0; });
const _igIsActiveIdUsingNavDir = $SHBuiltin.extern_c({}, function igIsActiveIdUsingNavDir(_dir: c_int): c_bool { throw 0; });
const _igGetKeyOwner = $SHBuiltin.extern_c({}, function igGetKeyOwner(_key: c_int): c_uint { throw 0; });
const _igSetKeyOwner = $SHBuiltin.extern_c({}, function igSetKeyOwner(_key: c_int, _owner_id: c_uint, _flags: c_int): void { throw 0; });
const _igSetKeyOwnersForKeyChord = $SHBuiltin.extern_c({}, function igSetKeyOwnersForKeyChord(_key: c_int, _owner_id: c_uint, _flags: c_int): void { throw 0; });
const _igSetItemKeyOwner = $SHBuiltin.extern_c({}, function igSetItemKeyOwner(_key: c_int, _flags: c_int): void { throw 0; });
const _igTestKeyOwner = $SHBuiltin.extern_c({}, function igTestKeyOwner(_key: c_int, _owner_id: c_uint): c_bool { throw 0; });
const _igGetKeyOwnerData = $SHBuiltin.extern_c({}, function igGetKeyOwnerData(_ctx: c_ptr, _key: c_int): c_ptr { throw 0; });
const _igIsKeyDown_ID = $SHBuiltin.extern_c({}, function igIsKeyDown_ID(_key: c_int, _owner_id: c_uint): c_bool { throw 0; });
const _igIsKeyPressed_ID = $SHBuiltin.extern_c({}, function igIsKeyPressed_ID(_key: c_int, _owner_id: c_uint, _flags: c_int): c_bool { throw 0; });
const _igIsKeyReleased_ID = $SHBuiltin.extern_c({}, function igIsKeyReleased_ID(_key: c_int, _owner_id: c_uint): c_bool { throw 0; });
const _igIsMouseDown_ID = $SHBuiltin.extern_c({}, function igIsMouseDown_ID(_button: c_int, _owner_id: c_uint): c_bool { throw 0; });
const _igIsMouseClicked_ID = $SHBuiltin.extern_c({}, function igIsMouseClicked_ID(_button: c_int, _owner_id: c_uint, _flags: c_int): c_bool { throw 0; });
const _igIsMouseReleased_ID = $SHBuiltin.extern_c({}, function igIsMouseReleased_ID(_button: c_int, _owner_id: c_uint): c_bool { throw 0; });
const _igShortcut = $SHBuiltin.extern_c({}, function igShortcut(_key_chord: c_int, _owner_id: c_uint, _flags: c_int): c_bool { throw 0; });
const _igSetShortcutRouting = $SHBuiltin.extern_c({}, function igSetShortcutRouting(_key_chord: c_int, _owner_id: c_uint, _flags: c_int): c_bool { throw 0; });
const _igTestShortcutRouting = $SHBuiltin.extern_c({}, function igTestShortcutRouting(_key_chord: c_int, _owner_id: c_uint): c_bool { throw 0; });
const _igGetShortcutRoutingData = $SHBuiltin.extern_c({}, function igGetShortcutRoutingData(_key_chord: c_int): c_ptr { throw 0; });
const _igPushFocusScope = $SHBuiltin.extern_c({}, function igPushFocusScope(_id: c_uint): void { throw 0; });
const _igPopFocusScope = $SHBuiltin.extern_c({}, function igPopFocusScope(): void { throw 0; });
const _igGetCurrentFocusScope = $SHBuiltin.extern_c({}, function igGetCurrentFocusScope(): c_uint { throw 0; });
const _igIsDragDropActive = $SHBuiltin.extern_c({}, function igIsDragDropActive(): c_bool { throw 0; });
const _igBeginDragDropTargetCustom = $SHBuiltin.extern_c({}, function igBeginDragDropTargetCustom_cwrap(_bb: c_ptr, _id: c_uint): c_bool { throw 0; });
const _igClearDragDrop = $SHBuiltin.extern_c({}, function igClearDragDrop(): void { throw 0; });
const _igIsDragDropPayloadBeingAccepted = $SHBuiltin.extern_c({}, function igIsDragDropPayloadBeingAccepted(): c_bool { throw 0; });
const _igRenderDragDropTargetRect = $SHBuiltin.extern_c({}, function igRenderDragDropTargetRect_cwrap(_bb: c_ptr): void { throw 0; });
const _igSetWindowClipRectBeforeSetChannel = $SHBuiltin.extern_c({}, function igSetWindowClipRectBeforeSetChannel_cwrap(_window: c_ptr, _clip_rect: c_ptr): void { throw 0; });
const _igBeginColumns = $SHBuiltin.extern_c({}, function igBeginColumns(_str_id: c_ptr, _count: c_int, _flags: c_int): void { throw 0; });
const _igEndColumns = $SHBuiltin.extern_c({}, function igEndColumns(): void { throw 0; });
const _igPushColumnClipRect = $SHBuiltin.extern_c({}, function igPushColumnClipRect(_column_index: c_int): void { throw 0; });
const _igPushColumnsBackground = $SHBuiltin.extern_c({}, function igPushColumnsBackground(): void { throw 0; });
const _igPopColumnsBackground = $SHBuiltin.extern_c({}, function igPopColumnsBackground(): void { throw 0; });
const _igGetColumnsID = $SHBuiltin.extern_c({}, function igGetColumnsID(_str_id: c_ptr, _count: c_int): c_uint { throw 0; });
const _igFindOrCreateColumns = $SHBuiltin.extern_c({}, function igFindOrCreateColumns(_window: c_ptr, _id: c_uint): c_ptr { throw 0; });
const _igGetColumnOffsetFromNorm = $SHBuiltin.extern_c({}, function igGetColumnOffsetFromNorm(_columns: c_ptr, _offset_norm: c_float): c_float { throw 0; });
const _igGetColumnNormFromOffset = $SHBuiltin.extern_c({}, function igGetColumnNormFromOffset(_columns: c_ptr, _offset: c_float): c_float { throw 0; });
const _igTableOpenContextMenu = $SHBuiltin.extern_c({}, function igTableOpenContextMenu(_column_n: c_int): void { throw 0; });
const _igTableSetColumnWidth = $SHBuiltin.extern_c({}, function igTableSetColumnWidth(_column_n: c_int, _width: c_float): void { throw 0; });
const _igTableSetColumnSortDirection = $SHBuiltin.extern_c({}, function igTableSetColumnSortDirection(_column_n: c_int, _sort_direction: c_int, _append_to_sort_specs: c_bool): void { throw 0; });
const _igTableGetHoveredColumn = $SHBuiltin.extern_c({}, function igTableGetHoveredColumn(): c_int { throw 0; });
const _igTableGetHoveredRow = $SHBuiltin.extern_c({}, function igTableGetHoveredRow(): c_int { throw 0; });
const _igTableGetHeaderRowHeight = $SHBuiltin.extern_c({}, function igTableGetHeaderRowHeight(): c_float { throw 0; });
const _igTablePushBackgroundChannel = $SHBuiltin.extern_c({}, function igTablePushBackgroundChannel(): void { throw 0; });
const _igTablePopBackgroundChannel = $SHBuiltin.extern_c({}, function igTablePopBackgroundChannel(): void { throw 0; });
const _igGetCurrentTable = $SHBuiltin.extern_c({}, function igGetCurrentTable(): c_ptr { throw 0; });
const _igTableFindByID = $SHBuiltin.extern_c({}, function igTableFindByID(_id: c_uint): c_ptr { throw 0; });
const _igBeginTableEx = $SHBuiltin.extern_c({}, function igBeginTableEx_cwrap(_name: c_ptr, _id: c_uint, _columns_count: c_int, _flags: c_int, _outer_size: c_ptr, _inner_width: c_float): c_bool { throw 0; });
const _igTableBeginInitMemory = $SHBuiltin.extern_c({}, function igTableBeginInitMemory(_table: c_ptr, _columns_count: c_int): void { throw 0; });
const _igTableBeginApplyRequests = $SHBuiltin.extern_c({}, function igTableBeginApplyRequests(_table: c_ptr): void { throw 0; });
const _igTableSetupDrawChannels = $SHBuiltin.extern_c({}, function igTableSetupDrawChannels(_table: c_ptr): void { throw 0; });
const _igTableUpdateLayout = $SHBuiltin.extern_c({}, function igTableUpdateLayout(_table: c_ptr): void { throw 0; });
const _igTableUpdateBorders = $SHBuiltin.extern_c({}, function igTableUpdateBorders(_table: c_ptr): void { throw 0; });
const _igTableUpdateColumnsWeightFromWidth = $SHBuiltin.extern_c({}, function igTableUpdateColumnsWeightFromWidth(_table: c_ptr): void { throw 0; });
const _igTableDrawBorders = $SHBuiltin.extern_c({}, function igTableDrawBorders(_table: c_ptr): void { throw 0; });
const _igTableDrawContextMenu = $SHBuiltin.extern_c({}, function igTableDrawContextMenu(_table: c_ptr): void { throw 0; });
const _igTableBeginContextMenuPopup = $SHBuiltin.extern_c({}, function igTableBeginContextMenuPopup(_table: c_ptr): c_bool { throw 0; });
const _igTableMergeDrawChannels = $SHBuiltin.extern_c({}, function igTableMergeDrawChannels(_table: c_ptr): void { throw 0; });
const _igTableGetInstanceData = $SHBuiltin.extern_c({}, function igTableGetInstanceData(_table: c_ptr, _instance_no: c_int): c_ptr { throw 0; });
const _igTableGetInstanceID = $SHBuiltin.extern_c({}, function igTableGetInstanceID(_table: c_ptr, _instance_no: c_int): c_uint { throw 0; });
const _igTableSortSpecsSanitize = $SHBuiltin.extern_c({}, function igTableSortSpecsSanitize(_table: c_ptr): void { throw 0; });
const _igTableSortSpecsBuild = $SHBuiltin.extern_c({}, function igTableSortSpecsBuild(_table: c_ptr): void { throw 0; });
const _igTableGetColumnNextSortDirection = $SHBuiltin.extern_c({}, function igTableGetColumnNextSortDirection(_column: c_ptr): c_int { throw 0; });
const _igTableFixColumnSortDirection = $SHBuiltin.extern_c({}, function igTableFixColumnSortDirection(_table: c_ptr, _column: c_ptr): void { throw 0; });
const _igTableGetColumnWidthAuto = $SHBuiltin.extern_c({}, function igTableGetColumnWidthAuto(_table: c_ptr, _column: c_ptr): c_float { throw 0; });
const _igTableBeginRow = $SHBuiltin.extern_c({}, function igTableBeginRow(_table: c_ptr): void { throw 0; });
const _igTableEndRow = $SHBuiltin.extern_c({}, function igTableEndRow(_table: c_ptr): void { throw 0; });
const _igTableBeginCell = $SHBuiltin.extern_c({}, function igTableBeginCell(_table: c_ptr, _column_n: c_int): void { throw 0; });
const _igTableEndCell = $SHBuiltin.extern_c({}, function igTableEndCell(_table: c_ptr): void { throw 0; });
const _igTableGetCellBgRect = $SHBuiltin.extern_c({}, function igTableGetCellBgRect(_pOut: c_ptr, _table: c_ptr, _column_n: c_int): void { throw 0; });
const _igTableGetColumnName_TablePtr = $SHBuiltin.extern_c({}, function igTableGetColumnName_TablePtr(_table: c_ptr, _column_n: c_int): c_ptr { throw 0; });
const _igTableGetColumnResizeID = $SHBuiltin.extern_c({}, function igTableGetColumnResizeID(_table: c_ptr, _column_n: c_int, _instance_no: c_int): c_uint { throw 0; });
const _igTableGetMaxColumnWidth = $SHBuiltin.extern_c({}, function igTableGetMaxColumnWidth(_table: c_ptr, _column_n: c_int): c_float { throw 0; });
const _igTableSetColumnWidthAutoSingle = $SHBuiltin.extern_c({}, function igTableSetColumnWidthAutoSingle(_table: c_ptr, _column_n: c_int): void { throw 0; });
const _igTableSetColumnWidthAutoAll = $SHBuiltin.extern_c({}, function igTableSetColumnWidthAutoAll(_table: c_ptr): void { throw 0; });
const _igTableRemove = $SHBuiltin.extern_c({}, function igTableRemove(_table: c_ptr): void { throw 0; });
const _igTableGcCompactTransientBuffers_TablePtr = $SHBuiltin.extern_c({}, function igTableGcCompactTransientBuffers_TablePtr(_table: c_ptr): void { throw 0; });
const _igTableGcCompactTransientBuffers_TableTempDataPtr = $SHBuiltin.extern_c({}, function igTableGcCompactTransientBuffers_TableTempDataPtr(_table: c_ptr): void { throw 0; });
const _igTableGcCompactSettings = $SHBuiltin.extern_c({}, function igTableGcCompactSettings(): void { throw 0; });
const _igTableLoadSettings = $SHBuiltin.extern_c({}, function igTableLoadSettings(_table: c_ptr): void { throw 0; });
const _igTableSaveSettings = $SHBuiltin.extern_c({}, function igTableSaveSettings(_table: c_ptr): void { throw 0; });
const _igTableResetSettings = $SHBuiltin.extern_c({}, function igTableResetSettings(_table: c_ptr): void { throw 0; });
const _igTableGetBoundSettings = $SHBuiltin.extern_c({}, function igTableGetBoundSettings(_table: c_ptr): c_ptr { throw 0; });
const _igTableSettingsAddSettingsHandler = $SHBuiltin.extern_c({}, function igTableSettingsAddSettingsHandler(): void { throw 0; });
const _igTableSettingsCreate = $SHBuiltin.extern_c({}, function igTableSettingsCreate(_id: c_uint, _columns_count: c_int): c_ptr { throw 0; });
const _igTableSettingsFindByID = $SHBuiltin.extern_c({}, function igTableSettingsFindByID(_id: c_uint): c_ptr { throw 0; });
const _igGetCurrentTabBar = $SHBuiltin.extern_c({}, function igGetCurrentTabBar(): c_ptr { throw 0; });
const _igBeginTabBarEx = $SHBuiltin.extern_c({}, function igBeginTabBarEx_cwrap(_tab_bar: c_ptr, _bb: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igTabBarFindTabByID = $SHBuiltin.extern_c({}, function igTabBarFindTabByID(_tab_bar: c_ptr, _tab_id: c_uint): c_ptr { throw 0; });
const _igTabBarFindTabByOrder = $SHBuiltin.extern_c({}, function igTabBarFindTabByOrder(_tab_bar: c_ptr, _order: c_int): c_ptr { throw 0; });
const _igTabBarGetCurrentTab = $SHBuiltin.extern_c({}, function igTabBarGetCurrentTab(_tab_bar: c_ptr): c_ptr { throw 0; });
const _igTabBarGetTabOrder = $SHBuiltin.extern_c({}, function igTabBarGetTabOrder(_tab_bar: c_ptr, _tab: c_ptr): c_int { throw 0; });
const _igTabBarGetTabName = $SHBuiltin.extern_c({}, function igTabBarGetTabName(_tab_bar: c_ptr, _tab: c_ptr): c_ptr { throw 0; });
const _igTabBarRemoveTab = $SHBuiltin.extern_c({}, function igTabBarRemoveTab(_tab_bar: c_ptr, _tab_id: c_uint): void { throw 0; });
const _igTabBarCloseTab = $SHBuiltin.extern_c({}, function igTabBarCloseTab(_tab_bar: c_ptr, _tab: c_ptr): void { throw 0; });
const _igTabBarQueueFocus = $SHBuiltin.extern_c({}, function igTabBarQueueFocus(_tab_bar: c_ptr, _tab: c_ptr): void { throw 0; });
const _igTabBarQueueReorder = $SHBuiltin.extern_c({}, function igTabBarQueueReorder(_tab_bar: c_ptr, _tab: c_ptr, _offset: c_int): void { throw 0; });
const _igTabBarQueueReorderFromMousePos = $SHBuiltin.extern_c({}, function igTabBarQueueReorderFromMousePos_cwrap(_tab_bar: c_ptr, _tab: c_ptr, _mouse_pos: c_ptr): void { throw 0; });
const _igTabBarProcessReorder = $SHBuiltin.extern_c({}, function igTabBarProcessReorder(_tab_bar: c_ptr): c_bool { throw 0; });
const _igTabItemEx = $SHBuiltin.extern_c({}, function igTabItemEx(_tab_bar: c_ptr, _label: c_ptr, _p_open: c_ptr, _flags: c_int, _docked_window: c_ptr): c_bool { throw 0; });
const _igTabItemCalcSize_Str = $SHBuiltin.extern_c({}, function igTabItemCalcSize_Str(_pOut: c_ptr, _label: c_ptr, _has_close_button_or_unsaved_marker: c_bool): void { throw 0; });
const _igTabItemCalcSize_WindowPtr = $SHBuiltin.extern_c({}, function igTabItemCalcSize_WindowPtr(_pOut: c_ptr, _window: c_ptr): void { throw 0; });
const _igTabItemBackground = $SHBuiltin.extern_c({}, function igTabItemBackground_cwrap(_draw_list: c_ptr, _bb: c_ptr, _flags: c_int, _col: c_uint): void { throw 0; });
const _igTabItemLabelAndCloseButton = $SHBuiltin.extern_c({}, function igTabItemLabelAndCloseButton_cwrap(_draw_list: c_ptr, _bb: c_ptr, _flags: c_int, _frame_padding: c_ptr, _label: c_ptr, _tab_id: c_uint, _close_button_id: c_uint, _is_contents_visible: c_bool, _out_just_closed: c_ptr, _out_text_clipped: c_ptr): void { throw 0; });
const _igRenderText = $SHBuiltin.extern_c({}, function igRenderText_cwrap(_pos: c_ptr, _text: c_ptr, _text_end: c_ptr, _hide_text_after_hash: c_bool): void { throw 0; });
const _igRenderTextWrapped = $SHBuiltin.extern_c({}, function igRenderTextWrapped_cwrap(_pos: c_ptr, _text: c_ptr, _text_end: c_ptr, _wrap_width: c_float): void { throw 0; });
const _igRenderTextClipped = $SHBuiltin.extern_c({}, function igRenderTextClipped_cwrap(_pos_min: c_ptr, _pos_max: c_ptr, _text: c_ptr, _text_end: c_ptr, _text_size_if_known: c_ptr, _align: c_ptr, _clip_rect: c_ptr): void { throw 0; });
const _igRenderTextClippedEx = $SHBuiltin.extern_c({}, function igRenderTextClippedEx_cwrap(_draw_list: c_ptr, _pos_min: c_ptr, _pos_max: c_ptr, _text: c_ptr, _text_end: c_ptr, _text_size_if_known: c_ptr, _align: c_ptr, _clip_rect: c_ptr): void { throw 0; });
const _igRenderTextEllipsis = $SHBuiltin.extern_c({}, function igRenderTextEllipsis_cwrap(_draw_list: c_ptr, _pos_min: c_ptr, _pos_max: c_ptr, _clip_max_x: c_float, _ellipsis_max_x: c_float, _text: c_ptr, _text_end: c_ptr, _text_size_if_known: c_ptr): void { throw 0; });
const _igRenderFrame = $SHBuiltin.extern_c({}, function igRenderFrame_cwrap(_p_min: c_ptr, _p_max: c_ptr, _fill_col: c_uint, _border: c_bool, _rounding: c_float): void { throw 0; });
const _igRenderFrameBorder = $SHBuiltin.extern_c({}, function igRenderFrameBorder_cwrap(_p_min: c_ptr, _p_max: c_ptr, _rounding: c_float): void { throw 0; });
const _igRenderColorRectWithAlphaCheckerboard = $SHBuiltin.extern_c({}, function igRenderColorRectWithAlphaCheckerboard_cwrap(_draw_list: c_ptr, _p_min: c_ptr, _p_max: c_ptr, _fill_col: c_uint, _grid_step: c_float, _grid_off: c_ptr, _rounding: c_float, _flags: c_int): void { throw 0; });
const _igRenderNavHighlight = $SHBuiltin.extern_c({}, function igRenderNavHighlight_cwrap(_bb: c_ptr, _id: c_uint, _flags: c_int): void { throw 0; });
const _igFindRenderedTextEnd = $SHBuiltin.extern_c({}, function igFindRenderedTextEnd(_text: c_ptr, _text_end: c_ptr): c_ptr { throw 0; });
const _igRenderMouseCursor = $SHBuiltin.extern_c({}, function igRenderMouseCursor_cwrap(_pos: c_ptr, _scale: c_float, _mouse_cursor: c_int, _col_fill: c_uint, _col_border: c_uint, _col_shadow: c_uint): void { throw 0; });
const _igRenderArrow = $SHBuiltin.extern_c({}, function igRenderArrow_cwrap(_draw_list: c_ptr, _pos: c_ptr, _col: c_uint, _dir: c_int, _scale: c_float): void { throw 0; });
const _igRenderBullet = $SHBuiltin.extern_c({}, function igRenderBullet_cwrap(_draw_list: c_ptr, _pos: c_ptr, _col: c_uint): void { throw 0; });
const _igRenderCheckMark = $SHBuiltin.extern_c({}, function igRenderCheckMark_cwrap(_draw_list: c_ptr, _pos: c_ptr, _col: c_uint, _sz: c_float): void { throw 0; });
const _igRenderArrowPointingAt = $SHBuiltin.extern_c({}, function igRenderArrowPointingAt_cwrap(_draw_list: c_ptr, _pos: c_ptr, _half_sz: c_ptr, _direction: c_int, _col: c_uint): void { throw 0; });
const _igRenderRectFilledRangeH = $SHBuiltin.extern_c({}, function igRenderRectFilledRangeH_cwrap(_draw_list: c_ptr, _rect: c_ptr, _col: c_uint, _x_start_norm: c_float, _x_end_norm: c_float, _rounding: c_float): void { throw 0; });
const _igRenderRectFilledWithHole = $SHBuiltin.extern_c({}, function igRenderRectFilledWithHole_cwrap(_draw_list: c_ptr, _outer: c_ptr, _inner: c_ptr, _col: c_uint, _rounding: c_float): void { throw 0; });
const _igTextEx = $SHBuiltin.extern_c({}, function igTextEx(_text: c_ptr, _text_end: c_ptr, _flags: c_int): void { throw 0; });
const _igButtonEx = $SHBuiltin.extern_c({}, function igButtonEx_cwrap(_label: c_ptr, _size_arg: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igArrowButtonEx = $SHBuiltin.extern_c({}, function igArrowButtonEx_cwrap(_str_id: c_ptr, _dir: c_int, _size_arg: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igImageButtonEx = $SHBuiltin.extern_c({}, function igImageButtonEx_cwrap(_id: c_uint, _texture_id: c_ptr, _size: c_ptr, _uv0: c_ptr, _uv1: c_ptr, _bg_col: c_ptr, _tint_col: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSeparatorEx = $SHBuiltin.extern_c({}, function igSeparatorEx(_flags: c_int, _thickness: c_float): void { throw 0; });
const _igSeparatorTextEx = $SHBuiltin.extern_c({}, function igSeparatorTextEx(_id: c_uint, _label: c_ptr, _label_end: c_ptr, _extra_width: c_float): void { throw 0; });
const _igCheckboxFlags_S64Ptr = $SHBuiltin.extern_c({}, function igCheckboxFlags_S64Ptr(_label: c_ptr, _flags: c_ptr, _flags_value: c_longlong): c_bool { throw 0; });
const _igCheckboxFlags_U64Ptr = $SHBuiltin.extern_c({}, function igCheckboxFlags_U64Ptr(_label: c_ptr, _flags: c_ptr, _flags_value: c_ulonglong): c_bool { throw 0; });
const _igCloseButton = $SHBuiltin.extern_c({}, function igCloseButton_cwrap(_id: c_uint, _pos: c_ptr): c_bool { throw 0; });
const _igCollapseButton = $SHBuiltin.extern_c({}, function igCollapseButton_cwrap(_id: c_uint, _pos: c_ptr): c_bool { throw 0; });
const _igScrollbar = $SHBuiltin.extern_c({}, function igScrollbar(_axis: c_int): void { throw 0; });
const _igScrollbarEx = $SHBuiltin.extern_c({}, function igScrollbarEx_cwrap(_bb: c_ptr, _id: c_uint, _axis: c_int, _p_scroll_v: c_ptr, _avail_v: c_longlong, _contents_v: c_longlong, _flags: c_int): c_bool { throw 0; });
const _igGetWindowScrollbarRect = $SHBuiltin.extern_c({}, function igGetWindowScrollbarRect(_pOut: c_ptr, _window: c_ptr, _axis: c_int): void { throw 0; });
const _igGetWindowScrollbarID = $SHBuiltin.extern_c({}, function igGetWindowScrollbarID(_window: c_ptr, _axis: c_int): c_uint { throw 0; });
const _igGetWindowResizeCornerID = $SHBuiltin.extern_c({}, function igGetWindowResizeCornerID(_window: c_ptr, _n: c_int): c_uint { throw 0; });
const _igGetWindowResizeBorderID = $SHBuiltin.extern_c({}, function igGetWindowResizeBorderID(_window: c_ptr, _dir: c_int): c_uint { throw 0; });
const _igButtonBehavior = $SHBuiltin.extern_c({}, function igButtonBehavior_cwrap(_bb: c_ptr, _id: c_uint, _out_hovered: c_ptr, _out_held: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igDragBehavior = $SHBuiltin.extern_c({}, function igDragBehavior(_id: c_uint, _data_type: c_int, _p_v: c_ptr, _v_speed: c_float, _p_min: c_ptr, _p_max: c_ptr, _format: c_ptr, _flags: c_int): c_bool { throw 0; });
const _igSliderBehavior = $SHBuiltin.extern_c({}, function igSliderBehavior_cwrap(_bb: c_ptr, _id: c_uint, _data_type: c_int, _p_v: c_ptr, _p_min: c_ptr, _p_max: c_ptr, _format: c_ptr, _flags: c_int, _out_grab_bb: c_ptr): c_bool { throw 0; });
const _igSplitterBehavior = $SHBuiltin.extern_c({}, function igSplitterBehavior_cwrap(_bb: c_ptr, _id: c_uint, _axis: c_int, _size1: c_ptr, _size2: c_ptr, _min_size1: c_float, _min_size2: c_float, _hover_extend: c_float, _hover_visibility_delay: c_float, _bg_col: c_uint): c_bool { throw 0; });
const _igTreeNodeBehavior = $SHBuiltin.extern_c({}, function igTreeNodeBehavior(_id: c_uint, _flags: c_int, _label: c_ptr, _label_end: c_ptr): c_bool { throw 0; });
const _igTreePushOverrideID = $SHBuiltin.extern_c({}, function igTreePushOverrideID(_id: c_uint): void { throw 0; });
const _igTreeNodeSetOpen = $SHBuiltin.extern_c({}, function igTreeNodeSetOpen(_id: c_uint, _open: c_bool): void { throw 0; });
const _igTreeNodeUpdateNextOpen = $SHBuiltin.extern_c({}, function igTreeNodeUpdateNextOpen(_id: c_uint, _flags: c_int): c_bool { throw 0; });
const _igDataTypeGetInfo = $SHBuiltin.extern_c({}, function igDataTypeGetInfo(_data_type: c_int): c_ptr { throw 0; });
const _igDataTypeFormatString = $SHBuiltin.extern_c({}, function igDataTypeFormatString(_buf: c_ptr, _buf_size: c_int, _data_type: c_int, _p_data: c_ptr, _format: c_ptr): c_int { throw 0; });
const _igDataTypeApplyOp = $SHBuiltin.extern_c({}, function igDataTypeApplyOp(_data_type: c_int, _op: c_int, _output: c_ptr, _arg_1: c_ptr, _arg_2: c_ptr): void { throw 0; });
const _igDataTypeApplyFromText = $SHBuiltin.extern_c({}, function igDataTypeApplyFromText(_buf: c_ptr, _data_type: c_int, _p_data: c_ptr, _format: c_ptr): c_bool { throw 0; });
const _igDataTypeCompare = $SHBuiltin.extern_c({}, function igDataTypeCompare(_data_type: c_int, _arg_1: c_ptr, _arg_2: c_ptr): c_int { throw 0; });
const _igDataTypeClamp = $SHBuiltin.extern_c({}, function igDataTypeClamp(_data_type: c_int, _p_data: c_ptr, _p_min: c_ptr, _p_max: c_ptr): c_bool { throw 0; });
const _igInputTextEx = $SHBuiltin.extern_c({}, function igInputTextEx_cwrap(_label: c_ptr, _hint: c_ptr, _buf: c_ptr, _buf_size: c_int, _size_arg: c_ptr, _flags: c_int, _callback: c_ptr, _user_data: c_ptr): c_bool { throw 0; });
const _igInputTextDeactivateHook = $SHBuiltin.extern_c({}, function igInputTextDeactivateHook(_id: c_uint): void { throw 0; });
const _igTempInputText = $SHBuiltin.extern_c({}, function igTempInputText_cwrap(_bb: c_ptr, _id: c_uint, _label: c_ptr, _buf: c_ptr, _buf_size: c_int, _flags: c_int): c_bool { throw 0; });
const _igTempInputScalar = $SHBuiltin.extern_c({}, function igTempInputScalar_cwrap(_bb: c_ptr, _id: c_uint, _label: c_ptr, _data_type: c_int, _p_data: c_ptr, _format: c_ptr, _p_clamp_min: c_ptr, _p_clamp_max: c_ptr): c_bool { throw 0; });
const _igTempInputIsActive = $SHBuiltin.extern_c({}, function igTempInputIsActive(_id: c_uint): c_bool { throw 0; });
const _igGetInputTextState = $SHBuiltin.extern_c({}, function igGetInputTextState(_id: c_uint): c_ptr { throw 0; });
const _igColorTooltip = $SHBuiltin.extern_c({}, function igColorTooltip(_text: c_ptr, _col: c_ptr, _flags: c_int): void { throw 0; });
const _igColorEditOptionsPopup = $SHBuiltin.extern_c({}, function igColorEditOptionsPopup(_col: c_ptr, _flags: c_int): void { throw 0; });
const _igColorPickerOptionsPopup = $SHBuiltin.extern_c({}, function igColorPickerOptionsPopup(_ref_col: c_ptr, _flags: c_int): void { throw 0; });
const _igPlotEx = $SHBuiltin.extern_c({}, function igPlotEx_cwrap(_plot_type: c_int, _label: c_ptr, _values_getter: c_ptr, _data: c_ptr, _values_count: c_int, _values_offset: c_int, _overlay_text: c_ptr, _scale_min: c_float, _scale_max: c_float, _size_arg: c_ptr): c_int { throw 0; });
const _igShadeVertsLinearColorGradientKeepAlpha = $SHBuiltin.extern_c({}, function igShadeVertsLinearColorGradientKeepAlpha_cwrap(_draw_list: c_ptr, _vert_start_idx: c_int, _vert_end_idx: c_int, _gradient_p0: c_ptr, _gradient_p1: c_ptr, _col0: c_uint, _col1: c_uint): void { throw 0; });
const _igShadeVertsLinearUV = $SHBuiltin.extern_c({}, function igShadeVertsLinearUV_cwrap(_draw_list: c_ptr, _vert_start_idx: c_int, _vert_end_idx: c_int, _a: c_ptr, _b: c_ptr, _uv_a: c_ptr, _uv_b: c_ptr, _clamp: c_bool): void { throw 0; });
const _igGcCompactTransientMiscBuffers = $SHBuiltin.extern_c({}, function igGcCompactTransientMiscBuffers(): void { throw 0; });
const _igGcCompactTransientWindowBuffers = $SHBuiltin.extern_c({}, function igGcCompactTransientWindowBuffers(_window: c_ptr): void { throw 0; });
const _igGcAwakeTransientWindowBuffers = $SHBuiltin.extern_c({}, function igGcAwakeTransientWindowBuffers(_window: c_ptr): void { throw 0; });
const _igDebugLog = $SHBuiltin.extern_c({}, function igDebugLog(_fmt: c_ptr): void { throw 0; });
const _igDebugLogV = $SHBuiltin.extern_c({}, function igDebugLogV(_fmt: c_ptr, _args: c_ptr): void { throw 0; });
const _igErrorCheckEndFrameRecover = $SHBuiltin.extern_c({}, function igErrorCheckEndFrameRecover(_log_callback: c_ptr, _user_data: c_ptr): void { throw 0; });
const _igErrorCheckEndWindowRecover = $SHBuiltin.extern_c({}, function igErrorCheckEndWindowRecover(_log_callback: c_ptr, _user_data: c_ptr): void { throw 0; });
const _igErrorCheckUsingSetCursorPosToExtendParentBoundaries = $SHBuiltin.extern_c({}, function igErrorCheckUsingSetCursorPosToExtendParentBoundaries(): void { throw 0; });
const _igDebugDrawCursorPos = $SHBuiltin.extern_c({}, function igDebugDrawCursorPos(_col: c_uint): void { throw 0; });
const _igDebugDrawLineExtents = $SHBuiltin.extern_c({}, function igDebugDrawLineExtents(_col: c_uint): void { throw 0; });
const _igDebugDrawItemRect = $SHBuiltin.extern_c({}, function igDebugDrawItemRect(_col: c_uint): void { throw 0; });
const _igDebugLocateItem = $SHBuiltin.extern_c({}, function igDebugLocateItem(_target_id: c_uint): void { throw 0; });
const _igDebugLocateItemOnHover = $SHBuiltin.extern_c({}, function igDebugLocateItemOnHover(_target_id: c_uint): void { throw 0; });
const _igDebugLocateItemResolveWithLastItem = $SHBuiltin.extern_c({}, function igDebugLocateItemResolveWithLastItem(): void { throw 0; });
const _igDebugStartItemPicker = $SHBuiltin.extern_c({}, function igDebugStartItemPicker(): void { throw 0; });
const _igShowFontAtlas = $SHBuiltin.extern_c({}, function igShowFontAtlas(_atlas: c_ptr): void { throw 0; });
const _igDebugHookIdInfo = $SHBuiltin.extern_c({}, function igDebugHookIdInfo(_id: c_uint, _data_type: c_int, _data_id: c_ptr, _data_id_end: c_ptr): void { throw 0; });
const _igDebugNodeColumns = $SHBuiltin.extern_c({}, function igDebugNodeColumns(_columns: c_ptr): void { throw 0; });
const _igDebugNodeDrawList = $SHBuiltin.extern_c({}, function igDebugNodeDrawList(_window: c_ptr, _viewport: c_ptr, _draw_list: c_ptr, _label: c_ptr): void { throw 0; });
const _igDebugNodeDrawCmdShowMeshAndBoundingBox = $SHBuiltin.extern_c({}, function igDebugNodeDrawCmdShowMeshAndBoundingBox(_out_draw_list: c_ptr, _draw_list: c_ptr, _draw_cmd: c_ptr, _show_mesh: c_bool, _show_aabb: c_bool): void { throw 0; });
const _igDebugNodeFont = $SHBuiltin.extern_c({}, function igDebugNodeFont(_font: c_ptr): void { throw 0; });
const _igDebugNodeFontGlyph = $SHBuiltin.extern_c({}, function igDebugNodeFontGlyph(_font: c_ptr, _glyph: c_ptr): void { throw 0; });
const _igDebugNodeStorage = $SHBuiltin.extern_c({}, function igDebugNodeStorage(_storage: c_ptr, _label: c_ptr): void { throw 0; });
const _igDebugNodeTabBar = $SHBuiltin.extern_c({}, function igDebugNodeTabBar(_tab_bar: c_ptr, _label: c_ptr): void { throw 0; });
const _igDebugNodeTable = $SHBuiltin.extern_c({}, function igDebugNodeTable(_table: c_ptr): void { throw 0; });
const _igDebugNodeTableSettings = $SHBuiltin.extern_c({}, function igDebugNodeTableSettings(_settings: c_ptr): void { throw 0; });
const _igDebugNodeInputTextState = $SHBuiltin.extern_c({}, function igDebugNodeInputTextState(_state: c_ptr): void { throw 0; });
const _igDebugNodeWindow = $SHBuiltin.extern_c({}, function igDebugNodeWindow(_window: c_ptr, _label: c_ptr): void { throw 0; });
const _igDebugNodeWindowSettings = $SHBuiltin.extern_c({}, function igDebugNodeWindowSettings(_settings: c_ptr): void { throw 0; });
const _igDebugNodeWindowsList = $SHBuiltin.extern_c({}, function igDebugNodeWindowsList(_windows: c_ptr, _label: c_ptr): void { throw 0; });
const _igDebugNodeWindowsListByBeginStackParent = $SHBuiltin.extern_c({}, function igDebugNodeWindowsListByBeginStackParent(_windows: c_ptr, _windows_size: c_int, _parent_in_begin_stack: c_ptr): void { throw 0; });
const _igDebugNodeViewport = $SHBuiltin.extern_c({}, function igDebugNodeViewport(_viewport: c_ptr): void { throw 0; });
const _igDebugRenderKeyboardPreview = $SHBuiltin.extern_c({}, function igDebugRenderKeyboardPreview(_draw_list: c_ptr): void { throw 0; });
const _igDebugRenderViewportThumbnail = $SHBuiltin.extern_c({}, function igDebugRenderViewportThumbnail_cwrap(_draw_list: c_ptr, _viewport: c_ptr, _bb: c_ptr): void { throw 0; });
const _igIsKeyPressedMap = $SHBuiltin.extern_c({}, function igIsKeyPressedMap(_key: c_int, _repeat: c_bool): c_bool { throw 0; });
const _igImFontAtlasGetBuilderForStbTruetype = $SHBuiltin.extern_c({}, function igImFontAtlasGetBuilderForStbTruetype(): c_ptr { throw 0; });
const _igImFontAtlasBuildInit = $SHBuiltin.extern_c({}, function igImFontAtlasBuildInit(_atlas: c_ptr): void { throw 0; });
const _igImFontAtlasBuildSetupFont = $SHBuiltin.extern_c({}, function igImFontAtlasBuildSetupFont(_atlas: c_ptr, _font: c_ptr, _font_config: c_ptr, _ascent: c_float, _descent: c_float): void { throw 0; });
const _igImFontAtlasBuildPackCustomRects = $SHBuiltin.extern_c({}, function igImFontAtlasBuildPackCustomRects(_atlas: c_ptr, _stbrp_context_opaque: c_ptr): void { throw 0; });
const _igImFontAtlasBuildFinish = $SHBuiltin.extern_c({}, function igImFontAtlasBuildFinish(_atlas: c_ptr): void { throw 0; });
const _igImFontAtlasBuildRender8bppRectFromString = $SHBuiltin.extern_c({}, function igImFontAtlasBuildRender8bppRectFromString(_atlas: c_ptr, _x: c_int, _y: c_int, _w: c_int, _h: c_int, _in_str: c_ptr, _in_marker_char: c_char, _in_marker_pixel_value: c_uchar): void { throw 0; });
const _igImFontAtlasBuildRender32bppRectFromString = $SHBuiltin.extern_c({}, function igImFontAtlasBuildRender32bppRectFromString(_atlas: c_ptr, _x: c_int, _y: c_int, _w: c_int, _h: c_int, _in_str: c_ptr, _in_marker_char: c_char, _in_marker_pixel_value: c_uint): void { throw 0; });
const _igImFontAtlasBuildMultiplyCalcLookupTable = $SHBuiltin.extern_c({}, function igImFontAtlasBuildMultiplyCalcLookupTable(_out_table: c_ptr, _in_multiply_factor: c_float): void { throw 0; });
const _igImFontAtlasBuildMultiplyRectAlpha8 = $SHBuiltin.extern_c({}, function igImFontAtlasBuildMultiplyRectAlpha8(_table: c_ptr, _pixels: c_ptr, _x: c_int, _y: c_int, _w: c_int, _h: c_int, _stride: c_int): void { throw 0; });
const _igLogText = $SHBuiltin.extern_c({}, function igLogText(_fmt: c_ptr): void { throw 0; });
const _ImGuiTextBuffer_appendf = $SHBuiltin.extern_c({}, function ImGuiTextBuffer_appendf(_buffer: c_ptr, _fmt: c_ptr): void { throw 0; });
const _igGET_FLT_MAX = $SHBuiltin.extern_c({}, function igGET_FLT_MAX(): c_float { throw 0; });
const _igGET_FLT_MIN = $SHBuiltin.extern_c({}, function igGET_FLT_MIN(): c_float { throw 0; });
const _ImVector_ImWchar_create = $SHBuiltin.extern_c({}, function ImVector_ImWchar_create(): c_ptr { throw 0; });
const _ImVector_ImWchar_destroy = $SHBuiltin.extern_c({}, function ImVector_ImWchar_destroy(_self: c_ptr): void { throw 0; });
const _ImVector_ImWchar_Init = $SHBuiltin.extern_c({}, function ImVector_ImWchar_Init(_p: c_ptr): void { throw 0; });
const _ImVector_ImWchar_UnInit = $SHBuiltin.extern_c({}, function ImVector_ImWchar_UnInit(_p: c_ptr): void { throw 0; });
const _simgui_setup = $SHBuiltin.extern_c({}, function simgui_setup(_desc: c_ptr): void { throw 0; });
const _simgui_new_frame = $SHBuiltin.extern_c({}, function simgui_new_frame(_desc: c_ptr): void { throw 0; });
const _simgui_render = $SHBuiltin.extern_c({}, function simgui_render(): void { throw 0; });
const _simgui_make_image = $SHBuiltin.extern_c({}, function simgui_make_image_cwrap(_out: c_ptr, _desc: c_ptr): void { throw 0; });
const _simgui_destroy_image = $SHBuiltin.extern_c({}, function simgui_destroy_image_cwrap(_img: c_ptr): void { throw 0; });
const _simgui_query_image_desc = $SHBuiltin.extern_c({}, function simgui_query_image_desc_cwrap(_out: c_ptr, _img: c_ptr): void { throw 0; });
const _simgui_imtextureid = $SHBuiltin.extern_c({}, function simgui_imtextureid_cwrap(_img: c_ptr): c_ptr { throw 0; });
const _simgui_image_from_imtextureid = $SHBuiltin.extern_c({}, function simgui_image_from_imtextureid_cwrap(_out: c_ptr, _imtextureid: c_ptr): void { throw 0; });
const _simgui_add_focus_event = $SHBuiltin.extern_c({}, function simgui_add_focus_event(_focus: c_bool): void { throw 0; });
const _simgui_add_mouse_pos_event = $SHBuiltin.extern_c({}, function simgui_add_mouse_pos_event(_x: c_float, _y: c_float): void { throw 0; });
const _simgui_add_touch_pos_event = $SHBuiltin.extern_c({}, function simgui_add_touch_pos_event(_x: c_float, _y: c_float): void { throw 0; });
const _simgui_add_mouse_button_event = $SHBuiltin.extern_c({}, function simgui_add_mouse_button_event(_mouse_button: c_int, _down: c_bool): void { throw 0; });
const _simgui_add_mouse_wheel_event = $SHBuiltin.extern_c({}, function simgui_add_mouse_wheel_event(_wheel_x: c_float, _wheel_y: c_float): void { throw 0; });
const _simgui_add_key_event = $SHBuiltin.extern_c({}, function simgui_add_key_event(_map_keycode: c_ptr, _keycode: c_int, _down: c_bool): void { throw 0; });
const _simgui_add_input_character = $SHBuiltin.extern_c({}, function simgui_add_input_character(_c: c_uint): void { throw 0; });
const _simgui_add_input_characters_utf8 = $SHBuiltin.extern_c({}, function simgui_add_input_characters_utf8(_c: c_ptr): void { throw 0; });
const _simgui_add_touch_button_event = $SHBuiltin.extern_c({}, function simgui_add_touch_button_event(_mouse_button: c_int, _down: c_bool): void { throw 0; });
const _simgui_shutdown = $SHBuiltin.extern_c({}, function simgui_shutdown(): void { throw 0; });
const _stm_setup = $SHBuiltin.extern_c({}, function stm_setup(): void { throw 0; });
const _stm_now = $SHBuiltin.extern_c({}, function stm_now(): c_ulonglong { throw 0; });
const _stm_diff = $SHBuiltin.extern_c({}, function stm_diff(_new_ticks: c_ulonglong, _old_ticks: c_ulonglong): c_ulonglong { throw 0; });
const _stm_since = $SHBuiltin.extern_c({}, function stm_since(_start_ticks: c_ulonglong): c_ulonglong { throw 0; });
const _stm_laptime = $SHBuiltin.extern_c({}, function stm_laptime(_last_time: c_ptr): c_ulonglong { throw 0; });
const _stm_round_to_common_refresh_rate = $SHBuiltin.extern_c({}, function stm_round_to_common_refresh_rate(_frame_ticks: c_ulonglong): c_ulonglong { throw 0; });
const _stm_sec = $SHBuiltin.extern_c({}, function stm_sec(_ticks: c_ulonglong): c_double { throw 0; });
const _stm_ms = $SHBuiltin.extern_c({}, function stm_ms(_ticks: c_ulonglong): c_double { throw 0; });
const _stm_us = $SHBuiltin.extern_c({}, function stm_us(_ticks: c_ulonglong): c_double { throw 0; });
const _stm_ns = $SHBuiltin.extern_c({}, function stm_ns(_ticks: c_ulonglong): c_double { throw 0; });
const _sizeof_ImDrawChannel = 32;
const _sizeof_ImDrawCmd = 56;
const _sizeof_ImDrawData = 64;
const _sizeof_ImDrawList = 200;
const _sizeof_ImDrawListSharedData = 528;
const _sizeof_ImDrawListSplitter = 24;
const _sizeof_ImDrawVert = 20;
const _sizeof_ImFont = 120;
const _sizeof_ImFontAtlas = 1184;
const _sizeof_ImFontBuilderIO = 8;
const _sizeof_ImFontConfig = 136;
const _sizeof_ImFontGlyph = 40;
const _sizeof_ImFontGlyphRangesBuilder = 16;
const _sizeof_ImColor = 16;
const _sizeof_ImGuiContext = 24184;
const _sizeof_ImGuiIO = 14272;
const _sizeof_ImGuiInputTextCallbackData = 64;
const _sizeof_ImGuiKeyData = 16;
const _sizeof_ImGuiListClipper = 40;
const _sizeof_ImGuiOnceUponAFrame = 4;
const _sizeof_ImGuiPayload = 64;
const _sizeof_ImGuiPlatformImeData = 16;
const _sizeof_ImGuiSizeCallbackData = 32;
const _sizeof_ImGuiStorage = 16;
const _sizeof_ImGuiStyle = 1088;
const _sizeof_ImGuiTableSortSpecs = 16;
const _sizeof_ImGuiTableColumnSortSpecs = 12;
const _sizeof_ImGuiTextBuffer = 16;
const _sizeof_ImGuiTextFilter = 280;
const _sizeof_ImGuiViewport = 48;
const _sizeof_ImBitVector = 16;
const _sizeof_ImRect = 16;
const _sizeof_ImDrawDataBuilder = 32;
const _sizeof_ImGuiColorMod = 20;
const _sizeof_ImGuiContextHook = 32;
const _sizeof_ImGuiDataVarInfo = 12;
const _sizeof_ImGuiDataTypeInfo = 32;
const _sizeof_ImGuiGroupData = 48;
const _sizeof_ImGuiInputTextState = 3728;
const _sizeof_ImGuiLastItemData = 60;
const _sizeof_ImGuiLocEntry = 16;
const _sizeof_ImGuiMenuColumns = 28;
const _sizeof_ImGuiNavItemData = 48;
const _sizeof_ImGuiNavTreeNodeData = 24;
const _sizeof_ImGuiMetricsConfig = 16;
const _sizeof_ImGuiNextWindowData = 112;
const _sizeof_ImGuiNextItemData = 24;
const _sizeof_ImGuiOldColumnData = 28;
const _sizeof_ImGuiOldColumns = 136;
const _sizeof_ImGuiPopupData = 56;
const _sizeof_ImGuiSettingsHandler = 72;
const _sizeof_ImGuiStackSizes = 18;
const _sizeof_ImGuiStyleMod = 12;
const _sizeof_ImGuiTabBar = 152;
const _sizeof_ImGuiTabItem = 44;
const _sizeof_ImGuiTable = 576;
const _sizeof_ImGuiTableColumn = 112;
const _sizeof_ImGuiTableInstanceData = 24;
const _sizeof_ImGuiTableTempData = 112;
const _sizeof_ImGuiTableSettings = 20;
const _sizeof_ImGuiWindow = 1000;
const _sizeof_ImGuiWindowTempData = 232;
const _sizeof_ImGuiWindowSettings = 16;
const _sizeof_ImVec2 = 8;
const _sizeof_ImVec4 = 16;
const _sizeof_ImVector_ImWchar = 16;
const _sizeof_ImGuiTextRange = 16;
const _sizeof_ImVector_ImGuiTextRange = 16;
const _sizeof_ImVector_char = 16;
const _sizeof_ImGuiStoragePair = 16;
const _sizeof_ImVector_ImGuiStoragePair = 16;
const _sizeof_ImDrawCmdHeader = 32;
const _sizeof_ImVector_ImDrawCmd = 16;
const _sizeof_ImVector_ImDrawIdx = 16;
const _sizeof_ImVector_ImDrawChannel = 16;
const _sizeof_ImVector_ImDrawVert = 16;
const _sizeof_ImVector_ImVec4 = 16;
const _sizeof_ImVector_ImTextureID = 16;
const _sizeof_ImVector_ImVec2 = 16;
const _sizeof_ImVector_ImDrawListPtr = 16;
const _sizeof_ImVector_ImU32 = 16;
const _sizeof_ImFontAtlasCustomRect = 32;
const _sizeof_ImVector_ImFontPtr = 16;
const _sizeof_ImVector_ImFontAtlasCustomRect = 16;
const _sizeof_ImVector_ImFontConfig = 16;
const _sizeof_ImVector_float = 16;
const _sizeof_ImVector_ImFontGlyph = 16;
const _sizeof_StbUndoRecord = 16;
const _sizeof_StbUndoState = 3596;
const _sizeof_STB_TexteditState = 3628;
const _sizeof_StbTexteditRow = 24;
const _sizeof_ImVec1 = 4;
const _sizeof_ImVec2ih = 4;
const _sizeof_ImGuiTextIndex = 24;
const _sizeof_ImVector_int = 16;
const _sizeof_ImGuiDataTypeTempStorage = 8;
const _sizeof_ImGuiComboPreviewData = 48;
const _sizeof_ImGuiInputTextDeactivatedState = 24;
const _sizeof_ImGuiWindowStackData = 88;
const _sizeof_ImGuiShrinkWidthItem = 12;
const _sizeof_ImGuiPtrOrIndex = 16;
const _sizeof_ImBitArray_ImGuiKey_NamedKey_COUNT__lessImGuiKey_NamedKey_BEGIN = 20;
const _sizeof_ImGuiInputEventMousePos = 12;
const _sizeof_ImGuiInputEventMouseWheel = 12;
const _sizeof_ImGuiInputEventMouseButton = 12;
const _sizeof_ImGuiInputEventKey = 12;
const _sizeof_ImGuiInputEventText = 4;
const _sizeof_ImGuiInputEventAppFocused = 1;
const _sizeof_ImGuiInputEvent = 28;
const _sizeof_ImGuiKeyRoutingData = 16;
const _sizeof_ImGuiKeyRoutingTable = 312;
const _sizeof_ImVector_ImGuiKeyRoutingData = 16;
const _sizeof_ImGuiKeyOwnerData = 12;
const _sizeof_ImGuiListClipperRange = 12;
const _sizeof_ImGuiListClipperData = 40;
const _sizeof_ImVector_ImGuiListClipperRange = 16;
const _sizeof_ImVector_ImGuiOldColumnData = 16;
const _sizeof_ImGuiViewportP = 200;
const _sizeof_ImGuiStackLevelInfo = 64;
const _sizeof_ImGuiStackTool = 40;
const _sizeof_ImVector_ImGuiStackLevelInfo = 16;
const _sizeof_ImVector_ImGuiInputEvent = 16;
const _sizeof_ImVector_ImGuiWindowPtr = 16;
const _sizeof_ImVector_ImGuiWindowStackData = 16;
const _sizeof_ImVector_ImGuiColorMod = 16;
const _sizeof_ImVector_ImGuiStyleMod = 16;
const _sizeof_ImVector_ImGuiID = 16;
const _sizeof_ImVector_ImGuiItemFlags = 16;
const _sizeof_ImVector_ImGuiGroupData = 16;
const _sizeof_ImVector_ImGuiPopupData = 16;
const _sizeof_ImVector_ImGuiNavTreeNodeData = 16;
const _sizeof_ImVector_ImGuiViewportPPtr = 16;
const _sizeof_ImVector_unsigned_char = 16;
const _sizeof_ImVector_ImGuiListClipperData = 16;
const _sizeof_ImVector_ImGuiTableTempData = 16;
const _sizeof_ImVector_ImGuiTable = 16;
const _sizeof_ImPool_ImGuiTable = 40;
const _sizeof_ImVector_ImGuiTabBar = 16;
const _sizeof_ImPool_ImGuiTabBar = 40;
const _sizeof_ImVector_ImGuiPtrOrIndex = 16;
const _sizeof_ImVector_ImGuiShrinkWidthItem = 16;
const _sizeof_ImVector_ImGuiSettingsHandler = 16;
const _sizeof_ImChunkStream_ImGuiWindowSettings = 16;
const _sizeof_ImChunkStream_ImGuiTableSettings = 16;
const _sizeof_ImVector_ImGuiContextHook = 16;
const _sizeof_ImVector_ImGuiOldColumns = 16;
const _sizeof_ImVector_ImGuiTabItem = 16;
const _sizeof_ImGuiTableCellData = 8;
const _sizeof_ImSpan_ImGuiTableColumn = 16;
const _sizeof_ImSpan_ImGuiTableColumnIdx = 16;
const _sizeof_ImSpan_ImGuiTableCellData = 16;
const _sizeof_ImVector_ImGuiTableInstanceData = 16;
const _sizeof_ImVector_ImGuiTableColumnSortSpecs = 16;
const _sizeof_ImGuiTableColumnSettings = 16;
const _sizeof_simgui_image_t = 4;
const _sizeof_simgui_allocator_t = 24;
const _sizeof_simgui_logger_t = 16;
const _sizeof_simgui_frame_desc_t = 24;
const _sizeof_simgui_image_desc_t = 1;
const _sizeof_simgui_desc_t = 1;
function get_ImDrawChannel__CmdBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImDrawChannel__IdxBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImDrawCmd_ClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImDrawCmd_TextureId(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16);
}
function set_ImDrawCmd_TextureId(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16, v);
}
function get_ImDrawCmd_VtxOffset(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 24);
}
function set_ImDrawCmd_VtxOffset(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 24, v);
}
function get_ImDrawCmd_IdxOffset(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 28);
}
function set_ImDrawCmd_IdxOffset(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 28, v);
}
function get_ImDrawCmd_ElemCount(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 32);
}
function set_ImDrawCmd_ElemCount(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 32, v);
}
function get_ImDrawCmd_UserCallback(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 40);
}
function set_ImDrawCmd_UserCallback(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 40, v);
}
function get_ImDrawCmd_UserCallbackData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 48);
}
function set_ImDrawCmd_UserCallbackData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 48, v);
}
function get_ImDrawData_Valid(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 0);
}
function set_ImDrawData_Valid(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 0, v);
}
function get_ImDrawData_CmdListsCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImDrawData_CmdListsCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImDrawData_TotalIdxCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImDrawData_TotalIdxCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImDrawData_TotalVtxCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 12);
}
function set_ImDrawData_TotalVtxCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 12, v);
}
function get_ImDrawData_CmdLists(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImDrawData_DisplayPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 32);
}
function get_ImDrawData_DisplaySize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 40);
}
function get_ImDrawData_FramebufferScale(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 48);
}
function get_ImDrawData_OwnerViewport(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 56);
}
function set_ImDrawData_OwnerViewport(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 56, v);
}
function get_ImDrawList_CmdBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImDrawList_IdxBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImDrawList_VtxBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 32);
}
function get_ImDrawList_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 48);
}
function set_ImDrawList_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 48, v);
}
function get_ImDrawList__VtxCurrentIdx(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 52);
}
function set_ImDrawList__VtxCurrentIdx(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 52, v);
}
function get_ImDrawList__Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 56);
}
function set_ImDrawList__Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 56, v);
}
function get_ImDrawList__OwnerName(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 64);
}
function set_ImDrawList__OwnerName(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 64, v);
}
function get_ImDrawList__VtxWritePtr(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 72);
}
function set_ImDrawList__VtxWritePtr(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 72, v);
}
function get_ImDrawList__IdxWritePtr(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 80);
}
function set_ImDrawList__IdxWritePtr(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 80, v);
}
function get_ImDrawList__ClipRectStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 88);
}
function get_ImDrawList__TextureIdStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 104);
}
function get_ImDrawList__Path(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 120);
}
function get_ImDrawList__CmdHeader(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 136);
}
function get_ImDrawList__Splitter(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 168);
}
function get_ImDrawList__FringeScale(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 192);
}
function set_ImDrawList__FringeScale(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 192, v);
}
function get_ImDrawListSharedData_TexUvWhitePixel(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImDrawListSharedData_Font(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImDrawListSharedData_Font(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImDrawListSharedData_FontSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16);
}
function set_ImDrawListSharedData_FontSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16, v);
}
function get_ImDrawListSharedData_CurveTessellationTol(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 20);
}
function set_ImDrawListSharedData_CurveTessellationTol(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 20, v);
}
function get_ImDrawListSharedData_CircleSegmentMaxError(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 24);
}
function set_ImDrawListSharedData_CircleSegmentMaxError(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 24, v);
}
function get_ImDrawListSharedData_ClipRectFullscreen(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 28);
}
function get_ImDrawListSharedData_InitialFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 44);
}
function set_ImDrawListSharedData_InitialFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 44, v);
}
function get_ImDrawListSharedData_TempBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 48);
}
function get_ImDrawListSharedData_ArcFastVtx(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 64);
}
function set_ImDrawListSharedData_ArcFastVtx(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 64, v);
}
function get_ImDrawListSharedData_ArcFastRadiusCutoff(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 448);
}
function set_ImDrawListSharedData_ArcFastRadiusCutoff(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 448, v);
}
function get_ImDrawListSharedData_CircleSegmentCounts(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 452);
}
function set_ImDrawListSharedData_CircleSegmentCounts(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 452, v);
}
function get_ImDrawListSharedData_TexUvLines(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 520);
}
function set_ImDrawListSharedData_TexUvLines(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 520, v);
}
function get_ImDrawListSplitter__Current(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImDrawListSplitter__Current(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImDrawListSplitter__Count(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImDrawListSplitter__Count(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImDrawListSplitter__Channels(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImDrawVert_pos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImDrawVert_uv(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImDrawVert_col(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 16);
}
function set_ImDrawVert_col(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 16, v);
}
function get_ImFont_IndexAdvanceX(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImFont_FallbackAdvanceX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16);
}
function set_ImFont_FallbackAdvanceX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16, v);
}
function get_ImFont_FontSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 20);
}
function set_ImFont_FontSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 20, v);
}
function get_ImFont_IndexLookup(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24);
}
function get_ImFont_Glyphs(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 40);
}
function get_ImFont_FallbackGlyph(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 56);
}
function set_ImFont_FallbackGlyph(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 56, v);
}
function get_ImFont_ContainerAtlas(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 64);
}
function set_ImFont_ContainerAtlas(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 64, v);
}
function get_ImFont_ConfigData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 72);
}
function set_ImFont_ConfigData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 72, v);
}
function get_ImFont_ConfigDataCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 80);
}
function set_ImFont_ConfigDataCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 80, v);
}
function get_ImFont_FallbackChar(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 82);
}
function set_ImFont_FallbackChar(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 82, v);
}
function get_ImFont_EllipsisChar(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 84);
}
function set_ImFont_EllipsisChar(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 84, v);
}
function get_ImFont_EllipsisCharCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 86);
}
function set_ImFont_EllipsisCharCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 86, v);
}
function get_ImFont_EllipsisWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 88);
}
function set_ImFont_EllipsisWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 88, v);
}
function get_ImFont_EllipsisCharStep(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 92);
}
function set_ImFont_EllipsisCharStep(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 92, v);
}
function get_ImFont_DirtyLookupTables(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 96);
}
function set_ImFont_DirtyLookupTables(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 96, v);
}
function get_ImFont_Scale(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 100);
}
function set_ImFont_Scale(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 100, v);
}
function get_ImFont_Ascent(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 104);
}
function set_ImFont_Ascent(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 104, v);
}
function get_ImFont_Descent(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 108);
}
function set_ImFont_Descent(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 108, v);
}
function get_ImFont_MetricsTotalSurface(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 112);
}
function set_ImFont_MetricsTotalSurface(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 112, v);
}
function get_ImFont_Used4kPagesMap(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 116);
}
function set_ImFont_Used4kPagesMap(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 116, v);
}
function get_ImFontAtlas_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImFontAtlas_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImFontAtlas_TexID(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImFontAtlas_TexID(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImFontAtlas_TexDesiredWidth(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_ImFontAtlas_TexDesiredWidth(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_ImFontAtlas_TexGlyphPadding(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 20);
}
function set_ImFontAtlas_TexGlyphPadding(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 20, v);
}
function get_ImFontAtlas_Locked(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 24);
}
function set_ImFontAtlas_Locked(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 24, v);
}
function get_ImFontAtlas_UserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 32);
}
function set_ImFontAtlas_UserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 32, v);
}
function get_ImFontAtlas_TexReady(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 40);
}
function set_ImFontAtlas_TexReady(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 40, v);
}
function get_ImFontAtlas_TexPixelsUseColors(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 41);
}
function set_ImFontAtlas_TexPixelsUseColors(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 41, v);
}
function get_ImFontAtlas_TexPixelsAlpha8(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 48);
}
function set_ImFontAtlas_TexPixelsAlpha8(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 48, v);
}
function get_ImFontAtlas_TexPixelsRGBA32(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 56);
}
function set_ImFontAtlas_TexPixelsRGBA32(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 56, v);
}
function get_ImFontAtlas_TexWidth(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 64);
}
function set_ImFontAtlas_TexWidth(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 64, v);
}
function get_ImFontAtlas_TexHeight(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 68);
}
function set_ImFontAtlas_TexHeight(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 68, v);
}
function get_ImFontAtlas_TexUvScale(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 72);
}
function get_ImFontAtlas_TexUvWhitePixel(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 80);
}
function get_ImFontAtlas_Fonts(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 88);
}
function get_ImFontAtlas_CustomRects(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 104);
}
function get_ImFontAtlas_ConfigData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 120);
}
function get_ImFontAtlas_TexUvLines(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 136);
}
function set_ImFontAtlas_TexUvLines(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 136, v);
}
function get_ImFontAtlas_FontBuilderIO(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 1160);
}
function set_ImFontAtlas_FontBuilderIO(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 1160, v);
}
function get_ImFontAtlas_FontBuilderFlags(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 1168);
}
function set_ImFontAtlas_FontBuilderFlags(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 1168, v);
}
function get_ImFontAtlas_PackIdMouseCursors(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 1172);
}
function set_ImFontAtlas_PackIdMouseCursors(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 1172, v);
}
function get_ImFontAtlas_PackIdLines(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 1176);
}
function set_ImFontAtlas_PackIdLines(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 1176, v);
}
function get_ImFontBuilderIO_FontBuilder_Build(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImFontBuilderIO_FontBuilder_Build(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImFontConfig_FontData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImFontConfig_FontData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImFontConfig_FontDataSize(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImFontConfig_FontDataSize(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImFontConfig_FontDataOwnedByAtlas(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 12);
}
function set_ImFontConfig_FontDataOwnedByAtlas(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 12, v);
}
function get_ImFontConfig_FontNo(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_ImFontConfig_FontNo(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_ImFontConfig_SizePixels(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 20);
}
function set_ImFontConfig_SizePixels(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 20, v);
}
function get_ImFontConfig_OversampleH(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 24);
}
function set_ImFontConfig_OversampleH(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 24, v);
}
function get_ImFontConfig_OversampleV(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 28);
}
function set_ImFontConfig_OversampleV(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 28, v);
}
function get_ImFontConfig_PixelSnapH(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 32);
}
function set_ImFontConfig_PixelSnapH(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 32, v);
}
function get_ImFontConfig_GlyphExtraSpacing(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 36);
}
function get_ImFontConfig_GlyphOffset(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 44);
}
function get_ImFontConfig_GlyphRanges(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 56);
}
function set_ImFontConfig_GlyphRanges(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 56, v);
}
function get_ImFontConfig_GlyphMinAdvanceX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 64);
}
function set_ImFontConfig_GlyphMinAdvanceX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 64, v);
}
function get_ImFontConfig_GlyphMaxAdvanceX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 68);
}
function set_ImFontConfig_GlyphMaxAdvanceX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 68, v);
}
function get_ImFontConfig_MergeMode(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 72);
}
function set_ImFontConfig_MergeMode(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 72, v);
}
function get_ImFontConfig_FontBuilderFlags(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 76);
}
function set_ImFontConfig_FontBuilderFlags(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 76, v);
}
function get_ImFontConfig_RasterizerMultiply(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 80);
}
function set_ImFontConfig_RasterizerMultiply(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 80, v);
}
function get_ImFontConfig_EllipsisChar(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 84);
}
function set_ImFontConfig_EllipsisChar(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 84, v);
}
function get_ImFontConfig_Name(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 86);
}
function set_ImFontConfig_Name(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 86, v);
}
function get_ImFontConfig_DstFont(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 128);
}
function set_ImFontConfig_DstFont(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 128, v);
}
function get_ImFontGlyph_Colored(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImFontGlyph_Colored(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImFontGlyph_Visible(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImFontGlyph_Visible(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImFontGlyph_Codepoint(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImFontGlyph_Codepoint(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImFontGlyph_AdvanceX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImFontGlyph_AdvanceX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImFontGlyph_X0(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_ImFontGlyph_X0(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_ImFontGlyph_Y0(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 12);
}
function set_ImFontGlyph_Y0(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 12, v);
}
function get_ImFontGlyph_X1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16);
}
function set_ImFontGlyph_X1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16, v);
}
function get_ImFontGlyph_Y1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 20);
}
function set_ImFontGlyph_Y1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 20, v);
}
function get_ImFontGlyph_U0(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 24);
}
function set_ImFontGlyph_U0(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 24, v);
}
function get_ImFontGlyph_V0(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 28);
}
function set_ImFontGlyph_V0(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 28, v);
}
function get_ImFontGlyph_U1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 32);
}
function set_ImFontGlyph_U1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 32, v);
}
function get_ImFontGlyph_V1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 36);
}
function set_ImFontGlyph_V1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 36, v);
}
function get_ImFontGlyphRangesBuilder_UsedChars(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImColor_Value(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImGuiContext_Initialized(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 0);
}
function set_ImGuiContext_Initialized(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 0, v);
}
function get_ImGuiContext_FontAtlasOwnedByContext(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 1);
}
function set_ImGuiContext_FontAtlasOwnedByContext(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 1, v);
}
function get_ImGuiContext_IO(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImGuiContext_Style(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 14280);
}
function get_ImGuiContext_Font(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 15368);
}
function set_ImGuiContext_Font(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 15368, v);
}
function get_ImGuiContext_FontSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 15376);
}
function set_ImGuiContext_FontSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 15376, v);
}
function get_ImGuiContext_FontBaseSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 15380);
}
function set_ImGuiContext_FontBaseSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 15380, v);
}
function get_ImGuiContext_DrawListSharedData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 15384);
}
function get_ImGuiContext_Time(s: c_ptr): c_double {
  "inline";
  return _sh_ptr_read_c_double(s, 15912);
}
function set_ImGuiContext_Time(s: c_ptr, v: c_double): void {
  "inline";
  _sh_ptr_write_c_double(s, 15912, v);
}
function get_ImGuiContext_FrameCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 15920);
}
function set_ImGuiContext_FrameCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 15920, v);
}
function get_ImGuiContext_FrameCountEnded(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 15924);
}
function set_ImGuiContext_FrameCountEnded(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 15924, v);
}
function get_ImGuiContext_FrameCountRendered(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 15928);
}
function set_ImGuiContext_FrameCountRendered(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 15928, v);
}
function get_ImGuiContext_WithinFrameScope(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 15932);
}
function set_ImGuiContext_WithinFrameScope(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 15932, v);
}
function get_ImGuiContext_WithinFrameScopeWithImplicitWindow(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 15933);
}
function set_ImGuiContext_WithinFrameScopeWithImplicitWindow(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 15933, v);
}
function get_ImGuiContext_WithinEndChild(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 15934);
}
function set_ImGuiContext_WithinEndChild(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 15934, v);
}
function get_ImGuiContext_GcCompactAll(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 15935);
}
function set_ImGuiContext_GcCompactAll(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 15935, v);
}
function get_ImGuiContext_TestEngineHookItems(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 15936);
}
function set_ImGuiContext_TestEngineHookItems(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 15936, v);
}
function get_ImGuiContext_TestEngine(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 15944);
}
function set_ImGuiContext_TestEngine(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 15944, v);
}
function get_ImGuiContext_InputEventsQueue(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 15952);
}
function get_ImGuiContext_InputEventsTrail(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 15968);
}
function get_ImGuiContext_InputEventsNextMouseSource(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 15984);
}
function set_ImGuiContext_InputEventsNextMouseSource(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 15984, v);
}
function get_ImGuiContext_InputEventsNextEventId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 15988);
}
function set_ImGuiContext_InputEventsNextEventId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 15988, v);
}
function get_ImGuiContext_Windows(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 15992);
}
function get_ImGuiContext_WindowsFocusOrder(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16008);
}
function get_ImGuiContext_WindowsTempSortBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16024);
}
function get_ImGuiContext_CurrentWindowStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16040);
}
function get_ImGuiContext_WindowsById(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16056);
}
function get_ImGuiContext_WindowsActiveCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16072);
}
function set_ImGuiContext_WindowsActiveCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16072, v);
}
function get_ImGuiContext_WindowsHoverPadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16076);
}
function get_ImGuiContext_CurrentWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16088);
}
function set_ImGuiContext_CurrentWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16088, v);
}
function get_ImGuiContext_HoveredWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16096);
}
function set_ImGuiContext_HoveredWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16096, v);
}
function get_ImGuiContext_HoveredWindowUnderMovingWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16104);
}
function set_ImGuiContext_HoveredWindowUnderMovingWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16104, v);
}
function get_ImGuiContext_MovingWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16112);
}
function set_ImGuiContext_MovingWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16112, v);
}
function get_ImGuiContext_WheelingWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16120);
}
function set_ImGuiContext_WheelingWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16120, v);
}
function get_ImGuiContext_WheelingWindowRefMousePos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16128);
}
function get_ImGuiContext_WheelingWindowStartFrame(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16136);
}
function set_ImGuiContext_WheelingWindowStartFrame(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16136, v);
}
function get_ImGuiContext_WheelingWindowReleaseTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16140);
}
function set_ImGuiContext_WheelingWindowReleaseTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16140, v);
}
function get_ImGuiContext_WheelingWindowWheelRemainder(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16144);
}
function get_ImGuiContext_WheelingAxisAvg(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16152);
}
function get_ImGuiContext_DebugHookIdInfo(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 16160);
}
function set_ImGuiContext_DebugHookIdInfo(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 16160, v);
}
function get_ImGuiContext_HoveredId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 16164);
}
function set_ImGuiContext_HoveredId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 16164, v);
}
function get_ImGuiContext_HoveredIdPreviousFrame(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 16168);
}
function set_ImGuiContext_HoveredIdPreviousFrame(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 16168, v);
}
function get_ImGuiContext_HoveredIdAllowOverlap(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16172);
}
function set_ImGuiContext_HoveredIdAllowOverlap(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16172, v);
}
function get_ImGuiContext_HoveredIdDisabled(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16173);
}
function set_ImGuiContext_HoveredIdDisabled(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16173, v);
}
function get_ImGuiContext_HoveredIdTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16176);
}
function set_ImGuiContext_HoveredIdTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16176, v);
}
function get_ImGuiContext_HoveredIdNotActiveTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16180);
}
function set_ImGuiContext_HoveredIdNotActiveTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16180, v);
}
function get_ImGuiContext_ActiveId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 16184);
}
function set_ImGuiContext_ActiveId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 16184, v);
}
function get_ImGuiContext_ActiveIdIsAlive(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 16188);
}
function set_ImGuiContext_ActiveIdIsAlive(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 16188, v);
}
function get_ImGuiContext_ActiveIdTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16192);
}
function set_ImGuiContext_ActiveIdTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16192, v);
}
function get_ImGuiContext_ActiveIdIsJustActivated(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16196);
}
function set_ImGuiContext_ActiveIdIsJustActivated(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16196, v);
}
function get_ImGuiContext_ActiveIdAllowOverlap(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16197);
}
function set_ImGuiContext_ActiveIdAllowOverlap(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16197, v);
}
function get_ImGuiContext_ActiveIdNoClearOnFocusLoss(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16198);
}
function set_ImGuiContext_ActiveIdNoClearOnFocusLoss(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16198, v);
}
function get_ImGuiContext_ActiveIdHasBeenPressedBefore(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16199);
}
function set_ImGuiContext_ActiveIdHasBeenPressedBefore(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16199, v);
}
function get_ImGuiContext_ActiveIdHasBeenEditedBefore(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16200);
}
function set_ImGuiContext_ActiveIdHasBeenEditedBefore(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16200, v);
}
function get_ImGuiContext_ActiveIdHasBeenEditedThisFrame(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16201);
}
function set_ImGuiContext_ActiveIdHasBeenEditedThisFrame(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16201, v);
}
function get_ImGuiContext_ActiveIdClickOffset(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16204);
}
function get_ImGuiContext_ActiveIdWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16216);
}
function set_ImGuiContext_ActiveIdWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16216, v);
}
function get_ImGuiContext_ActiveIdSource(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16224);
}
function set_ImGuiContext_ActiveIdSource(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16224, v);
}
function get_ImGuiContext_ActiveIdMouseButton(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16228);
}
function set_ImGuiContext_ActiveIdMouseButton(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16228, v);
}
function get_ImGuiContext_ActiveIdPreviousFrame(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 16232);
}
function set_ImGuiContext_ActiveIdPreviousFrame(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 16232, v);
}
function get_ImGuiContext_ActiveIdPreviousFrameIsAlive(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16236);
}
function set_ImGuiContext_ActiveIdPreviousFrameIsAlive(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16236, v);
}
function get_ImGuiContext_ActiveIdPreviousFrameHasBeenEditedBefore(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16237);
}
function set_ImGuiContext_ActiveIdPreviousFrameHasBeenEditedBefore(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16237, v);
}
function get_ImGuiContext_ActiveIdPreviousFrameWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16240);
}
function set_ImGuiContext_ActiveIdPreviousFrameWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16240, v);
}
function get_ImGuiContext_LastActiveId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 16248);
}
function set_ImGuiContext_LastActiveId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 16248, v);
}
function get_ImGuiContext_LastActiveIdTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16252);
}
function set_ImGuiContext_LastActiveIdTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16252, v);
}
function get_ImGuiContext_KeysOwnerData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16256);
}
function set_ImGuiContext_KeysOwnerData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16256, v);
}
function get_ImGuiContext_KeysRoutingTable(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 17936);
}
function get_ImGuiContext_ActiveIdUsingNavDirMask(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18248);
}
function set_ImGuiContext_ActiveIdUsingNavDirMask(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18248, v);
}
function get_ImGuiContext_ActiveIdUsingAllKeyboardKeys(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18252);
}
function set_ImGuiContext_ActiveIdUsingAllKeyboardKeys(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18252, v);
}
function get_ImGuiContext_ActiveIdUsingNavInputMask(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18256);
}
function set_ImGuiContext_ActiveIdUsingNavInputMask(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18256, v);
}
function get_ImGuiContext_CurrentFocusScopeId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18260);
}
function set_ImGuiContext_CurrentFocusScopeId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18260, v);
}
function get_ImGuiContext_CurrentItemFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18264);
}
function set_ImGuiContext_CurrentItemFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18264, v);
}
function get_ImGuiContext_DebugLocateId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18268);
}
function set_ImGuiContext_DebugLocateId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18268, v);
}
function get_ImGuiContext_NextItemData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18272);
}
function get_ImGuiContext_LastItemData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18296);
}
function get_ImGuiContext_NextWindowData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18360);
}
function get_ImGuiContext_ColorStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18472);
}
function get_ImGuiContext_StyleVarStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18488);
}
function get_ImGuiContext_FontStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18504);
}
function get_ImGuiContext_FocusScopeStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18520);
}
function get_ImGuiContext_ItemFlagsStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18536);
}
function get_ImGuiContext_GroupStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18552);
}
function get_ImGuiContext_OpenPopupStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18568);
}
function get_ImGuiContext_BeginPopupStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18584);
}
function get_ImGuiContext_NavTreeNodeStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18600);
}
function get_ImGuiContext_BeginMenuCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18616);
}
function set_ImGuiContext_BeginMenuCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18616, v);
}
function get_ImGuiContext_Viewports(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18624);
}
function get_ImGuiContext_NavWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 18640);
}
function set_ImGuiContext_NavWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 18640, v);
}
function get_ImGuiContext_NavId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18648);
}
function set_ImGuiContext_NavId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18648, v);
}
function get_ImGuiContext_NavFocusScopeId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18652);
}
function set_ImGuiContext_NavFocusScopeId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18652, v);
}
function get_ImGuiContext_NavActivateId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18656);
}
function set_ImGuiContext_NavActivateId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18656, v);
}
function get_ImGuiContext_NavActivateDownId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18660);
}
function set_ImGuiContext_NavActivateDownId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18660, v);
}
function get_ImGuiContext_NavActivatePressedId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18664);
}
function set_ImGuiContext_NavActivatePressedId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18664, v);
}
function get_ImGuiContext_NavActivateFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18668);
}
function set_ImGuiContext_NavActivateFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18668, v);
}
function get_ImGuiContext_NavJustMovedToId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18672);
}
function set_ImGuiContext_NavJustMovedToId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18672, v);
}
function get_ImGuiContext_NavJustMovedToFocusScopeId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18676);
}
function set_ImGuiContext_NavJustMovedToFocusScopeId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18676, v);
}
function get_ImGuiContext_NavJustMovedToKeyMods(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18680);
}
function set_ImGuiContext_NavJustMovedToKeyMods(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18680, v);
}
function get_ImGuiContext_NavNextActivateId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 18684);
}
function set_ImGuiContext_NavNextActivateId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 18684, v);
}
function get_ImGuiContext_NavNextActivateFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18688);
}
function set_ImGuiContext_NavNextActivateFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18688, v);
}
function get_ImGuiContext_NavInputSource(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18692);
}
function set_ImGuiContext_NavInputSource(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18692, v);
}
function get_ImGuiContext_NavLayer(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18696);
}
function set_ImGuiContext_NavLayer(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18696, v);
}
function get_ImGuiContext_NavIdIsAlive(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18700);
}
function set_ImGuiContext_NavIdIsAlive(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18700, v);
}
function get_ImGuiContext_NavMousePosDirty(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18701);
}
function set_ImGuiContext_NavMousePosDirty(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18701, v);
}
function get_ImGuiContext_NavDisableHighlight(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18702);
}
function set_ImGuiContext_NavDisableHighlight(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18702, v);
}
function get_ImGuiContext_NavDisableMouseHover(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18703);
}
function set_ImGuiContext_NavDisableMouseHover(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18703, v);
}
function get_ImGuiContext_NavAnyRequest(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18704);
}
function set_ImGuiContext_NavAnyRequest(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18704, v);
}
function get_ImGuiContext_NavInitRequest(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18705);
}
function set_ImGuiContext_NavInitRequest(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18705, v);
}
function get_ImGuiContext_NavInitRequestFromMove(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18706);
}
function set_ImGuiContext_NavInitRequestFromMove(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18706, v);
}
function get_ImGuiContext_NavInitResult(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18712);
}
function get_ImGuiContext_NavMoveSubmitted(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18760);
}
function set_ImGuiContext_NavMoveSubmitted(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18760, v);
}
function get_ImGuiContext_NavMoveScoringItems(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18761);
}
function set_ImGuiContext_NavMoveScoringItems(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18761, v);
}
function get_ImGuiContext_NavMoveForwardToNextFrame(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 18762);
}
function set_ImGuiContext_NavMoveForwardToNextFrame(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 18762, v);
}
function get_ImGuiContext_NavMoveFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18764);
}
function set_ImGuiContext_NavMoveFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18764, v);
}
function get_ImGuiContext_NavMoveScrollFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18768);
}
function set_ImGuiContext_NavMoveScrollFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18768, v);
}
function get_ImGuiContext_NavMoveKeyMods(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18772);
}
function set_ImGuiContext_NavMoveKeyMods(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18772, v);
}
function get_ImGuiContext_NavMoveDir(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18776);
}
function set_ImGuiContext_NavMoveDir(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18776, v);
}
function get_ImGuiContext_NavMoveDirForDebug(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18780);
}
function set_ImGuiContext_NavMoveDirForDebug(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18780, v);
}
function get_ImGuiContext_NavMoveClipDir(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18784);
}
function set_ImGuiContext_NavMoveClipDir(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18784, v);
}
function get_ImGuiContext_NavScoringRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18788);
}
function get_ImGuiContext_NavScoringNoClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18804);
}
function get_ImGuiContext_NavScoringDebugCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18820);
}
function set_ImGuiContext_NavScoringDebugCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18820, v);
}
function get_ImGuiContext_NavTabbingDir(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18824);
}
function set_ImGuiContext_NavTabbingDir(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18824, v);
}
function get_ImGuiContext_NavTabbingCounter(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 18828);
}
function set_ImGuiContext_NavTabbingCounter(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 18828, v);
}
function get_ImGuiContext_NavMoveResultLocal(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18832);
}
function get_ImGuiContext_NavMoveResultLocalVisible(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18880);
}
function get_ImGuiContext_NavMoveResultOther(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18928);
}
function get_ImGuiContext_NavTabbingResultFirst(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 18976);
}
function get_ImGuiContext_ConfigNavWindowingKeyNext(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 19024);
}
function set_ImGuiContext_ConfigNavWindowingKeyNext(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 19024, v);
}
function get_ImGuiContext_ConfigNavWindowingKeyPrev(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 19028);
}
function set_ImGuiContext_ConfigNavWindowingKeyPrev(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 19028, v);
}
function get_ImGuiContext_NavWindowingTarget(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 19032);
}
function set_ImGuiContext_NavWindowingTarget(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 19032, v);
}
function get_ImGuiContext_NavWindowingTargetAnim(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 19040);
}
function set_ImGuiContext_NavWindowingTargetAnim(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 19040, v);
}
function get_ImGuiContext_NavWindowingListWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 19048);
}
function set_ImGuiContext_NavWindowingListWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 19048, v);
}
function get_ImGuiContext_NavWindowingTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 19056);
}
function set_ImGuiContext_NavWindowingTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 19056, v);
}
function get_ImGuiContext_NavWindowingHighlightAlpha(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 19060);
}
function set_ImGuiContext_NavWindowingHighlightAlpha(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 19060, v);
}
function get_ImGuiContext_NavWindowingToggleLayer(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 19064);
}
function set_ImGuiContext_NavWindowingToggleLayer(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 19064, v);
}
function get_ImGuiContext_NavWindowingAccumDeltaPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19068);
}
function get_ImGuiContext_NavWindowingAccumDeltaSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19076);
}
function get_ImGuiContext_DimBgRatio(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 19084);
}
function set_ImGuiContext_DimBgRatio(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 19084, v);
}
function get_ImGuiContext_DragDropActive(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 19088);
}
function set_ImGuiContext_DragDropActive(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 19088, v);
}
function get_ImGuiContext_DragDropWithinSource(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 19089);
}
function set_ImGuiContext_DragDropWithinSource(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 19089, v);
}
function get_ImGuiContext_DragDropWithinTarget(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 19090);
}
function set_ImGuiContext_DragDropWithinTarget(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 19090, v);
}
function get_ImGuiContext_DragDropSourceFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 19092);
}
function set_ImGuiContext_DragDropSourceFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 19092, v);
}
function get_ImGuiContext_DragDropSourceFrameCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 19096);
}
function set_ImGuiContext_DragDropSourceFrameCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 19096, v);
}
function get_ImGuiContext_DragDropMouseButton(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 19100);
}
function set_ImGuiContext_DragDropMouseButton(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 19100, v);
}
function get_ImGuiContext_DragDropPayload(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19104);
}
function get_ImGuiContext_DragDropTargetRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19168);
}
function get_ImGuiContext_DragDropTargetId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 19184);
}
function set_ImGuiContext_DragDropTargetId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 19184, v);
}
function get_ImGuiContext_DragDropAcceptFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 19188);
}
function set_ImGuiContext_DragDropAcceptFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 19188, v);
}
function get_ImGuiContext_DragDropAcceptIdCurrRectSurface(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 19192);
}
function set_ImGuiContext_DragDropAcceptIdCurrRectSurface(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 19192, v);
}
function get_ImGuiContext_DragDropAcceptIdCurr(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 19196);
}
function set_ImGuiContext_DragDropAcceptIdCurr(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 19196, v);
}
function get_ImGuiContext_DragDropAcceptIdPrev(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 19200);
}
function set_ImGuiContext_DragDropAcceptIdPrev(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 19200, v);
}
function get_ImGuiContext_DragDropAcceptFrameCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 19204);
}
function set_ImGuiContext_DragDropAcceptFrameCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 19204, v);
}
function get_ImGuiContext_DragDropHoldJustPressedId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 19208);
}
function set_ImGuiContext_DragDropHoldJustPressedId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 19208, v);
}
function get_ImGuiContext_DragDropPayloadBufHeap(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19216);
}
function get_ImGuiContext_DragDropPayloadBufLocal(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 19232);
}
function set_ImGuiContext_DragDropPayloadBufLocal(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 19232, v);
}
function get_ImGuiContext_ClipperTempDataStacked(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 19248);
}
function set_ImGuiContext_ClipperTempDataStacked(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 19248, v);
}
function get_ImGuiContext_ClipperTempData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19256);
}
function get_ImGuiContext_CurrentTable(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 19272);
}
function set_ImGuiContext_CurrentTable(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 19272, v);
}
function get_ImGuiContext_TablesTempDataStacked(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 19280);
}
function set_ImGuiContext_TablesTempDataStacked(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 19280, v);
}
function get_ImGuiContext_TablesTempData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19288);
}
function get_ImGuiContext_Tables(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19304);
}
function get_ImGuiContext_TablesLastTimeActive(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19344);
}
function get_ImGuiContext_DrawChannelsTempMergeBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19360);
}
function get_ImGuiContext_CurrentTabBar(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 19376);
}
function set_ImGuiContext_CurrentTabBar(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 19376, v);
}
function get_ImGuiContext_TabBars(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19384);
}
function get_ImGuiContext_CurrentTabBarStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19424);
}
function get_ImGuiContext_ShrinkWidthBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19440);
}
function get_ImGuiContext_HoverItemDelayId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 19456);
}
function set_ImGuiContext_HoverItemDelayId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 19456, v);
}
function get_ImGuiContext_HoverItemDelayIdPreviousFrame(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 19460);
}
function set_ImGuiContext_HoverItemDelayIdPreviousFrame(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 19460, v);
}
function get_ImGuiContext_HoverItemDelayTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 19464);
}
function set_ImGuiContext_HoverItemDelayTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 19464, v);
}
function get_ImGuiContext_HoverItemDelayClearTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 19468);
}
function set_ImGuiContext_HoverItemDelayClearTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 19468, v);
}
function get_ImGuiContext_HoverItemUnlockedStationaryId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 19472);
}
function set_ImGuiContext_HoverItemUnlockedStationaryId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 19472, v);
}
function get_ImGuiContext_HoverWindowUnlockedStationaryId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 19476);
}
function set_ImGuiContext_HoverWindowUnlockedStationaryId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 19476, v);
}
function get_ImGuiContext_MouseCursor(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 19480);
}
function set_ImGuiContext_MouseCursor(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 19480, v);
}
function get_ImGuiContext_MouseStationaryTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 19484);
}
function set_ImGuiContext_MouseStationaryTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 19484, v);
}
function get_ImGuiContext_MouseLastValidPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19488);
}
function get_ImGuiContext_InputTextState(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 19496);
}
function get_ImGuiContext_InputTextDeactivatedState(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23224);
}
function get_ImGuiContext_InputTextPasswordFont(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23248);
}
function get_ImGuiContext_TempInputId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 23368);
}
function set_ImGuiContext_TempInputId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 23368, v);
}
function get_ImGuiContext_ColorEditOptions(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 23372);
}
function set_ImGuiContext_ColorEditOptions(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 23372, v);
}
function get_ImGuiContext_ColorEditCurrentID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 23376);
}
function set_ImGuiContext_ColorEditCurrentID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 23376, v);
}
function get_ImGuiContext_ColorEditSavedID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 23380);
}
function set_ImGuiContext_ColorEditSavedID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 23380, v);
}
function get_ImGuiContext_ColorEditSavedHue(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 23384);
}
function set_ImGuiContext_ColorEditSavedHue(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 23384, v);
}
function get_ImGuiContext_ColorEditSavedSat(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 23388);
}
function set_ImGuiContext_ColorEditSavedSat(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 23388, v);
}
function get_ImGuiContext_ColorEditSavedColor(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 23392);
}
function set_ImGuiContext_ColorEditSavedColor(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 23392, v);
}
function get_ImGuiContext_ColorPickerRef(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23396);
}
function get_ImGuiContext_ComboPreviewData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23412);
}
function get_ImGuiContext_SliderGrabClickOffset(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 23460);
}
function set_ImGuiContext_SliderGrabClickOffset(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 23460, v);
}
function get_ImGuiContext_SliderCurrentAccum(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 23464);
}
function set_ImGuiContext_SliderCurrentAccum(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 23464, v);
}
function get_ImGuiContext_SliderCurrentAccumDirty(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 23468);
}
function set_ImGuiContext_SliderCurrentAccumDirty(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 23468, v);
}
function get_ImGuiContext_DragCurrentAccumDirty(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 23469);
}
function set_ImGuiContext_DragCurrentAccumDirty(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 23469, v);
}
function get_ImGuiContext_DragCurrentAccum(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 23472);
}
function set_ImGuiContext_DragCurrentAccum(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 23472, v);
}
function get_ImGuiContext_DragSpeedDefaultRatio(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 23476);
}
function set_ImGuiContext_DragSpeedDefaultRatio(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 23476, v);
}
function get_ImGuiContext_ScrollbarClickDeltaToGrabCenter(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 23480);
}
function set_ImGuiContext_ScrollbarClickDeltaToGrabCenter(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 23480, v);
}
function get_ImGuiContext_DisabledAlphaBackup(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 23484);
}
function set_ImGuiContext_DisabledAlphaBackup(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 23484, v);
}
function get_ImGuiContext_DisabledStackSize(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 23488);
}
function set_ImGuiContext_DisabledStackSize(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 23488, v);
}
function get_ImGuiContext_LockMarkEdited(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 23490);
}
function set_ImGuiContext_LockMarkEdited(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 23490, v);
}
function get_ImGuiContext_TooltipOverrideCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 23492);
}
function set_ImGuiContext_TooltipOverrideCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 23492, v);
}
function get_ImGuiContext_ClipboardHandlerData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23496);
}
function get_ImGuiContext_MenusIdSubmittedThisFrame(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23512);
}
function get_ImGuiContext_PlatformImeData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23528);
}
function get_ImGuiContext_PlatformImeDataPrev(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23544);
}
function get_ImGuiContext_SettingsLoaded(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 23560);
}
function set_ImGuiContext_SettingsLoaded(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 23560, v);
}
function get_ImGuiContext_SettingsDirtyTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 23564);
}
function set_ImGuiContext_SettingsDirtyTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 23564, v);
}
function get_ImGuiContext_SettingsIniData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23568);
}
function get_ImGuiContext_SettingsHandlers(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23584);
}
function get_ImGuiContext_SettingsWindows(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23600);
}
function get_ImGuiContext_SettingsTables(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23616);
}
function get_ImGuiContext_Hooks(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23632);
}
function get_ImGuiContext_HookIdNext(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 23648);
}
function set_ImGuiContext_HookIdNext(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 23648, v);
}
function get_ImGuiContext_LocalizationTable(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 23656);
}
function set_ImGuiContext_LocalizationTable(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 23656, v);
}
function get_ImGuiContext_LogEnabled(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 23720);
}
function set_ImGuiContext_LogEnabled(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 23720, v);
}
function get_ImGuiContext_LogType(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 23724);
}
function set_ImGuiContext_LogType(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 23724, v);
}
function get_ImGuiContext_LogFile(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 23728);
}
function set_ImGuiContext_LogFile(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 23728, v);
}
function get_ImGuiContext_LogBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23736);
}
function get_ImGuiContext_LogNextPrefix(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 23752);
}
function set_ImGuiContext_LogNextPrefix(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 23752, v);
}
function get_ImGuiContext_LogNextSuffix(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 23760);
}
function set_ImGuiContext_LogNextSuffix(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 23760, v);
}
function get_ImGuiContext_LogLinePosY(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 23768);
}
function set_ImGuiContext_LogLinePosY(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 23768, v);
}
function get_ImGuiContext_LogLineFirstItem(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 23772);
}
function set_ImGuiContext_LogLineFirstItem(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 23772, v);
}
function get_ImGuiContext_LogDepthRef(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 23776);
}
function set_ImGuiContext_LogDepthRef(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 23776, v);
}
function get_ImGuiContext_LogDepthToExpand(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 23780);
}
function set_ImGuiContext_LogDepthToExpand(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 23780, v);
}
function get_ImGuiContext_LogDepthToExpandDefault(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 23784);
}
function set_ImGuiContext_LogDepthToExpandDefault(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 23784, v);
}
function get_ImGuiContext_DebugLogFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 23788);
}
function set_ImGuiContext_DebugLogFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 23788, v);
}
function get_ImGuiContext_DebugLogBuf(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23792);
}
function get_ImGuiContext_DebugLogIndex(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23808);
}
function get_ImGuiContext_DebugLogClipperAutoDisableFrames(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 23832);
}
function set_ImGuiContext_DebugLogClipperAutoDisableFrames(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 23832, v);
}
function get_ImGuiContext_DebugLocateFrames(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 23833);
}
function set_ImGuiContext_DebugLocateFrames(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 23833, v);
}
function get_ImGuiContext_DebugBeginReturnValueCullDepth(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 23834);
}
function set_ImGuiContext_DebugBeginReturnValueCullDepth(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 23834, v);
}
function get_ImGuiContext_DebugItemPickerActive(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 23835);
}
function set_ImGuiContext_DebugItemPickerActive(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 23835, v);
}
function get_ImGuiContext_DebugItemPickerMouseButton(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 23836);
}
function set_ImGuiContext_DebugItemPickerMouseButton(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 23836, v);
}
function get_ImGuiContext_DebugItemPickerBreakId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 23840);
}
function set_ImGuiContext_DebugItemPickerBreakId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 23840, v);
}
function get_ImGuiContext_DebugMetricsConfig(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23844);
}
function get_ImGuiContext_DebugStackTool(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 23864);
}
function get_ImGuiContext_FramerateSecPerFrame(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 23904);
}
function set_ImGuiContext_FramerateSecPerFrame(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 23904, v);
}
function get_ImGuiContext_FramerateSecPerFrameIdx(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 24144);
}
function set_ImGuiContext_FramerateSecPerFrameIdx(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 24144, v);
}
function get_ImGuiContext_FramerateSecPerFrameCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 24148);
}
function set_ImGuiContext_FramerateSecPerFrameCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 24148, v);
}
function get_ImGuiContext_FramerateSecPerFrameAccum(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 24152);
}
function set_ImGuiContext_FramerateSecPerFrameAccum(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 24152, v);
}
function get_ImGuiContext_WantCaptureMouseNextFrame(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 24156);
}
function set_ImGuiContext_WantCaptureMouseNextFrame(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 24156, v);
}
function get_ImGuiContext_WantCaptureKeyboardNextFrame(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 24160);
}
function set_ImGuiContext_WantCaptureKeyboardNextFrame(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 24160, v);
}
function get_ImGuiContext_WantTextInputNextFrame(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 24164);
}
function set_ImGuiContext_WantTextInputNextFrame(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 24164, v);
}
function get_ImGuiContext_TempBuffer(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24168);
}
function get_ImGuiIO_ConfigFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiIO_ConfigFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiIO_BackendFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiIO_BackendFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiIO_DisplaySize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImGuiIO_DeltaTime(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16);
}
function set_ImGuiIO_DeltaTime(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16, v);
}
function get_ImGuiIO_IniSavingRate(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 20);
}
function set_ImGuiIO_IniSavingRate(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 20, v);
}
function get_ImGuiIO_IniFilename(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 24);
}
function set_ImGuiIO_IniFilename(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 24, v);
}
function get_ImGuiIO_LogFilename(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 32);
}
function set_ImGuiIO_LogFilename(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 32, v);
}
function get_ImGuiIO_UserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 40);
}
function set_ImGuiIO_UserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 40, v);
}
function get_ImGuiIO_Fonts(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 48);
}
function set_ImGuiIO_Fonts(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 48, v);
}
function get_ImGuiIO_FontGlobalScale(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 56);
}
function set_ImGuiIO_FontGlobalScale(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 56, v);
}
function get_ImGuiIO_FontAllowUserScaling(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 60);
}
function set_ImGuiIO_FontAllowUserScaling(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 60, v);
}
function get_ImGuiIO_FontDefault(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 64);
}
function set_ImGuiIO_FontDefault(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 64, v);
}
function get_ImGuiIO_DisplayFramebufferScale(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 72);
}
function get_ImGuiIO_MouseDrawCursor(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 80);
}
function set_ImGuiIO_MouseDrawCursor(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 80, v);
}
function get_ImGuiIO_ConfigMacOSXBehaviors(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 81);
}
function set_ImGuiIO_ConfigMacOSXBehaviors(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 81, v);
}
function get_ImGuiIO_ConfigInputTrickleEventQueue(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 82);
}
function set_ImGuiIO_ConfigInputTrickleEventQueue(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 82, v);
}
function get_ImGuiIO_ConfigInputTextCursorBlink(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 83);
}
function set_ImGuiIO_ConfigInputTextCursorBlink(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 83, v);
}
function get_ImGuiIO_ConfigInputTextEnterKeepActive(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 84);
}
function set_ImGuiIO_ConfigInputTextEnterKeepActive(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 84, v);
}
function get_ImGuiIO_ConfigDragClickToInputText(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 85);
}
function set_ImGuiIO_ConfigDragClickToInputText(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 85, v);
}
function get_ImGuiIO_ConfigWindowsResizeFromEdges(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 86);
}
function set_ImGuiIO_ConfigWindowsResizeFromEdges(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 86, v);
}
function get_ImGuiIO_ConfigWindowsMoveFromTitleBarOnly(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 87);
}
function set_ImGuiIO_ConfigWindowsMoveFromTitleBarOnly(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 87, v);
}
function get_ImGuiIO_ConfigMemoryCompactTimer(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 88);
}
function set_ImGuiIO_ConfigMemoryCompactTimer(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 88, v);
}
function get_ImGuiIO_MouseDoubleClickTime(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 92);
}
function set_ImGuiIO_MouseDoubleClickTime(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 92, v);
}
function get_ImGuiIO_MouseDoubleClickMaxDist(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 96);
}
function set_ImGuiIO_MouseDoubleClickMaxDist(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 96, v);
}
function get_ImGuiIO_MouseDragThreshold(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 100);
}
function set_ImGuiIO_MouseDragThreshold(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 100, v);
}
function get_ImGuiIO_KeyRepeatDelay(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 104);
}
function set_ImGuiIO_KeyRepeatDelay(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 104, v);
}
function get_ImGuiIO_KeyRepeatRate(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 108);
}
function set_ImGuiIO_KeyRepeatRate(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 108, v);
}
function get_ImGuiIO_ConfigDebugBeginReturnValueOnce(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 112);
}
function set_ImGuiIO_ConfigDebugBeginReturnValueOnce(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 112, v);
}
function get_ImGuiIO_ConfigDebugBeginReturnValueLoop(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 113);
}
function set_ImGuiIO_ConfigDebugBeginReturnValueLoop(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 113, v);
}
function get_ImGuiIO_ConfigDebugIgnoreFocusLoss(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 114);
}
function set_ImGuiIO_ConfigDebugIgnoreFocusLoss(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 114, v);
}
function get_ImGuiIO_ConfigDebugIniSettings(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 115);
}
function set_ImGuiIO_ConfigDebugIniSettings(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 115, v);
}
function get_ImGuiIO_BackendPlatformName(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 120);
}
function set_ImGuiIO_BackendPlatformName(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 120, v);
}
function get_ImGuiIO_BackendRendererName(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 128);
}
function set_ImGuiIO_BackendRendererName(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 128, v);
}
function get_ImGuiIO_BackendPlatformUserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 136);
}
function set_ImGuiIO_BackendPlatformUserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 136, v);
}
function get_ImGuiIO_BackendRendererUserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 144);
}
function set_ImGuiIO_BackendRendererUserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 144, v);
}
function get_ImGuiIO_BackendLanguageUserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 152);
}
function set_ImGuiIO_BackendLanguageUserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 152, v);
}
function get_ImGuiIO_GetClipboardTextFn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 160);
}
function set_ImGuiIO_GetClipboardTextFn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 160, v);
}
function get_ImGuiIO_SetClipboardTextFn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 168);
}
function set_ImGuiIO_SetClipboardTextFn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 168, v);
}
function get_ImGuiIO_ClipboardUserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 176);
}
function set_ImGuiIO_ClipboardUserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 176, v);
}
function get_ImGuiIO_SetPlatformImeDataFn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 184);
}
function set_ImGuiIO_SetPlatformImeDataFn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 184, v);
}
function get_ImGuiIO__UnusedPadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 192);
}
function set_ImGuiIO__UnusedPadding(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 192, v);
}
function get_ImGuiIO_PlatformLocaleDecimalPoint(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 200);
}
function set_ImGuiIO_PlatformLocaleDecimalPoint(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 200, v);
}
function get_ImGuiIO_WantCaptureMouse(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 202);
}
function set_ImGuiIO_WantCaptureMouse(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 202, v);
}
function get_ImGuiIO_WantCaptureKeyboard(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 203);
}
function set_ImGuiIO_WantCaptureKeyboard(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 203, v);
}
function get_ImGuiIO_WantTextInput(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 204);
}
function set_ImGuiIO_WantTextInput(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 204, v);
}
function get_ImGuiIO_WantSetMousePos(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 205);
}
function set_ImGuiIO_WantSetMousePos(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 205, v);
}
function get_ImGuiIO_WantSaveIniSettings(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 206);
}
function set_ImGuiIO_WantSaveIniSettings(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 206, v);
}
function get_ImGuiIO_NavActive(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 207);
}
function set_ImGuiIO_NavActive(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 207, v);
}
function get_ImGuiIO_NavVisible(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 208);
}
function set_ImGuiIO_NavVisible(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 208, v);
}
function get_ImGuiIO_Framerate(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 212);
}
function set_ImGuiIO_Framerate(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 212, v);
}
function get_ImGuiIO_MetricsRenderVertices(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 216);
}
function set_ImGuiIO_MetricsRenderVertices(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 216, v);
}
function get_ImGuiIO_MetricsRenderIndices(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 220);
}
function set_ImGuiIO_MetricsRenderIndices(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 220, v);
}
function get_ImGuiIO_MetricsRenderWindows(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 224);
}
function set_ImGuiIO_MetricsRenderWindows(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 224, v);
}
function get_ImGuiIO_MetricsActiveWindows(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 228);
}
function set_ImGuiIO_MetricsActiveWindows(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 228, v);
}
function get_ImGuiIO_MetricsActiveAllocations(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 232);
}
function set_ImGuiIO_MetricsActiveAllocations(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 232, v);
}
function get_ImGuiIO_MouseDelta(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 236);
}
function get_ImGuiIO_KeyMap(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 244);
}
function set_ImGuiIO_KeyMap(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 244, v);
}
function get_ImGuiIO_KeysDown(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 2852);
}
function set_ImGuiIO_KeysDown(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 2852, v);
}
function get_ImGuiIO_NavInputs(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 3504);
}
function set_ImGuiIO_NavInputs(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 3504, v);
}
function get_ImGuiIO_Ctx(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 3568);
}
function set_ImGuiIO_Ctx(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 3568, v);
}
function get_ImGuiIO_MousePos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 3576);
}
function get_ImGuiIO_MouseDown(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 3584);
}
function set_ImGuiIO_MouseDown(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 3584, v);
}
function get_ImGuiIO_MouseWheel(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 3592);
}
function set_ImGuiIO_MouseWheel(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 3592, v);
}
function get_ImGuiIO_MouseWheelH(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 3596);
}
function set_ImGuiIO_MouseWheelH(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 3596, v);
}
function get_ImGuiIO_MouseSource(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 3600);
}
function set_ImGuiIO_MouseSource(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 3600, v);
}
function get_ImGuiIO_KeyCtrl(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 3604);
}
function set_ImGuiIO_KeyCtrl(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 3604, v);
}
function get_ImGuiIO_KeyShift(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 3605);
}
function set_ImGuiIO_KeyShift(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 3605, v);
}
function get_ImGuiIO_KeyAlt(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 3606);
}
function set_ImGuiIO_KeyAlt(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 3606, v);
}
function get_ImGuiIO_KeySuper(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 3607);
}
function set_ImGuiIO_KeySuper(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 3607, v);
}
function get_ImGuiIO_KeyMods(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 3608);
}
function set_ImGuiIO_KeyMods(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 3608, v);
}
function get_ImGuiIO_KeysData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 3612);
}
function set_ImGuiIO_KeysData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 3612, v);
}
function get_ImGuiIO_WantCaptureMouseUnlessPopupClose(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 14044);
}
function set_ImGuiIO_WantCaptureMouseUnlessPopupClose(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 14044, v);
}
function get_ImGuiIO_MousePosPrev(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 14048);
}
function get_ImGuiIO_MouseClickedPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14056);
}
function set_ImGuiIO_MouseClickedPos(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14056, v);
}
function get_ImGuiIO_MouseClickedTime(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14096);
}
function set_ImGuiIO_MouseClickedTime(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14096, v);
}
function get_ImGuiIO_MouseClicked(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14136);
}
function set_ImGuiIO_MouseClicked(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14136, v);
}
function get_ImGuiIO_MouseDoubleClicked(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14141);
}
function set_ImGuiIO_MouseDoubleClicked(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14141, v);
}
function get_ImGuiIO_MouseClickedCount(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14146);
}
function set_ImGuiIO_MouseClickedCount(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14146, v);
}
function get_ImGuiIO_MouseClickedLastCount(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14156);
}
function set_ImGuiIO_MouseClickedLastCount(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14156, v);
}
function get_ImGuiIO_MouseReleased(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14166);
}
function set_ImGuiIO_MouseReleased(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14166, v);
}
function get_ImGuiIO_MouseDownOwned(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14171);
}
function set_ImGuiIO_MouseDownOwned(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14171, v);
}
function get_ImGuiIO_MouseDownOwnedUnlessPopupClose(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14176);
}
function set_ImGuiIO_MouseDownOwnedUnlessPopupClose(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14176, v);
}
function get_ImGuiIO_MouseWheelRequestAxisSwap(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 14181);
}
function set_ImGuiIO_MouseWheelRequestAxisSwap(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 14181, v);
}
function get_ImGuiIO_MouseDownDuration(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14184);
}
function set_ImGuiIO_MouseDownDuration(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14184, v);
}
function get_ImGuiIO_MouseDownDurationPrev(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14204);
}
function set_ImGuiIO_MouseDownDurationPrev(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14204, v);
}
function get_ImGuiIO_MouseDragMaxDistanceSqr(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 14224);
}
function set_ImGuiIO_MouseDragMaxDistanceSqr(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 14224, v);
}
function get_ImGuiIO_PenPressure(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 14244);
}
function set_ImGuiIO_PenPressure(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 14244, v);
}
function get_ImGuiIO_AppFocusLost(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 14248);
}
function set_ImGuiIO_AppFocusLost(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 14248, v);
}
function get_ImGuiIO_AppAcceptingEvents(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 14249);
}
function set_ImGuiIO_AppAcceptingEvents(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 14249, v);
}
function get_ImGuiIO_BackendUsingLegacyKeyArrays(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 14250);
}
function set_ImGuiIO_BackendUsingLegacyKeyArrays(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 14250, v);
}
function get_ImGuiIO_BackendUsingLegacyNavInputArray(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 14251);
}
function set_ImGuiIO_BackendUsingLegacyNavInputArray(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 14251, v);
}
function get_ImGuiIO_InputQueueSurrogate(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 14252);
}
function set_ImGuiIO_InputQueueSurrogate(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 14252, v);
}
function get_ImGuiIO_InputQueueCharacters(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 14256);
}
function get_ImGuiInputTextCallbackData_Ctx(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiInputTextCallbackData_Ctx(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiInputTextCallbackData_EventFlag(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiInputTextCallbackData_EventFlag(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiInputTextCallbackData_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 12);
}
function set_ImGuiInputTextCallbackData_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 12, v);
}
function get_ImGuiInputTextCallbackData_UserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16);
}
function set_ImGuiInputTextCallbackData_UserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16, v);
}
function get_ImGuiInputTextCallbackData_EventChar(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 24);
}
function set_ImGuiInputTextCallbackData_EventChar(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 24, v);
}
function get_ImGuiInputTextCallbackData_EventKey(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 28);
}
function set_ImGuiInputTextCallbackData_EventKey(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 28, v);
}
function get_ImGuiInputTextCallbackData_Buf(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 32);
}
function set_ImGuiInputTextCallbackData_Buf(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 32, v);
}
function get_ImGuiInputTextCallbackData_BufTextLen(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 40);
}
function set_ImGuiInputTextCallbackData_BufTextLen(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 40, v);
}
function get_ImGuiInputTextCallbackData_BufSize(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 44);
}
function set_ImGuiInputTextCallbackData_BufSize(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 44, v);
}
function get_ImGuiInputTextCallbackData_BufDirty(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 48);
}
function set_ImGuiInputTextCallbackData_BufDirty(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 48, v);
}
function get_ImGuiInputTextCallbackData_CursorPos(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 52);
}
function set_ImGuiInputTextCallbackData_CursorPos(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 52, v);
}
function get_ImGuiInputTextCallbackData_SelectionStart(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 56);
}
function set_ImGuiInputTextCallbackData_SelectionStart(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 56, v);
}
function get_ImGuiInputTextCallbackData_SelectionEnd(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 60);
}
function set_ImGuiInputTextCallbackData_SelectionEnd(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 60, v);
}
function get_ImGuiKeyData_Down(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 0);
}
function set_ImGuiKeyData_Down(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 0, v);
}
function get_ImGuiKeyData_DownDuration(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImGuiKeyData_DownDuration(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImGuiKeyData_DownDurationPrev(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_ImGuiKeyData_DownDurationPrev(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_ImGuiKeyData_AnalogValue(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 12);
}
function set_ImGuiKeyData_AnalogValue(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 12, v);
}
function get_ImGuiListClipper_Ctx(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiListClipper_Ctx(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiListClipper_DisplayStart(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiListClipper_DisplayStart(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiListClipper_DisplayEnd(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 12);
}
function set_ImGuiListClipper_DisplayEnd(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 12, v);
}
function get_ImGuiListClipper_ItemsCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_ImGuiListClipper_ItemsCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_ImGuiListClipper_ItemsHeight(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 20);
}
function set_ImGuiListClipper_ItemsHeight(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 20, v);
}
function get_ImGuiListClipper_StartPosY(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 24);
}
function set_ImGuiListClipper_StartPosY(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 24, v);
}
function get_ImGuiListClipper_TempData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 32);
}
function set_ImGuiListClipper_TempData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 32, v);
}
function get_ImGuiOnceUponAFrame_RefFrame(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiOnceUponAFrame_RefFrame(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiPayload_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiPayload_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiPayload_DataSize(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiPayload_DataSize(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiPayload_SourceId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 12);
}
function set_ImGuiPayload_SourceId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 12, v);
}
function get_ImGuiPayload_SourceParentId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 16);
}
function set_ImGuiPayload_SourceParentId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 16, v);
}
function get_ImGuiPayload_DataFrameCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 20);
}
function set_ImGuiPayload_DataFrameCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 20, v);
}
function get_ImGuiPayload_DataType(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 24);
}
function set_ImGuiPayload_DataType(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 24, v);
}
function get_ImGuiPayload_Preview(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 57);
}
function set_ImGuiPayload_Preview(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 57, v);
}
function get_ImGuiPayload_Delivery(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 58);
}
function set_ImGuiPayload_Delivery(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 58, v);
}
function get_ImGuiPlatformImeData_WantVisible(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 0);
}
function set_ImGuiPlatformImeData_WantVisible(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 0, v);
}
function get_ImGuiPlatformImeData_InputPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 4);
}
function get_ImGuiPlatformImeData_InputLineHeight(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 12);
}
function set_ImGuiPlatformImeData_InputLineHeight(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 12, v);
}
function get_ImGuiSizeCallbackData_UserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiSizeCallbackData_UserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiSizeCallbackData_Pos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImGuiSizeCallbackData_CurrentSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImGuiSizeCallbackData_DesiredSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24);
}
function get_ImGuiStorage_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImGuiStyle_Alpha(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 0);
}
function set_ImGuiStyle_Alpha(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 0, v);
}
function get_ImGuiStyle_DisabledAlpha(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImGuiStyle_DisabledAlpha(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImGuiStyle_WindowPadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImGuiStyle_WindowRounding(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16);
}
function set_ImGuiStyle_WindowRounding(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16, v);
}
function get_ImGuiStyle_WindowBorderSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 20);
}
function set_ImGuiStyle_WindowBorderSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 20, v);
}
function get_ImGuiStyle_WindowMinSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24);
}
function get_ImGuiStyle_WindowTitleAlign(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 32);
}
function get_ImGuiStyle_WindowMenuButtonPosition(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 40);
}
function set_ImGuiStyle_WindowMenuButtonPosition(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 40, v);
}
function get_ImGuiStyle_ChildRounding(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 44);
}
function set_ImGuiStyle_ChildRounding(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 44, v);
}
function get_ImGuiStyle_ChildBorderSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 48);
}
function set_ImGuiStyle_ChildBorderSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 48, v);
}
function get_ImGuiStyle_PopupRounding(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 52);
}
function set_ImGuiStyle_PopupRounding(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 52, v);
}
function get_ImGuiStyle_PopupBorderSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 56);
}
function set_ImGuiStyle_PopupBorderSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 56, v);
}
function get_ImGuiStyle_FramePadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 60);
}
function get_ImGuiStyle_FrameRounding(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 68);
}
function set_ImGuiStyle_FrameRounding(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 68, v);
}
function get_ImGuiStyle_FrameBorderSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 72);
}
function set_ImGuiStyle_FrameBorderSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 72, v);
}
function get_ImGuiStyle_ItemSpacing(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 76);
}
function get_ImGuiStyle_ItemInnerSpacing(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 84);
}
function get_ImGuiStyle_CellPadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 92);
}
function get_ImGuiStyle_TouchExtraPadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 100);
}
function get_ImGuiStyle_IndentSpacing(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 108);
}
function set_ImGuiStyle_IndentSpacing(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 108, v);
}
function get_ImGuiStyle_ColumnsMinSpacing(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 112);
}
function set_ImGuiStyle_ColumnsMinSpacing(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 112, v);
}
function get_ImGuiStyle_ScrollbarSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 116);
}
function set_ImGuiStyle_ScrollbarSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 116, v);
}
function get_ImGuiStyle_ScrollbarRounding(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 120);
}
function set_ImGuiStyle_ScrollbarRounding(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 120, v);
}
function get_ImGuiStyle_GrabMinSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 124);
}
function set_ImGuiStyle_GrabMinSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 124, v);
}
function get_ImGuiStyle_GrabRounding(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 128);
}
function set_ImGuiStyle_GrabRounding(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 128, v);
}
function get_ImGuiStyle_LogSliderDeadzone(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 132);
}
function set_ImGuiStyle_LogSliderDeadzone(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 132, v);
}
function get_ImGuiStyle_TabRounding(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 136);
}
function set_ImGuiStyle_TabRounding(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 136, v);
}
function get_ImGuiStyle_TabBorderSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 140);
}
function set_ImGuiStyle_TabBorderSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 140, v);
}
function get_ImGuiStyle_TabMinWidthForCloseButton(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 144);
}
function set_ImGuiStyle_TabMinWidthForCloseButton(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 144, v);
}
function get_ImGuiStyle_ColorButtonPosition(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 148);
}
function set_ImGuiStyle_ColorButtonPosition(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 148, v);
}
function get_ImGuiStyle_ButtonTextAlign(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 152);
}
function get_ImGuiStyle_SelectableTextAlign(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 160);
}
function get_ImGuiStyle_SeparatorTextBorderSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 168);
}
function set_ImGuiStyle_SeparatorTextBorderSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 168, v);
}
function get_ImGuiStyle_SeparatorTextAlign(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 172);
}
function get_ImGuiStyle_SeparatorTextPadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 180);
}
function get_ImGuiStyle_DisplayWindowPadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 188);
}
function get_ImGuiStyle_DisplaySafeAreaPadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 196);
}
function get_ImGuiStyle_MouseCursorScale(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 204);
}
function set_ImGuiStyle_MouseCursorScale(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 204, v);
}
function get_ImGuiStyle_AntiAliasedLines(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 208);
}
function set_ImGuiStyle_AntiAliasedLines(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 208, v);
}
function get_ImGuiStyle_AntiAliasedLinesUseTex(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 209);
}
function set_ImGuiStyle_AntiAliasedLinesUseTex(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 209, v);
}
function get_ImGuiStyle_AntiAliasedFill(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 210);
}
function set_ImGuiStyle_AntiAliasedFill(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 210, v);
}
function get_ImGuiStyle_CurveTessellationTol(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 212);
}
function set_ImGuiStyle_CurveTessellationTol(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 212, v);
}
function get_ImGuiStyle_CircleTessellationMaxError(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 216);
}
function set_ImGuiStyle_CircleTessellationMaxError(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 216, v);
}
function get_ImGuiStyle_Colors(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 220);
}
function set_ImGuiStyle_Colors(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 220, v);
}
function get_ImGuiStyle_HoverStationaryDelay(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 1068);
}
function set_ImGuiStyle_HoverStationaryDelay(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 1068, v);
}
function get_ImGuiStyle_HoverDelayShort(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 1072);
}
function set_ImGuiStyle_HoverDelayShort(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 1072, v);
}
function get_ImGuiStyle_HoverDelayNormal(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 1076);
}
function set_ImGuiStyle_HoverDelayNormal(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 1076, v);
}
function get_ImGuiStyle_HoverFlagsForTooltipMouse(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 1080);
}
function set_ImGuiStyle_HoverFlagsForTooltipMouse(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 1080, v);
}
function get_ImGuiStyle_HoverFlagsForTooltipNav(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 1084);
}
function set_ImGuiStyle_HoverFlagsForTooltipNav(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 1084, v);
}
function get_ImGuiTableSortSpecs_Specs(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiTableSortSpecs_Specs(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiTableSortSpecs_SpecsCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiTableSortSpecs_SpecsCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiTableSortSpecs_SpecsDirty(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 12);
}
function set_ImGuiTableSortSpecs_SpecsDirty(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 12, v);
}
function get_ImGuiTableColumnSortSpecs_ColumnUserID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiTableColumnSortSpecs_ColumnUserID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiTableColumnSortSpecs_ColumnIndex(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 4);
}
function set_ImGuiTableColumnSortSpecs_ColumnIndex(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 4, v);
}
function get_ImGuiTableColumnSortSpecs_SortOrder(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 6);
}
function set_ImGuiTableColumnSortSpecs_SortOrder(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 6, v);
}
function get_ImGuiTableColumnSortSpecs_SortDirection(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiTableColumnSortSpecs_SortDirection(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiTextBuffer_Buf(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImGuiTextFilter_InputBuf(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiTextFilter_InputBuf(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiTextFilter_Filters(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 256);
}
function get_ImGuiTextFilter_CountGrep(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 272);
}
function set_ImGuiTextFilter_CountGrep(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 272, v);
}
function get_ImGuiViewport_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiViewport_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiViewport_Pos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 4);
}
function get_ImGuiViewport_Size(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 12);
}
function get_ImGuiViewport_WorkPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 20);
}
function get_ImGuiViewport_WorkSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 28);
}
function get_ImGuiViewport_PlatformHandleRaw(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 40);
}
function set_ImGuiViewport_PlatformHandleRaw(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 40, v);
}
function get_ImBitVector_Storage(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImRect_Min(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImRect_Max(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImDrawDataBuilder_Layers(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImDrawDataBuilder_Layers(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImDrawDataBuilder_LayerData1(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImGuiColorMod_Col(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiColorMod_Col(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiColorMod_BackupValue(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 4);
}
function get_ImGuiContextHook_HookId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiContextHook_HookId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiContextHook_Type(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiContextHook_Type(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiContextHook_Owner(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 8);
}
function set_ImGuiContextHook_Owner(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 8, v);
}
function get_ImGuiContextHook_Callback(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16);
}
function set_ImGuiContextHook_Callback(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16, v);
}
function get_ImGuiContextHook_UserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 24);
}
function set_ImGuiContextHook_UserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 24, v);
}
function get_ImGuiDataVarInfo_Type(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiDataVarInfo_Type(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiDataVarInfo_Count(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 4);
}
function set_ImGuiDataVarInfo_Count(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 4, v);
}
function get_ImGuiDataVarInfo_Offset(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 8);
}
function set_ImGuiDataVarInfo_Offset(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 8, v);
}
function get_ImGuiDataTypeInfo_Size(s: c_ptr): c_ulong {
  "inline";
  return _sh_ptr_read_c_ulong(s, 0);
}
function set_ImGuiDataTypeInfo_Size(s: c_ptr, v: c_ulong): void {
  "inline";
  _sh_ptr_write_c_ulong(s, 0, v);
}
function get_ImGuiDataTypeInfo_Name(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImGuiDataTypeInfo_Name(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiDataTypeInfo_PrintFmt(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16);
}
function set_ImGuiDataTypeInfo_PrintFmt(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16, v);
}
function get_ImGuiDataTypeInfo_ScanFmt(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 24);
}
function set_ImGuiDataTypeInfo_ScanFmt(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 24, v);
}
function get_ImGuiGroupData_WindowID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiGroupData_WindowID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiGroupData_BackupCursorPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 4);
}
function get_ImGuiGroupData_BackupCursorMaxPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 12);
}
function get_ImGuiGroupData_BackupIndent(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 20);
}
function get_ImGuiGroupData_BackupGroupOffset(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24);
}
function get_ImGuiGroupData_BackupCurrLineSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 28);
}
function get_ImGuiGroupData_BackupCurrLineTextBaseOffset(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 36);
}
function set_ImGuiGroupData_BackupCurrLineTextBaseOffset(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 36, v);
}
function get_ImGuiGroupData_BackupActiveIdIsAlive(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 40);
}
function set_ImGuiGroupData_BackupActiveIdIsAlive(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 40, v);
}
function get_ImGuiGroupData_BackupActiveIdPreviousFrameIsAlive(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 44);
}
function set_ImGuiGroupData_BackupActiveIdPreviousFrameIsAlive(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 44, v);
}
function get_ImGuiGroupData_BackupHoveredIdIsAlive(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 45);
}
function set_ImGuiGroupData_BackupHoveredIdIsAlive(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 45, v);
}
function get_ImGuiGroupData_EmitItem(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 46);
}
function set_ImGuiGroupData_EmitItem(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 46, v);
}
function get_ImGuiInputTextState_Ctx(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiInputTextState_Ctx(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiInputTextState_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 8);
}
function set_ImGuiInputTextState_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 8, v);
}
function get_ImGuiInputTextState_CurLenW(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 12);
}
function set_ImGuiInputTextState_CurLenW(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 12, v);
}
function get_ImGuiInputTextState_CurLenA(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_ImGuiInputTextState_CurLenA(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_ImGuiInputTextState_TextW(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24);
}
function get_ImGuiInputTextState_TextA(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 40);
}
function get_ImGuiInputTextState_InitialTextA(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 56);
}
function get_ImGuiInputTextState_TextAIsValid(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 72);
}
function set_ImGuiInputTextState_TextAIsValid(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 72, v);
}
function get_ImGuiInputTextState_BufCapacityA(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 76);
}
function set_ImGuiInputTextState_BufCapacityA(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 76, v);
}
function get_ImGuiInputTextState_ScrollX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 80);
}
function set_ImGuiInputTextState_ScrollX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 80, v);
}
function get_ImGuiInputTextState_Stb(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 84);
}
function get_ImGuiInputTextState_CursorAnim(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 3712);
}
function set_ImGuiInputTextState_CursorAnim(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 3712, v);
}
function get_ImGuiInputTextState_CursorFollow(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 3716);
}
function set_ImGuiInputTextState_CursorFollow(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 3716, v);
}
function get_ImGuiInputTextState_SelectedAllMouseLock(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 3717);
}
function set_ImGuiInputTextState_SelectedAllMouseLock(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 3717, v);
}
function get_ImGuiInputTextState_Edited(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 3718);
}
function set_ImGuiInputTextState_Edited(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 3718, v);
}
function get_ImGuiInputTextState_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 3720);
}
function set_ImGuiInputTextState_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 3720, v);
}
function get_ImGuiLastItemData_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiLastItemData_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiLastItemData_InFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiLastItemData_InFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiLastItemData_StatusFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiLastItemData_StatusFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiLastItemData_Rect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 12);
}
function get_ImGuiLastItemData_NavRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 28);
}
function get_ImGuiLastItemData_DisplayRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 44);
}
function get_ImGuiLocEntry_Key(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiLocEntry_Key(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiLocEntry_Text(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImGuiLocEntry_Text(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiMenuColumns_TotalWidth(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiMenuColumns_TotalWidth(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiMenuColumns_NextTotalWidth(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 4);
}
function set_ImGuiMenuColumns_NextTotalWidth(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 4, v);
}
function get_ImGuiMenuColumns_Spacing(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 8);
}
function set_ImGuiMenuColumns_Spacing(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 8, v);
}
function get_ImGuiMenuColumns_OffsetIcon(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 10);
}
function set_ImGuiMenuColumns_OffsetIcon(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 10, v);
}
function get_ImGuiMenuColumns_OffsetLabel(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 12);
}
function set_ImGuiMenuColumns_OffsetLabel(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 12, v);
}
function get_ImGuiMenuColumns_OffsetShortcut(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 14);
}
function set_ImGuiMenuColumns_OffsetShortcut(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 14, v);
}
function get_ImGuiMenuColumns_OffsetMark(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 16);
}
function set_ImGuiMenuColumns_OffsetMark(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 16, v);
}
function get_ImGuiMenuColumns_Widths(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 18);
}
function set_ImGuiMenuColumns_Widths(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 18, v);
}
function get_ImGuiNavItemData_Window(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiNavItemData_Window(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiNavItemData_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 8);
}
function set_ImGuiNavItemData_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 8, v);
}
function get_ImGuiNavItemData_FocusScopeId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 12);
}
function set_ImGuiNavItemData_FocusScopeId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 12, v);
}
function get_ImGuiNavItemData_RectRel(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImGuiNavItemData_InFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 32);
}
function set_ImGuiNavItemData_InFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 32, v);
}
function get_ImGuiNavItemData_DistBox(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 36);
}
function set_ImGuiNavItemData_DistBox(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 36, v);
}
function get_ImGuiNavItemData_DistCenter(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 40);
}
function set_ImGuiNavItemData_DistCenter(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 40, v);
}
function get_ImGuiNavItemData_DistAxial(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 44);
}
function set_ImGuiNavItemData_DistAxial(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 44, v);
}
function get_ImGuiNavTreeNodeData_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiNavTreeNodeData_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiNavTreeNodeData_InFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiNavTreeNodeData_InFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiNavTreeNodeData_NavRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImGuiMetricsConfig_ShowDebugLog(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 0);
}
function set_ImGuiMetricsConfig_ShowDebugLog(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 0, v);
}
function get_ImGuiMetricsConfig_ShowStackTool(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 1);
}
function set_ImGuiMetricsConfig_ShowStackTool(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 1, v);
}
function get_ImGuiMetricsConfig_ShowWindowsRects(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 2);
}
function set_ImGuiMetricsConfig_ShowWindowsRects(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 2, v);
}
function get_ImGuiMetricsConfig_ShowWindowsBeginOrder(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 3);
}
function set_ImGuiMetricsConfig_ShowWindowsBeginOrder(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 3, v);
}
function get_ImGuiMetricsConfig_ShowTablesRects(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 4);
}
function set_ImGuiMetricsConfig_ShowTablesRects(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 4, v);
}
function get_ImGuiMetricsConfig_ShowDrawCmdMesh(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 5);
}
function set_ImGuiMetricsConfig_ShowDrawCmdMesh(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 5, v);
}
function get_ImGuiMetricsConfig_ShowDrawCmdBoundingBoxes(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 6);
}
function set_ImGuiMetricsConfig_ShowDrawCmdBoundingBoxes(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 6, v);
}
function get_ImGuiMetricsConfig_ShowAtlasTintedWithTextColor(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 7);
}
function set_ImGuiMetricsConfig_ShowAtlasTintedWithTextColor(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 7, v);
}
function get_ImGuiMetricsConfig_ShowWindowsRectsType(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiMetricsConfig_ShowWindowsRectsType(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiMetricsConfig_ShowTablesRectsType(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 12);
}
function set_ImGuiMetricsConfig_ShowTablesRectsType(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 12, v);
}
function get_ImGuiNextWindowData_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiNextWindowData_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiNextWindowData_PosCond(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiNextWindowData_PosCond(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiNextWindowData_SizeCond(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiNextWindowData_SizeCond(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiNextWindowData_CollapsedCond(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 12);
}
function set_ImGuiNextWindowData_CollapsedCond(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 12, v);
}
function get_ImGuiNextWindowData_PosVal(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImGuiNextWindowData_PosPivotVal(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24);
}
function get_ImGuiNextWindowData_SizeVal(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 32);
}
function get_ImGuiNextWindowData_ContentSizeVal(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 40);
}
function get_ImGuiNextWindowData_ScrollVal(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 48);
}
function get_ImGuiNextWindowData_CollapsedVal(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 56);
}
function set_ImGuiNextWindowData_CollapsedVal(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 56, v);
}
function get_ImGuiNextWindowData_SizeConstraintRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 60);
}
function get_ImGuiNextWindowData_SizeCallback(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 80);
}
function set_ImGuiNextWindowData_SizeCallback(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 80, v);
}
function get_ImGuiNextWindowData_SizeCallbackUserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 88);
}
function set_ImGuiNextWindowData_SizeCallbackUserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 88, v);
}
function get_ImGuiNextWindowData_BgAlphaVal(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 96);
}
function set_ImGuiNextWindowData_BgAlphaVal(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 96, v);
}
function get_ImGuiNextWindowData_MenuBarOffsetMinVal(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 100);
}
function get_ImGuiNextItemData_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiNextItemData_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiNextItemData_ItemFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiNextItemData_ItemFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiNextItemData_Width(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_ImGuiNextItemData_Width(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_ImGuiNextItemData_FocusScopeId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 12);
}
function set_ImGuiNextItemData_FocusScopeId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 12, v);
}
function get_ImGuiNextItemData_OpenCond(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_ImGuiNextItemData_OpenCond(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_ImGuiNextItemData_OpenVal(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 20);
}
function set_ImGuiNextItemData_OpenVal(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 20, v);
}
function get_ImGuiOldColumnData_OffsetNorm(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 0);
}
function set_ImGuiOldColumnData_OffsetNorm(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 0, v);
}
function get_ImGuiOldColumnData_OffsetNormBeforeResize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImGuiOldColumnData_OffsetNormBeforeResize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImGuiOldColumnData_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiOldColumnData_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiOldColumnData_ClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 12);
}
function get_ImGuiOldColumns_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiOldColumns_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiOldColumns_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiOldColumns_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiOldColumns_IsFirstFrame(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 8);
}
function set_ImGuiOldColumns_IsFirstFrame(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 8, v);
}
function get_ImGuiOldColumns_IsBeingResized(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 9);
}
function set_ImGuiOldColumns_IsBeingResized(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 9, v);
}
function get_ImGuiOldColumns_Current(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 12);
}
function set_ImGuiOldColumns_Current(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 12, v);
}
function get_ImGuiOldColumns_Count(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_ImGuiOldColumns_Count(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_ImGuiOldColumns_OffMinX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 20);
}
function set_ImGuiOldColumns_OffMinX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 20, v);
}
function get_ImGuiOldColumns_OffMaxX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 24);
}
function set_ImGuiOldColumns_OffMaxX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 24, v);
}
function get_ImGuiOldColumns_LineMinY(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 28);
}
function set_ImGuiOldColumns_LineMinY(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 28, v);
}
function get_ImGuiOldColumns_LineMaxY(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 32);
}
function set_ImGuiOldColumns_LineMaxY(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 32, v);
}
function get_ImGuiOldColumns_HostCursorPosY(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 36);
}
function set_ImGuiOldColumns_HostCursorPosY(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 36, v);
}
function get_ImGuiOldColumns_HostCursorMaxPosX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 40);
}
function set_ImGuiOldColumns_HostCursorMaxPosX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 40, v);
}
function get_ImGuiOldColumns_HostInitialClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 44);
}
function get_ImGuiOldColumns_HostBackupClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 60);
}
function get_ImGuiOldColumns_HostBackupParentWorkRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 76);
}
function get_ImGuiOldColumns_Columns(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 96);
}
function get_ImGuiOldColumns_Splitter(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 112);
}
function get_ImGuiPopupData_PopupId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiPopupData_PopupId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiPopupData_Window(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImGuiPopupData_Window(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiPopupData_BackupNavWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16);
}
function set_ImGuiPopupData_BackupNavWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16, v);
}
function get_ImGuiPopupData_ParentNavLayer(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 24);
}
function set_ImGuiPopupData_ParentNavLayer(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 24, v);
}
function get_ImGuiPopupData_OpenFrameCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 28);
}
function set_ImGuiPopupData_OpenFrameCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 28, v);
}
function get_ImGuiPopupData_OpenParentId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 32);
}
function set_ImGuiPopupData_OpenParentId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 32, v);
}
function get_ImGuiPopupData_OpenPopupPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 36);
}
function get_ImGuiPopupData_OpenMousePos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 44);
}
function get_ImGuiSettingsHandler_TypeName(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiSettingsHandler_TypeName(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiSettingsHandler_TypeHash(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 8);
}
function set_ImGuiSettingsHandler_TypeHash(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 8, v);
}
function get_ImGuiSettingsHandler_ClearAllFn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16);
}
function set_ImGuiSettingsHandler_ClearAllFn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16, v);
}
function get_ImGuiSettingsHandler_ReadInitFn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 24);
}
function set_ImGuiSettingsHandler_ReadInitFn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 24, v);
}
function get_ImGuiSettingsHandler_ReadOpenFn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 32);
}
function set_ImGuiSettingsHandler_ReadOpenFn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 32, v);
}
function get_ImGuiSettingsHandler_ReadLineFn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 40);
}
function set_ImGuiSettingsHandler_ReadLineFn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 40, v);
}
function get_ImGuiSettingsHandler_ApplyAllFn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 48);
}
function set_ImGuiSettingsHandler_ApplyAllFn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 48, v);
}
function get_ImGuiSettingsHandler_WriteAllFn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 56);
}
function set_ImGuiSettingsHandler_WriteAllFn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 56, v);
}
function get_ImGuiSettingsHandler_UserData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 64);
}
function set_ImGuiSettingsHandler_UserData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 64, v);
}
function get_ImGuiStackSizes_SizeOfIDStack(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 0);
}
function set_ImGuiStackSizes_SizeOfIDStack(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 0, v);
}
function get_ImGuiStackSizes_SizeOfColorStack(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 2);
}
function set_ImGuiStackSizes_SizeOfColorStack(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 2, v);
}
function get_ImGuiStackSizes_SizeOfStyleVarStack(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 4);
}
function set_ImGuiStackSizes_SizeOfStyleVarStack(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 4, v);
}
function get_ImGuiStackSizes_SizeOfFontStack(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 6);
}
function set_ImGuiStackSizes_SizeOfFontStack(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 6, v);
}
function get_ImGuiStackSizes_SizeOfFocusScopeStack(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 8);
}
function set_ImGuiStackSizes_SizeOfFocusScopeStack(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 8, v);
}
function get_ImGuiStackSizes_SizeOfGroupStack(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 10);
}
function set_ImGuiStackSizes_SizeOfGroupStack(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 10, v);
}
function get_ImGuiStackSizes_SizeOfItemFlagsStack(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 12);
}
function set_ImGuiStackSizes_SizeOfItemFlagsStack(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 12, v);
}
function get_ImGuiStackSizes_SizeOfBeginPopupStack(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 14);
}
function set_ImGuiStackSizes_SizeOfBeginPopupStack(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 14, v);
}
function get_ImGuiStackSizes_SizeOfDisabledStack(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 16);
}
function set_ImGuiStackSizes_SizeOfDisabledStack(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 16, v);
}
function get_ImGuiStyleMod_VarIdx(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiStyleMod_VarIdx(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiStyleMod_(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 4);
}
function get_ImGuiTabBar_Tabs(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImGuiTabBar_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_ImGuiTabBar_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_ImGuiTabBar_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 20);
}
function set_ImGuiTabBar_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 20, v);
}
function get_ImGuiTabBar_SelectedTabId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 24);
}
function set_ImGuiTabBar_SelectedTabId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 24, v);
}
function get_ImGuiTabBar_NextSelectedTabId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 28);
}
function set_ImGuiTabBar_NextSelectedTabId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 28, v);
}
function get_ImGuiTabBar_VisibleTabId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 32);
}
function set_ImGuiTabBar_VisibleTabId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 32, v);
}
function get_ImGuiTabBar_CurrFrameVisible(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 36);
}
function set_ImGuiTabBar_CurrFrameVisible(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 36, v);
}
function get_ImGuiTabBar_PrevFrameVisible(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 40);
}
function set_ImGuiTabBar_PrevFrameVisible(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 40, v);
}
function get_ImGuiTabBar_BarRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 44);
}
function get_ImGuiTabBar_CurrTabsContentsHeight(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 60);
}
function set_ImGuiTabBar_CurrTabsContentsHeight(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 60, v);
}
function get_ImGuiTabBar_PrevTabsContentsHeight(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 64);
}
function set_ImGuiTabBar_PrevTabsContentsHeight(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 64, v);
}
function get_ImGuiTabBar_WidthAllTabs(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 68);
}
function set_ImGuiTabBar_WidthAllTabs(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 68, v);
}
function get_ImGuiTabBar_WidthAllTabsIdeal(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 72);
}
function set_ImGuiTabBar_WidthAllTabsIdeal(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 72, v);
}
function get_ImGuiTabBar_ScrollingAnim(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 76);
}
function set_ImGuiTabBar_ScrollingAnim(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 76, v);
}
function get_ImGuiTabBar_ScrollingTarget(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 80);
}
function set_ImGuiTabBar_ScrollingTarget(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 80, v);
}
function get_ImGuiTabBar_ScrollingTargetDistToVisibility(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 84);
}
function set_ImGuiTabBar_ScrollingTargetDistToVisibility(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 84, v);
}
function get_ImGuiTabBar_ScrollingSpeed(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 88);
}
function set_ImGuiTabBar_ScrollingSpeed(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 88, v);
}
function get_ImGuiTabBar_ScrollingRectMinX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 92);
}
function set_ImGuiTabBar_ScrollingRectMinX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 92, v);
}
function get_ImGuiTabBar_ScrollingRectMaxX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 96);
}
function set_ImGuiTabBar_ScrollingRectMaxX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 96, v);
}
function get_ImGuiTabBar_ReorderRequestTabId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 100);
}
function set_ImGuiTabBar_ReorderRequestTabId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 100, v);
}
function get_ImGuiTabBar_ReorderRequestOffset(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 104);
}
function set_ImGuiTabBar_ReorderRequestOffset(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 104, v);
}
function get_ImGuiTabBar_BeginCount(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 106);
}
function set_ImGuiTabBar_BeginCount(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 106, v);
}
function get_ImGuiTabBar_WantLayout(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 107);
}
function set_ImGuiTabBar_WantLayout(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 107, v);
}
function get_ImGuiTabBar_VisibleTabWasSubmitted(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 108);
}
function set_ImGuiTabBar_VisibleTabWasSubmitted(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 108, v);
}
function get_ImGuiTabBar_TabsAddedNew(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 109);
}
function set_ImGuiTabBar_TabsAddedNew(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 109, v);
}
function get_ImGuiTabBar_TabsActiveCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 110);
}
function set_ImGuiTabBar_TabsActiveCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 110, v);
}
function get_ImGuiTabBar_LastTabItemIdx(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 112);
}
function set_ImGuiTabBar_LastTabItemIdx(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 112, v);
}
function get_ImGuiTabBar_ItemSpacingY(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 116);
}
function set_ImGuiTabBar_ItemSpacingY(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 116, v);
}
function get_ImGuiTabBar_FramePadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 120);
}
function get_ImGuiTabBar_BackupCursorPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 128);
}
function get_ImGuiTabBar_TabsNames(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 136);
}
function get_ImGuiTabItem_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiTabItem_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiTabItem_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiTabItem_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiTabItem_LastFrameVisible(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiTabItem_LastFrameVisible(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiTabItem_LastFrameSelected(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 12);
}
function set_ImGuiTabItem_LastFrameSelected(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 12, v);
}
function get_ImGuiTabItem_Offset(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16);
}
function set_ImGuiTabItem_Offset(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16, v);
}
function get_ImGuiTabItem_Width(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 20);
}
function set_ImGuiTabItem_Width(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 20, v);
}
function get_ImGuiTabItem_ContentWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 24);
}
function set_ImGuiTabItem_ContentWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 24, v);
}
function get_ImGuiTabItem_RequestedWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 28);
}
function set_ImGuiTabItem_RequestedWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 28, v);
}
function get_ImGuiTabItem_NameOffset(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 32);
}
function set_ImGuiTabItem_NameOffset(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 32, v);
}
function get_ImGuiTabItem_BeginOrder(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 36);
}
function set_ImGuiTabItem_BeginOrder(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 36, v);
}
function get_ImGuiTabItem_IndexDuringLayout(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 38);
}
function set_ImGuiTabItem_IndexDuringLayout(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 38, v);
}
function get_ImGuiTabItem_WantClose(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 40);
}
function set_ImGuiTabItem_WantClose(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 40, v);
}
function get_ImGuiTable_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiTable_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiTable_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiTable_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiTable_RawData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImGuiTable_RawData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiTable_TempData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16);
}
function set_ImGuiTable_TempData(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16, v);
}
function get_ImGuiTable_Columns(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24);
}
function get_ImGuiTable_DisplayOrderToIndex(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 40);
}
function get_ImGuiTable_RowCellData(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 56);
}
function get_ImGuiTable_EnabledMaskByDisplayOrder(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 72);
}
function set_ImGuiTable_EnabledMaskByDisplayOrder(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 72, v);
}
function get_ImGuiTable_EnabledMaskByIndex(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 80);
}
function set_ImGuiTable_EnabledMaskByIndex(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 80, v);
}
function get_ImGuiTable_VisibleMaskByIndex(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 88);
}
function set_ImGuiTable_VisibleMaskByIndex(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 88, v);
}
function get_ImGuiTable_SettingsLoadedFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 96);
}
function set_ImGuiTable_SettingsLoadedFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 96, v);
}
function get_ImGuiTable_SettingsOffset(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 100);
}
function set_ImGuiTable_SettingsOffset(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 100, v);
}
function get_ImGuiTable_LastFrameActive(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 104);
}
function set_ImGuiTable_LastFrameActive(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 104, v);
}
function get_ImGuiTable_ColumnsCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 108);
}
function set_ImGuiTable_ColumnsCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 108, v);
}
function get_ImGuiTable_CurrentRow(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 112);
}
function set_ImGuiTable_CurrentRow(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 112, v);
}
function get_ImGuiTable_CurrentColumn(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 116);
}
function set_ImGuiTable_CurrentColumn(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 116, v);
}
function get_ImGuiTable_InstanceCurrent(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 120);
}
function set_ImGuiTable_InstanceCurrent(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 120, v);
}
function get_ImGuiTable_InstanceInteracted(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 122);
}
function set_ImGuiTable_InstanceInteracted(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 122, v);
}
function get_ImGuiTable_RowPosY1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 124);
}
function set_ImGuiTable_RowPosY1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 124, v);
}
function get_ImGuiTable_RowPosY2(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 128);
}
function set_ImGuiTable_RowPosY2(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 128, v);
}
function get_ImGuiTable_RowMinHeight(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 132);
}
function set_ImGuiTable_RowMinHeight(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 132, v);
}
function get_ImGuiTable_RowCellPaddingY(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 136);
}
function set_ImGuiTable_RowCellPaddingY(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 136, v);
}
function get_ImGuiTable_RowTextBaseline(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 140);
}
function set_ImGuiTable_RowTextBaseline(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 140, v);
}
function get_ImGuiTable_RowIndentOffsetX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 144);
}
function set_ImGuiTable_RowIndentOffsetX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 144, v);
}
function get_ImGuiTable_RowFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 148);
}
function set_ImGuiTable_RowFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 148, v);
}
function get_ImGuiTable_LastRowFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 150);
}
function set_ImGuiTable_LastRowFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 150, v);
}
function get_ImGuiTable_RowBgColorCounter(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 152);
}
function set_ImGuiTable_RowBgColorCounter(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 152, v);
}
function get_ImGuiTable_RowBgColor(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 156);
}
function set_ImGuiTable_RowBgColor(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 156, v);
}
function get_ImGuiTable_BorderColorStrong(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 164);
}
function set_ImGuiTable_BorderColorStrong(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 164, v);
}
function get_ImGuiTable_BorderColorLight(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 168);
}
function set_ImGuiTable_BorderColorLight(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 168, v);
}
function get_ImGuiTable_BorderX1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 172);
}
function set_ImGuiTable_BorderX1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 172, v);
}
function get_ImGuiTable_BorderX2(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 176);
}
function set_ImGuiTable_BorderX2(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 176, v);
}
function get_ImGuiTable_HostIndentX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 180);
}
function set_ImGuiTable_HostIndentX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 180, v);
}
function get_ImGuiTable_MinColumnWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 184);
}
function set_ImGuiTable_MinColumnWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 184, v);
}
function get_ImGuiTable_OuterPaddingX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 188);
}
function set_ImGuiTable_OuterPaddingX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 188, v);
}
function get_ImGuiTable_CellPaddingX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 192);
}
function set_ImGuiTable_CellPaddingX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 192, v);
}
function get_ImGuiTable_CellSpacingX1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 196);
}
function set_ImGuiTable_CellSpacingX1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 196, v);
}
function get_ImGuiTable_CellSpacingX2(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 200);
}
function set_ImGuiTable_CellSpacingX2(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 200, v);
}
function get_ImGuiTable_InnerWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 204);
}
function set_ImGuiTable_InnerWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 204, v);
}
function get_ImGuiTable_ColumnsGivenWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 208);
}
function set_ImGuiTable_ColumnsGivenWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 208, v);
}
function get_ImGuiTable_ColumnsAutoFitWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 212);
}
function set_ImGuiTable_ColumnsAutoFitWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 212, v);
}
function get_ImGuiTable_ColumnsStretchSumWeights(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 216);
}
function set_ImGuiTable_ColumnsStretchSumWeights(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 216, v);
}
function get_ImGuiTable_ResizedColumnNextWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 220);
}
function set_ImGuiTable_ResizedColumnNextWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 220, v);
}
function get_ImGuiTable_ResizeLockMinContentsX2(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 224);
}
function set_ImGuiTable_ResizeLockMinContentsX2(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 224, v);
}
function get_ImGuiTable_RefScale(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 228);
}
function set_ImGuiTable_RefScale(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 228, v);
}
function get_ImGuiTable_OuterRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 232);
}
function get_ImGuiTable_InnerRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 248);
}
function get_ImGuiTable_WorkRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 264);
}
function get_ImGuiTable_InnerClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 280);
}
function get_ImGuiTable_BgClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 296);
}
function get_ImGuiTable_Bg0ClipRectForDrawCmd(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 312);
}
function get_ImGuiTable_Bg2ClipRectForDrawCmd(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 328);
}
function get_ImGuiTable_HostClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 344);
}
function get_ImGuiTable_HostBackupInnerClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 360);
}
function get_ImGuiTable_OuterWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 376);
}
function set_ImGuiTable_OuterWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 376, v);
}
function get_ImGuiTable_InnerWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 384);
}
function set_ImGuiTable_InnerWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 384, v);
}
function get_ImGuiTable_ColumnsNames(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 392);
}
function get_ImGuiTable_DrawSplitter(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 408);
}
function set_ImGuiTable_DrawSplitter(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 408, v);
}
function get_ImGuiTable_InstanceDataFirst(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 416);
}
function get_ImGuiTable_InstanceDataExtra(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 440);
}
function get_ImGuiTable_SortSpecsSingle(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 456);
}
function get_ImGuiTable_SortSpecsMulti(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 472);
}
function get_ImGuiTable_SortSpecs(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 488);
}
function get_ImGuiTable_SortSpecsCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 504);
}
function set_ImGuiTable_SortSpecsCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 504, v);
}
function get_ImGuiTable_ColumnsEnabledCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 506);
}
function set_ImGuiTable_ColumnsEnabledCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 506, v);
}
function get_ImGuiTable_ColumnsEnabledFixedCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 508);
}
function set_ImGuiTable_ColumnsEnabledFixedCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 508, v);
}
function get_ImGuiTable_DeclColumnsCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 510);
}
function set_ImGuiTable_DeclColumnsCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 510, v);
}
function get_ImGuiTable_HoveredColumnBody(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 512);
}
function set_ImGuiTable_HoveredColumnBody(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 512, v);
}
function get_ImGuiTable_HoveredColumnBorder(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 514);
}
function set_ImGuiTable_HoveredColumnBorder(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 514, v);
}
function get_ImGuiTable_AutoFitSingleColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 516);
}
function set_ImGuiTable_AutoFitSingleColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 516, v);
}
function get_ImGuiTable_ResizedColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 518);
}
function set_ImGuiTable_ResizedColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 518, v);
}
function get_ImGuiTable_LastResizedColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 520);
}
function set_ImGuiTable_LastResizedColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 520, v);
}
function get_ImGuiTable_HeldHeaderColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 522);
}
function set_ImGuiTable_HeldHeaderColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 522, v);
}
function get_ImGuiTable_ReorderColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 524);
}
function set_ImGuiTable_ReorderColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 524, v);
}
function get_ImGuiTable_ReorderColumnDir(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 526);
}
function set_ImGuiTable_ReorderColumnDir(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 526, v);
}
function get_ImGuiTable_LeftMostEnabledColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 528);
}
function set_ImGuiTable_LeftMostEnabledColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 528, v);
}
function get_ImGuiTable_RightMostEnabledColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 530);
}
function set_ImGuiTable_RightMostEnabledColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 530, v);
}
function get_ImGuiTable_LeftMostStretchedColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 532);
}
function set_ImGuiTable_LeftMostStretchedColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 532, v);
}
function get_ImGuiTable_RightMostStretchedColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 534);
}
function set_ImGuiTable_RightMostStretchedColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 534, v);
}
function get_ImGuiTable_ContextPopupColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 536);
}
function set_ImGuiTable_ContextPopupColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 536, v);
}
function get_ImGuiTable_FreezeRowsRequest(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 538);
}
function set_ImGuiTable_FreezeRowsRequest(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 538, v);
}
function get_ImGuiTable_FreezeRowsCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 540);
}
function set_ImGuiTable_FreezeRowsCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 540, v);
}
function get_ImGuiTable_FreezeColumnsRequest(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 542);
}
function set_ImGuiTable_FreezeColumnsRequest(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 542, v);
}
function get_ImGuiTable_FreezeColumnsCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 544);
}
function set_ImGuiTable_FreezeColumnsCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 544, v);
}
function get_ImGuiTable_RowCellDataCurrent(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 546);
}
function set_ImGuiTable_RowCellDataCurrent(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 546, v);
}
function get_ImGuiTable_DummyDrawChannel(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 548);
}
function set_ImGuiTable_DummyDrawChannel(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 548, v);
}
function get_ImGuiTable_Bg2DrawChannelCurrent(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 550);
}
function set_ImGuiTable_Bg2DrawChannelCurrent(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 550, v);
}
function get_ImGuiTable_Bg2DrawChannelUnfrozen(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 552);
}
function set_ImGuiTable_Bg2DrawChannelUnfrozen(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 552, v);
}
function get_ImGuiTable_IsLayoutLocked(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 554);
}
function set_ImGuiTable_IsLayoutLocked(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 554, v);
}
function get_ImGuiTable_IsInsideRow(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 555);
}
function set_ImGuiTable_IsInsideRow(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 555, v);
}
function get_ImGuiTable_IsInitializing(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 556);
}
function set_ImGuiTable_IsInitializing(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 556, v);
}
function get_ImGuiTable_IsSortSpecsDirty(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 557);
}
function set_ImGuiTable_IsSortSpecsDirty(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 557, v);
}
function get_ImGuiTable_IsUsingHeaders(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 558);
}
function set_ImGuiTable_IsUsingHeaders(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 558, v);
}
function get_ImGuiTable_IsContextPopupOpen(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 559);
}
function set_ImGuiTable_IsContextPopupOpen(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 559, v);
}
function get_ImGuiTable_IsSettingsRequestLoad(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 560);
}
function set_ImGuiTable_IsSettingsRequestLoad(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 560, v);
}
function get_ImGuiTable_IsSettingsDirty(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 561);
}
function set_ImGuiTable_IsSettingsDirty(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 561, v);
}
function get_ImGuiTable_IsDefaultDisplayOrder(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 562);
}
function set_ImGuiTable_IsDefaultDisplayOrder(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 562, v);
}
function get_ImGuiTable_IsResetAllRequest(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 563);
}
function set_ImGuiTable_IsResetAllRequest(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 563, v);
}
function get_ImGuiTable_IsResetDisplayOrderRequest(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 564);
}
function set_ImGuiTable_IsResetDisplayOrderRequest(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 564, v);
}
function get_ImGuiTable_IsUnfrozenRows(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 565);
}
function set_ImGuiTable_IsUnfrozenRows(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 565, v);
}
function get_ImGuiTable_IsDefaultSizingPolicy(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 566);
}
function set_ImGuiTable_IsDefaultSizingPolicy(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 566, v);
}
function get_ImGuiTable_HasScrollbarYCurr(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 567);
}
function set_ImGuiTable_HasScrollbarYCurr(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 567, v);
}
function get_ImGuiTable_HasScrollbarYPrev(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 568);
}
function set_ImGuiTable_HasScrollbarYPrev(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 568, v);
}
function get_ImGuiTable_MemoryCompacted(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 569);
}
function set_ImGuiTable_MemoryCompacted(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 569, v);
}
function get_ImGuiTable_HostSkipItems(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 570);
}
function set_ImGuiTable_HostSkipItems(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 570, v);
}
function get_ImGuiTableColumn_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiTableColumn_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiTableColumn_WidthGiven(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImGuiTableColumn_WidthGiven(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImGuiTableColumn_MinX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_ImGuiTableColumn_MinX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_ImGuiTableColumn_MaxX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 12);
}
function set_ImGuiTableColumn_MaxX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 12, v);
}
function get_ImGuiTableColumn_WidthRequest(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16);
}
function set_ImGuiTableColumn_WidthRequest(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16, v);
}
function get_ImGuiTableColumn_WidthAuto(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 20);
}
function set_ImGuiTableColumn_WidthAuto(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 20, v);
}
function get_ImGuiTableColumn_StretchWeight(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 24);
}
function set_ImGuiTableColumn_StretchWeight(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 24, v);
}
function get_ImGuiTableColumn_InitStretchWeightOrWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 28);
}
function set_ImGuiTableColumn_InitStretchWeightOrWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 28, v);
}
function get_ImGuiTableColumn_ClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 32);
}
function get_ImGuiTableColumn_UserID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 48);
}
function set_ImGuiTableColumn_UserID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 48, v);
}
function get_ImGuiTableColumn_WorkMinX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 52);
}
function set_ImGuiTableColumn_WorkMinX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 52, v);
}
function get_ImGuiTableColumn_WorkMaxX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 56);
}
function set_ImGuiTableColumn_WorkMaxX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 56, v);
}
function get_ImGuiTableColumn_ItemWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 60);
}
function set_ImGuiTableColumn_ItemWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 60, v);
}
function get_ImGuiTableColumn_ContentMaxXFrozen(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 64);
}
function set_ImGuiTableColumn_ContentMaxXFrozen(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 64, v);
}
function get_ImGuiTableColumn_ContentMaxXUnfrozen(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 68);
}
function set_ImGuiTableColumn_ContentMaxXUnfrozen(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 68, v);
}
function get_ImGuiTableColumn_ContentMaxXHeadersUsed(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 72);
}
function set_ImGuiTableColumn_ContentMaxXHeadersUsed(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 72, v);
}
function get_ImGuiTableColumn_ContentMaxXHeadersIdeal(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 76);
}
function set_ImGuiTableColumn_ContentMaxXHeadersIdeal(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 76, v);
}
function get_ImGuiTableColumn_NameOffset(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 80);
}
function set_ImGuiTableColumn_NameOffset(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 80, v);
}
function get_ImGuiTableColumn_DisplayOrder(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 82);
}
function set_ImGuiTableColumn_DisplayOrder(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 82, v);
}
function get_ImGuiTableColumn_IndexWithinEnabledSet(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 84);
}
function set_ImGuiTableColumn_IndexWithinEnabledSet(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 84, v);
}
function get_ImGuiTableColumn_PrevEnabledColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 86);
}
function set_ImGuiTableColumn_PrevEnabledColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 86, v);
}
function get_ImGuiTableColumn_NextEnabledColumn(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 88);
}
function set_ImGuiTableColumn_NextEnabledColumn(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 88, v);
}
function get_ImGuiTableColumn_SortOrder(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 90);
}
function set_ImGuiTableColumn_SortOrder(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 90, v);
}
function get_ImGuiTableColumn_DrawChannelCurrent(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 92);
}
function set_ImGuiTableColumn_DrawChannelCurrent(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 92, v);
}
function get_ImGuiTableColumn_DrawChannelFrozen(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 94);
}
function set_ImGuiTableColumn_DrawChannelFrozen(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 94, v);
}
function get_ImGuiTableColumn_DrawChannelUnfrozen(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 96);
}
function set_ImGuiTableColumn_DrawChannelUnfrozen(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 96, v);
}
function get_ImGuiTableColumn_IsEnabled(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 98);
}
function set_ImGuiTableColumn_IsEnabled(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 98, v);
}
function get_ImGuiTableColumn_IsUserEnabled(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 99);
}
function set_ImGuiTableColumn_IsUserEnabled(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 99, v);
}
function get_ImGuiTableColumn_IsUserEnabledNextFrame(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 100);
}
function set_ImGuiTableColumn_IsUserEnabledNextFrame(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 100, v);
}
function get_ImGuiTableColumn_IsVisibleX(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 101);
}
function set_ImGuiTableColumn_IsVisibleX(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 101, v);
}
function get_ImGuiTableColumn_IsVisibleY(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 102);
}
function set_ImGuiTableColumn_IsVisibleY(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 102, v);
}
function get_ImGuiTableColumn_IsRequestOutput(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 103);
}
function set_ImGuiTableColumn_IsRequestOutput(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 103, v);
}
function get_ImGuiTableColumn_IsSkipItems(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 104);
}
function set_ImGuiTableColumn_IsSkipItems(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 104, v);
}
function get_ImGuiTableColumn_IsPreserveWidthAuto(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 105);
}
function set_ImGuiTableColumn_IsPreserveWidthAuto(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 105, v);
}
function get_ImGuiTableColumn_NavLayerCurrent(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 106);
}
function set_ImGuiTableColumn_NavLayerCurrent(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 106, v);
}
function get_ImGuiTableColumn_AutoFitQueue(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 107);
}
function set_ImGuiTableColumn_AutoFitQueue(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 107, v);
}
function get_ImGuiTableColumn_CannotSkipItemsQueue(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 108);
}
function set_ImGuiTableColumn_CannotSkipItemsQueue(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 108, v);
}
function get_ImGuiTableColumn_SortDirection(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 109);
}
function set_ImGuiTableColumn_SortDirection(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 109, v);
}
function get_ImGuiTableColumn_SortDirectionsAvailCount(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 109);
}
function set_ImGuiTableColumn_SortDirectionsAvailCount(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 109, v);
}
function get_ImGuiTableColumn_SortDirectionsAvailMask(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 109);
}
function set_ImGuiTableColumn_SortDirectionsAvailMask(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 109, v);
}
function get_ImGuiTableColumn_SortDirectionsAvailList(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 110);
}
function set_ImGuiTableColumn_SortDirectionsAvailList(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 110, v);
}
function get_ImGuiTableInstanceData_TableInstanceID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiTableInstanceData_TableInstanceID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiTableInstanceData_LastOuterHeight(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImGuiTableInstanceData_LastOuterHeight(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImGuiTableInstanceData_LastFirstRowHeight(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_ImGuiTableInstanceData_LastFirstRowHeight(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_ImGuiTableInstanceData_LastFrozenHeight(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 12);
}
function set_ImGuiTableInstanceData_LastFrozenHeight(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 12, v);
}
function get_ImGuiTableInstanceData_HoveredRowLast(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_ImGuiTableInstanceData_HoveredRowLast(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_ImGuiTableInstanceData_HoveredRowNext(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 20);
}
function set_ImGuiTableInstanceData_HoveredRowNext(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 20, v);
}
function get_ImGuiTableTempData_TableIndex(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiTableTempData_TableIndex(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiTableTempData_LastTimeActive(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImGuiTableTempData_LastTimeActive(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImGuiTableTempData_UserOuterSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImGuiTableTempData_DrawSplitter(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImGuiTableTempData_HostBackupWorkRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 40);
}
function get_ImGuiTableTempData_HostBackupParentWorkRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 56);
}
function get_ImGuiTableTempData_HostBackupPrevLineSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 72);
}
function get_ImGuiTableTempData_HostBackupCurrLineSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 80);
}
function get_ImGuiTableTempData_HostBackupCursorMaxPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 88);
}
function get_ImGuiTableTempData_HostBackupColumnsOffset(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 96);
}
function get_ImGuiTableTempData_HostBackupItemWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 100);
}
function set_ImGuiTableTempData_HostBackupItemWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 100, v);
}
function get_ImGuiTableTempData_HostBackupItemWidthStackSize(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 104);
}
function set_ImGuiTableTempData_HostBackupItemWidthStackSize(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 104, v);
}
function get_ImGuiTableSettings_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiTableSettings_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiTableSettings_SaveFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiTableSettings_SaveFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiTableSettings_RefScale(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_ImGuiTableSettings_RefScale(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_ImGuiTableSettings_ColumnsCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 12);
}
function set_ImGuiTableSettings_ColumnsCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 12, v);
}
function get_ImGuiTableSettings_ColumnsCountMax(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 14);
}
function set_ImGuiTableSettings_ColumnsCountMax(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 14, v);
}
function get_ImGuiTableSettings_WantApply(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 16);
}
function set_ImGuiTableSettings_WantApply(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 16, v);
}
function get_ImGuiWindow_Ctx(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiWindow_Ctx(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiWindow_Name(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImGuiWindow_Name(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiWindow_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 16);
}
function set_ImGuiWindow_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 16, v);
}
function get_ImGuiWindow_Flags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 20);
}
function set_ImGuiWindow_Flags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 20, v);
}
function get_ImGuiWindow_Viewport(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 24);
}
function set_ImGuiWindow_Viewport(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 24, v);
}
function get_ImGuiWindow_Pos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 32);
}
function get_ImGuiWindow_Size(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 40);
}
function get_ImGuiWindow_SizeFull(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 48);
}
function get_ImGuiWindow_ContentSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 56);
}
function get_ImGuiWindow_ContentSizeIdeal(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 64);
}
function get_ImGuiWindow_ContentSizeExplicit(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 72);
}
function get_ImGuiWindow_WindowPadding(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 80);
}
function get_ImGuiWindow_WindowRounding(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 88);
}
function set_ImGuiWindow_WindowRounding(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 88, v);
}
function get_ImGuiWindow_WindowBorderSize(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 92);
}
function set_ImGuiWindow_WindowBorderSize(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 92, v);
}
function get_ImGuiWindow_DecoOuterSizeX1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 96);
}
function set_ImGuiWindow_DecoOuterSizeX1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 96, v);
}
function get_ImGuiWindow_DecoOuterSizeY1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 100);
}
function set_ImGuiWindow_DecoOuterSizeY1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 100, v);
}
function get_ImGuiWindow_DecoOuterSizeX2(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 104);
}
function set_ImGuiWindow_DecoOuterSizeX2(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 104, v);
}
function get_ImGuiWindow_DecoOuterSizeY2(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 108);
}
function set_ImGuiWindow_DecoOuterSizeY2(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 108, v);
}
function get_ImGuiWindow_DecoInnerSizeX1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 112);
}
function set_ImGuiWindow_DecoInnerSizeX1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 112, v);
}
function get_ImGuiWindow_DecoInnerSizeY1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 116);
}
function set_ImGuiWindow_DecoInnerSizeY1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 116, v);
}
function get_ImGuiWindow_NameBufLen(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 120);
}
function set_ImGuiWindow_NameBufLen(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 120, v);
}
function get_ImGuiWindow_MoveId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 124);
}
function set_ImGuiWindow_MoveId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 124, v);
}
function get_ImGuiWindow_ChildId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 128);
}
function set_ImGuiWindow_ChildId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 128, v);
}
function get_ImGuiWindow_Scroll(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 132);
}
function get_ImGuiWindow_ScrollMax(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 140);
}
function get_ImGuiWindow_ScrollTarget(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 148);
}
function get_ImGuiWindow_ScrollTargetCenterRatio(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 156);
}
function get_ImGuiWindow_ScrollTargetEdgeSnapDist(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 164);
}
function get_ImGuiWindow_ScrollbarSizes(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 172);
}
function get_ImGuiWindow_ScrollbarX(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 180);
}
function set_ImGuiWindow_ScrollbarX(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 180, v);
}
function get_ImGuiWindow_ScrollbarY(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 181);
}
function set_ImGuiWindow_ScrollbarY(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 181, v);
}
function get_ImGuiWindow_Active(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 182);
}
function set_ImGuiWindow_Active(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 182, v);
}
function get_ImGuiWindow_WasActive(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 183);
}
function set_ImGuiWindow_WasActive(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 183, v);
}
function get_ImGuiWindow_WriteAccessed(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 184);
}
function set_ImGuiWindow_WriteAccessed(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 184, v);
}
function get_ImGuiWindow_Collapsed(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 185);
}
function set_ImGuiWindow_Collapsed(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 185, v);
}
function get_ImGuiWindow_WantCollapseToggle(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 186);
}
function set_ImGuiWindow_WantCollapseToggle(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 186, v);
}
function get_ImGuiWindow_SkipItems(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 187);
}
function set_ImGuiWindow_SkipItems(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 187, v);
}
function get_ImGuiWindow_Appearing(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 188);
}
function set_ImGuiWindow_Appearing(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 188, v);
}
function get_ImGuiWindow_Hidden(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 189);
}
function set_ImGuiWindow_Hidden(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 189, v);
}
function get_ImGuiWindow_IsFallbackWindow(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 190);
}
function set_ImGuiWindow_IsFallbackWindow(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 190, v);
}
function get_ImGuiWindow_IsExplicitChild(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 191);
}
function set_ImGuiWindow_IsExplicitChild(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 191, v);
}
function get_ImGuiWindow_HasCloseButton(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 192);
}
function set_ImGuiWindow_HasCloseButton(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 192, v);
}
function get_ImGuiWindow_ResizeBorderHeld(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 193);
}
function set_ImGuiWindow_ResizeBorderHeld(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 193, v);
}
function get_ImGuiWindow_BeginCount(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 194);
}
function set_ImGuiWindow_BeginCount(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 194, v);
}
function get_ImGuiWindow_BeginCountPreviousFrame(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 196);
}
function set_ImGuiWindow_BeginCountPreviousFrame(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 196, v);
}
function get_ImGuiWindow_BeginOrderWithinParent(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 198);
}
function set_ImGuiWindow_BeginOrderWithinParent(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 198, v);
}
function get_ImGuiWindow_BeginOrderWithinContext(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 200);
}
function set_ImGuiWindow_BeginOrderWithinContext(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 200, v);
}
function get_ImGuiWindow_FocusOrder(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 202);
}
function set_ImGuiWindow_FocusOrder(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 202, v);
}
function get_ImGuiWindow_PopupId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 204);
}
function set_ImGuiWindow_PopupId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 204, v);
}
function get_ImGuiWindow_AutoFitFramesX(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 208);
}
function set_ImGuiWindow_AutoFitFramesX(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 208, v);
}
function get_ImGuiWindow_AutoFitFramesY(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 209);
}
function set_ImGuiWindow_AutoFitFramesY(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 209, v);
}
function get_ImGuiWindow_AutoFitChildAxises(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 210);
}
function set_ImGuiWindow_AutoFitChildAxises(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 210, v);
}
function get_ImGuiWindow_AutoFitOnlyGrows(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 211);
}
function set_ImGuiWindow_AutoFitOnlyGrows(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 211, v);
}
function get_ImGuiWindow_AutoPosLastDirection(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 212);
}
function set_ImGuiWindow_AutoPosLastDirection(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 212, v);
}
function get_ImGuiWindow_HiddenFramesCanSkipItems(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 216);
}
function set_ImGuiWindow_HiddenFramesCanSkipItems(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 216, v);
}
function get_ImGuiWindow_HiddenFramesCannotSkipItems(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 217);
}
function set_ImGuiWindow_HiddenFramesCannotSkipItems(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 217, v);
}
function get_ImGuiWindow_HiddenFramesForRenderOnly(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 218);
}
function set_ImGuiWindow_HiddenFramesForRenderOnly(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 218, v);
}
function get_ImGuiWindow_DisableInputsFrames(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 219);
}
function set_ImGuiWindow_DisableInputsFrames(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 219, v);
}
function get_ImGuiWindow_SetWindowPosAllowFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 220);
}
function set_ImGuiWindow_SetWindowPosAllowFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 220, v);
}
function get_ImGuiWindow_SetWindowSizeAllowFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 221);
}
function set_ImGuiWindow_SetWindowSizeAllowFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 221, v);
}
function get_ImGuiWindow_SetWindowCollapsedAllowFlags(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 222);
}
function set_ImGuiWindow_SetWindowCollapsedAllowFlags(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 222, v);
}
function get_ImGuiWindow_SetWindowPosVal(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 224);
}
function get_ImGuiWindow_SetWindowPosPivot(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 232);
}
function get_ImGuiWindow_IDStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 240);
}
function get_ImGuiWindow_DC(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 256);
}
function get_ImGuiWindow_OuterRectClipped(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 488);
}
function get_ImGuiWindow_InnerRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 504);
}
function get_ImGuiWindow_InnerClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 520);
}
function get_ImGuiWindow_WorkRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 536);
}
function get_ImGuiWindow_ParentWorkRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 552);
}
function get_ImGuiWindow_ClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 568);
}
function get_ImGuiWindow_ContentRegionRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 584);
}
function get_ImGuiWindow_HitTestHoleSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 600);
}
function get_ImGuiWindow_HitTestHoleOffset(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 604);
}
function get_ImGuiWindow_LastFrameActive(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 608);
}
function set_ImGuiWindow_LastFrameActive(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 608, v);
}
function get_ImGuiWindow_LastTimeActive(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 612);
}
function set_ImGuiWindow_LastTimeActive(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 612, v);
}
function get_ImGuiWindow_ItemWidthDefault(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 616);
}
function set_ImGuiWindow_ItemWidthDefault(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 616, v);
}
function get_ImGuiWindow_StateStorage(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 624);
}
function get_ImGuiWindow_ColumnsStorage(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 640);
}
function get_ImGuiWindow_FontWindowScale(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 656);
}
function set_ImGuiWindow_FontWindowScale(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 656, v);
}
function get_ImGuiWindow_SettingsOffset(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 660);
}
function set_ImGuiWindow_SettingsOffset(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 660, v);
}
function get_ImGuiWindow_DrawList(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 664);
}
function set_ImGuiWindow_DrawList(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 664, v);
}
function get_ImGuiWindow_DrawListInst(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 672);
}
function get_ImGuiWindow_ParentWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 872);
}
function set_ImGuiWindow_ParentWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 872, v);
}
function get_ImGuiWindow_ParentWindowInBeginStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 880);
}
function set_ImGuiWindow_ParentWindowInBeginStack(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 880, v);
}
function get_ImGuiWindow_RootWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 888);
}
function set_ImGuiWindow_RootWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 888, v);
}
function get_ImGuiWindow_RootWindowPopupTree(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 896);
}
function set_ImGuiWindow_RootWindowPopupTree(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 896, v);
}
function get_ImGuiWindow_RootWindowForTitleBarHighlight(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 904);
}
function set_ImGuiWindow_RootWindowForTitleBarHighlight(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 904, v);
}
function get_ImGuiWindow_RootWindowForNav(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 912);
}
function set_ImGuiWindow_RootWindowForNav(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 912, v);
}
function get_ImGuiWindow_NavLastChildNavWindow(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 920);
}
function set_ImGuiWindow_NavLastChildNavWindow(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 920, v);
}
function get_ImGuiWindow_NavLastIds(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 928);
}
function set_ImGuiWindow_NavLastIds(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 928, v);
}
function get_ImGuiWindow_NavRectRel(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 936);
}
function set_ImGuiWindow_NavRectRel(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 936, v);
}
function get_ImGuiWindow_NavPreferredScoringPosRel(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 968);
}
function set_ImGuiWindow_NavPreferredScoringPosRel(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 968, v);
}
function get_ImGuiWindow_NavRootFocusScopeId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 984);
}
function set_ImGuiWindow_NavRootFocusScopeId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 984, v);
}
function get_ImGuiWindow_MemoryDrawListIdxCapacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 988);
}
function set_ImGuiWindow_MemoryDrawListIdxCapacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 988, v);
}
function get_ImGuiWindow_MemoryDrawListVtxCapacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 992);
}
function set_ImGuiWindow_MemoryDrawListVtxCapacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 992, v);
}
function get_ImGuiWindow_MemoryCompacted(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 996);
}
function set_ImGuiWindow_MemoryCompacted(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 996, v);
}
function get_ImGuiWindowTempData_CursorPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImGuiWindowTempData_CursorPosPrevLine(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImGuiWindowTempData_CursorStartPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImGuiWindowTempData_CursorMaxPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24);
}
function get_ImGuiWindowTempData_IdealMaxPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 32);
}
function get_ImGuiWindowTempData_CurrLineSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 40);
}
function get_ImGuiWindowTempData_PrevLineSize(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 48);
}
function get_ImGuiWindowTempData_CurrLineTextBaseOffset(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 56);
}
function set_ImGuiWindowTempData_CurrLineTextBaseOffset(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 56, v);
}
function get_ImGuiWindowTempData_PrevLineTextBaseOffset(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 60);
}
function set_ImGuiWindowTempData_PrevLineTextBaseOffset(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 60, v);
}
function get_ImGuiWindowTempData_IsSameLine(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 64);
}
function set_ImGuiWindowTempData_IsSameLine(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 64, v);
}
function get_ImGuiWindowTempData_IsSetPos(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 65);
}
function set_ImGuiWindowTempData_IsSetPos(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 65, v);
}
function get_ImGuiWindowTempData_Indent(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 68);
}
function get_ImGuiWindowTempData_ColumnsOffset(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 72);
}
function get_ImGuiWindowTempData_GroupOffset(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 76);
}
function get_ImGuiWindowTempData_CursorStartPosLossyness(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 80);
}
function get_ImGuiWindowTempData_NavLayerCurrent(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 88);
}
function set_ImGuiWindowTempData_NavLayerCurrent(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 88, v);
}
function get_ImGuiWindowTempData_NavLayersActiveMask(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 92);
}
function set_ImGuiWindowTempData_NavLayersActiveMask(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 92, v);
}
function get_ImGuiWindowTempData_NavLayersActiveMaskNext(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 94);
}
function set_ImGuiWindowTempData_NavLayersActiveMaskNext(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 94, v);
}
function get_ImGuiWindowTempData_NavIsScrollPushableX(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 96);
}
function set_ImGuiWindowTempData_NavIsScrollPushableX(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 96, v);
}
function get_ImGuiWindowTempData_NavHideHighlightOneFrame(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 97);
}
function set_ImGuiWindowTempData_NavHideHighlightOneFrame(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 97, v);
}
function get_ImGuiWindowTempData_NavWindowHasScrollY(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 98);
}
function set_ImGuiWindowTempData_NavWindowHasScrollY(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 98, v);
}
function get_ImGuiWindowTempData_MenuBarAppending(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 99);
}
function set_ImGuiWindowTempData_MenuBarAppending(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 99, v);
}
function get_ImGuiWindowTempData_MenuBarOffset(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 100);
}
function get_ImGuiWindowTempData_MenuColumns(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 108);
}
function get_ImGuiWindowTempData_TreeDepth(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 136);
}
function set_ImGuiWindowTempData_TreeDepth(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 136, v);
}
function get_ImGuiWindowTempData_TreeJumpToParentOnPopMask(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 140);
}
function set_ImGuiWindowTempData_TreeJumpToParentOnPopMask(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 140, v);
}
function get_ImGuiWindowTempData_ChildWindows(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 144);
}
function get_ImGuiWindowTempData_StateStorage(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 160);
}
function set_ImGuiWindowTempData_StateStorage(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 160, v);
}
function get_ImGuiWindowTempData_CurrentColumns(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 168);
}
function set_ImGuiWindowTempData_CurrentColumns(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 168, v);
}
function get_ImGuiWindowTempData_CurrentTableIdx(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 176);
}
function set_ImGuiWindowTempData_CurrentTableIdx(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 176, v);
}
function get_ImGuiWindowTempData_LayoutType(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 180);
}
function set_ImGuiWindowTempData_LayoutType(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 180, v);
}
function get_ImGuiWindowTempData_ParentLayoutType(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 184);
}
function set_ImGuiWindowTempData_ParentLayoutType(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 184, v);
}
function get_ImGuiWindowTempData_ItemWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 188);
}
function set_ImGuiWindowTempData_ItemWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 188, v);
}
function get_ImGuiWindowTempData_TextWrapPos(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 192);
}
function set_ImGuiWindowTempData_TextWrapPos(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 192, v);
}
function get_ImGuiWindowTempData_ItemWidthStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 200);
}
function get_ImGuiWindowTempData_TextWrapPosStack(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 216);
}
function get_ImGuiWindowSettings_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiWindowSettings_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiWindowSettings_Pos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 4);
}
function get_ImGuiWindowSettings_Size(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImGuiWindowSettings_Collapsed(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 12);
}
function set_ImGuiWindowSettings_Collapsed(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 12, v);
}
function get_ImGuiWindowSettings_WantApply(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 13);
}
function set_ImGuiWindowSettings_WantApply(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 13, v);
}
function get_ImGuiWindowSettings_WantDelete(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 14);
}
function set_ImGuiWindowSettings_WantDelete(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 14, v);
}
function get_ImVec2_x(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 0);
}
function set_ImVec2_x(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 0, v);
}
function get_ImVec2_y(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImVec2_y(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImVec4_x(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 0);
}
function set_ImVec4_x(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 0, v);
}
function get_ImVec4_y(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImVec4_y(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImVec4_z(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_ImVec4_z(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_ImVec4_w(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 12);
}
function set_ImVec4_w(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 12, v);
}
function get_ImVector_ImWchar_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImWchar_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImWchar_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImWchar_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImWchar_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImWchar_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiTextRange_b(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiTextRange_b(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiTextRange_e(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImGuiTextRange_e(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiTextRange_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiTextRange_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiTextRange_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiTextRange_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiTextRange_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiTextRange_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_char_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_char_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_char_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_char_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_char_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_char_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiStoragePair_key(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiStoragePair_key(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiStoragePair_(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImVector_ImGuiStoragePair_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiStoragePair_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiStoragePair_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiStoragePair_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiStoragePair_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiStoragePair_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImDrawCmdHeader_ClipRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImDrawCmdHeader_TextureId(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16);
}
function set_ImDrawCmdHeader_TextureId(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16, v);
}
function get_ImDrawCmdHeader_VtxOffset(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 24);
}
function set_ImDrawCmdHeader_VtxOffset(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 24, v);
}
function get_ImVector_ImDrawCmd_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImDrawCmd_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImDrawCmd_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImDrawCmd_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImDrawCmd_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImDrawCmd_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImDrawIdx_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImDrawIdx_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImDrawIdx_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImDrawIdx_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImDrawIdx_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImDrawIdx_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImDrawChannel_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImDrawChannel_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImDrawChannel_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImDrawChannel_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImDrawChannel_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImDrawChannel_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImDrawVert_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImDrawVert_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImDrawVert_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImDrawVert_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImDrawVert_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImDrawVert_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImVec4_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImVec4_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImVec4_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImVec4_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImVec4_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImVec4_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImTextureID_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImTextureID_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImTextureID_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImTextureID_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImTextureID_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImTextureID_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImVec2_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImVec2_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImVec2_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImVec2_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImVec2_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImVec2_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImDrawListPtr_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImDrawListPtr_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImDrawListPtr_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImDrawListPtr_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImDrawListPtr_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImDrawListPtr_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImU32_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImU32_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImU32_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImU32_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImU32_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImU32_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImFontAtlasCustomRect_Width(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 0);
}
function set_ImFontAtlasCustomRect_Width(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 0, v);
}
function get_ImFontAtlasCustomRect_Height(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 2);
}
function set_ImFontAtlasCustomRect_Height(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 2, v);
}
function get_ImFontAtlasCustomRect_X(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 4);
}
function set_ImFontAtlasCustomRect_X(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 4, v);
}
function get_ImFontAtlasCustomRect_Y(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 6);
}
function set_ImFontAtlasCustomRect_Y(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 6, v);
}
function get_ImFontAtlasCustomRect_GlyphID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 8);
}
function set_ImFontAtlasCustomRect_GlyphID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 8, v);
}
function get_ImFontAtlasCustomRect_GlyphAdvanceX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 12);
}
function set_ImFontAtlasCustomRect_GlyphAdvanceX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 12, v);
}
function get_ImFontAtlasCustomRect_GlyphOffset(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImFontAtlasCustomRect_Font(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 24);
}
function set_ImFontAtlasCustomRect_Font(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 24, v);
}
function get_ImVector_ImFontPtr_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImFontPtr_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImFontPtr_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImFontPtr_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImFontPtr_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImFontPtr_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImFontAtlasCustomRect_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImFontAtlasCustomRect_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImFontAtlasCustomRect_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImFontAtlasCustomRect_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImFontAtlasCustomRect_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImFontAtlasCustomRect_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImFontConfig_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImFontConfig_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImFontConfig_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImFontConfig_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImFontConfig_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImFontConfig_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_float_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_float_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_float_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_float_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_float_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_float_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImFontGlyph_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImFontGlyph_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImFontGlyph_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImFontGlyph_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImFontGlyph_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImFontGlyph_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_StbUndoRecord_where(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_StbUndoRecord_where(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_StbUndoRecord_insert_length(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_StbUndoRecord_insert_length(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_StbUndoRecord_delete_length(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_StbUndoRecord_delete_length(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_StbUndoRecord_char_storage(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 12);
}
function set_StbUndoRecord_char_storage(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 12, v);
}
function get_StbUndoState_undo_rec(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_StbUndoState_undo_rec(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_StbUndoState_undo_char(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 1584);
}
function set_StbUndoState_undo_char(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 1584, v);
}
function get_StbUndoState_undo_point(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 3582);
}
function set_StbUndoState_undo_point(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 3582, v);
}
function get_StbUndoState_redo_point(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 3584);
}
function set_StbUndoState_redo_point(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 3584, v);
}
function get_StbUndoState_undo_char_point(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 3588);
}
function set_StbUndoState_undo_char_point(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 3588, v);
}
function get_StbUndoState_redo_char_point(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 3592);
}
function set_StbUndoState_redo_char_point(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 3592, v);
}
function get_STB_TexteditState_cursor(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_STB_TexteditState_cursor(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_STB_TexteditState_select_start(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_STB_TexteditState_select_start(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_STB_TexteditState_select_end(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_STB_TexteditState_select_end(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_STB_TexteditState_insert_mode(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 12);
}
function set_STB_TexteditState_insert_mode(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 12, v);
}
function get_STB_TexteditState_row_count_per_page(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_STB_TexteditState_row_count_per_page(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_STB_TexteditState_cursor_at_end_of_line(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 20);
}
function set_STB_TexteditState_cursor_at_end_of_line(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 20, v);
}
function get_STB_TexteditState_initialized(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 21);
}
function set_STB_TexteditState_initialized(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 21, v);
}
function get_STB_TexteditState_has_preferred_x(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 22);
}
function set_STB_TexteditState_has_preferred_x(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 22, v);
}
function get_STB_TexteditState_single_line(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 23);
}
function set_STB_TexteditState_single_line(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 23, v);
}
function get_STB_TexteditState_padding1(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 24);
}
function set_STB_TexteditState_padding1(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 24, v);
}
function get_STB_TexteditState_padding2(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 25);
}
function set_STB_TexteditState_padding2(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 25, v);
}
function get_STB_TexteditState_padding3(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 26);
}
function set_STB_TexteditState_padding3(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 26, v);
}
function get_STB_TexteditState_preferred_x(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 28);
}
function set_STB_TexteditState_preferred_x(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 28, v);
}
function get_STB_TexteditState_undostate(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 32);
}
function get_StbTexteditRow_x0(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 0);
}
function set_StbTexteditRow_x0(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 0, v);
}
function get_StbTexteditRow_x1(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_StbTexteditRow_x1(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_StbTexteditRow_baseline_y_delta(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_StbTexteditRow_baseline_y_delta(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_StbTexteditRow_ymin(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 12);
}
function set_StbTexteditRow_ymin(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 12, v);
}
function get_StbTexteditRow_ymax(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16);
}
function set_StbTexteditRow_ymax(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16, v);
}
function get_StbTexteditRow_num_chars(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 20);
}
function set_StbTexteditRow_num_chars(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 20, v);
}
function get_ImVec1_x(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 0);
}
function set_ImVec1_x(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 0, v);
}
function get_ImVec2ih_x(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 0);
}
function set_ImVec2ih_x(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 0, v);
}
function get_ImVec2ih_y(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 2);
}
function set_ImVec2ih_y(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 2, v);
}
function get_ImGuiTextIndex_LineOffsets(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImGuiTextIndex_EndOffset(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_ImGuiTextIndex_EndOffset(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_ImVector_int_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_int_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_int_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_int_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_int_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_int_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiDataTypeTempStorage_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiDataTypeTempStorage_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiComboPreviewData_PreviewRect(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImGuiComboPreviewData_BackupCursorPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImGuiComboPreviewData_BackupCursorMaxPos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24);
}
function get_ImGuiComboPreviewData_BackupCursorPosPrevLine(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 32);
}
function get_ImGuiComboPreviewData_BackupPrevLineTextBaseOffset(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 40);
}
function set_ImGuiComboPreviewData_BackupPrevLineTextBaseOffset(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 40, v);
}
function get_ImGuiComboPreviewData_BackupLayout(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 44);
}
function set_ImGuiComboPreviewData_BackupLayout(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 44, v);
}
function get_ImGuiInputTextDeactivatedState_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiInputTextDeactivatedState_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiInputTextDeactivatedState_TextA(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImGuiWindowStackData_Window(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiWindowStackData_Window(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiWindowStackData_ParentLastItemDataBackup(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 8);
}
function get_ImGuiWindowStackData_StackSizesOnBegin(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 68);
}
function get_ImGuiShrinkWidthItem_Index(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiShrinkWidthItem_Index(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiShrinkWidthItem_Width(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImGuiShrinkWidthItem_Width(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImGuiShrinkWidthItem_InitialWidth(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_ImGuiShrinkWidthItem_InitialWidth(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_ImGuiPtrOrIndex_Ptr(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiPtrOrIndex_Ptr(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiPtrOrIndex_Index(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiPtrOrIndex_Index(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImBitArray_ImGuiKey_NamedKey_COUNT__lessImGuiKey_NamedKey_BEGIN_Storage(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImBitArray_ImGuiKey_NamedKey_COUNT__lessImGuiKey_NamedKey_BEGIN_Storage(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiInputEventMousePos_PosX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 0);
}
function set_ImGuiInputEventMousePos_PosX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 0, v);
}
function get_ImGuiInputEventMousePos_PosY(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImGuiInputEventMousePos_PosY(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImGuiInputEventMousePos_MouseSource(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiInputEventMousePos_MouseSource(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiInputEventMouseWheel_WheelX(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 0);
}
function set_ImGuiInputEventMouseWheel_WheelX(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 0, v);
}
function get_ImGuiInputEventMouseWheel_WheelY(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 4);
}
function set_ImGuiInputEventMouseWheel_WheelY(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 4, v);
}
function get_ImGuiInputEventMouseWheel_MouseSource(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiInputEventMouseWheel_MouseSource(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiInputEventMouseButton_Button(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiInputEventMouseButton_Button(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiInputEventMouseButton_Down(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 4);
}
function set_ImGuiInputEventMouseButton_Down(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 4, v);
}
function get_ImGuiInputEventMouseButton_MouseSource(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 8);
}
function set_ImGuiInputEventMouseButton_MouseSource(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 8, v);
}
function get_ImGuiInputEventKey_Key(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiInputEventKey_Key(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiInputEventKey_Down(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 4);
}
function set_ImGuiInputEventKey_Down(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 4, v);
}
function get_ImGuiInputEventKey_AnalogValue(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_ImGuiInputEventKey_AnalogValue(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_ImGuiInputEventText_Char(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiInputEventText_Char(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiInputEventAppFocused_Focused(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 0);
}
function set_ImGuiInputEventAppFocused_Focused(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 0, v);
}
function get_ImGuiInputEvent_Type(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiInputEvent_Type(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiInputEvent_Source(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiInputEvent_Source(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiInputEvent_EventId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 8);
}
function set_ImGuiInputEvent_EventId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 8, v);
}
function get_ImGuiInputEvent_(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 12);
}
function get_ImGuiInputEvent_AddedByTestEngine(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 24);
}
function set_ImGuiInputEvent_AddedByTestEngine(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 24, v);
}
function get_ImGuiKeyRoutingData_NextEntryIndex(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 0);
}
function set_ImGuiKeyRoutingData_NextEntryIndex(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 0, v);
}
function get_ImGuiKeyRoutingData_Mods(s: c_ptr): c_ushort {
  "inline";
  return _sh_ptr_read_c_ushort(s, 2);
}
function set_ImGuiKeyRoutingData_Mods(s: c_ptr, v: c_ushort): void {
  "inline";
  _sh_ptr_write_c_ushort(s, 2, v);
}
function get_ImGuiKeyRoutingData_RoutingNextScore(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 4);
}
function set_ImGuiKeyRoutingData_RoutingNextScore(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 4, v);
}
function get_ImGuiKeyRoutingData_RoutingCurr(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 8);
}
function set_ImGuiKeyRoutingData_RoutingCurr(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 8, v);
}
function get_ImGuiKeyRoutingData_RoutingNext(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 12);
}
function set_ImGuiKeyRoutingData_RoutingNext(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 12, v);
}
function get_ImGuiKeyRoutingTable_Index(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiKeyRoutingTable_Index(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiKeyRoutingTable_Entries(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 280);
}
function get_ImGuiKeyRoutingTable_EntriesNext(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 296);
}
function get_ImVector_ImGuiKeyRoutingData_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiKeyRoutingData_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiKeyRoutingData_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiKeyRoutingData_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiKeyRoutingData_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiKeyRoutingData_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiKeyOwnerData_OwnerCurr(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiKeyOwnerData_OwnerCurr(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiKeyOwnerData_OwnerNext(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 4);
}
function set_ImGuiKeyOwnerData_OwnerNext(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 4, v);
}
function get_ImGuiKeyOwnerData_LockThisFrame(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 8);
}
function set_ImGuiKeyOwnerData_LockThisFrame(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 8, v);
}
function get_ImGuiKeyOwnerData_LockUntilRelease(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 9);
}
function set_ImGuiKeyOwnerData_LockUntilRelease(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 9, v);
}
function get_ImGuiListClipperRange_Min(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiListClipperRange_Min(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiListClipperRange_Max(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiListClipperRange_Max(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiListClipperRange_PosToIndexConvert(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 8);
}
function set_ImGuiListClipperRange_PosToIndexConvert(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 8, v);
}
function get_ImGuiListClipperRange_PosToIndexOffsetMin(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 9);
}
function set_ImGuiListClipperRange_PosToIndexOffsetMin(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 9, v);
}
function get_ImGuiListClipperRange_PosToIndexOffsetMax(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 10);
}
function set_ImGuiListClipperRange_PosToIndexOffsetMax(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 10, v);
}
function get_ImGuiListClipperData_ListClipper(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImGuiListClipperData_ListClipper(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImGuiListClipperData_LossynessOffset(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 8);
}
function set_ImGuiListClipperData_LossynessOffset(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 8, v);
}
function get_ImGuiListClipperData_StepNo(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 12);
}
function set_ImGuiListClipperData_StepNo(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 12, v);
}
function get_ImGuiListClipperData_ItemsFrozen(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 16);
}
function set_ImGuiListClipperData_ItemsFrozen(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 16, v);
}
function get_ImGuiListClipperData_Ranges(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 24);
}
function get_ImVector_ImGuiListClipperRange_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiListClipperRange_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiListClipperRange_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiListClipperRange_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiListClipperRange_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiListClipperRange_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiOldColumnData_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiOldColumnData_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiOldColumnData_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiOldColumnData_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiOldColumnData_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiOldColumnData_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiViewportP__ImGuiViewport(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImGuiViewportP_BgFgDrawListsLastFrame(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 48);
}
function set_ImGuiViewportP_BgFgDrawListsLastFrame(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 48, v);
}
function get_ImGuiViewportP_BgFgDrawLists(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 56);
}
function set_ImGuiViewportP_BgFgDrawLists(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 56, v);
}
function get_ImGuiViewportP_DrawDataP(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 72);
}
function get_ImGuiViewportP_DrawDataBuilder(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 136);
}
function get_ImGuiViewportP_WorkOffsetMin(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 168);
}
function get_ImGuiViewportP_WorkOffsetMax(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 176);
}
function get_ImGuiViewportP_BuildWorkOffsetMin(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 184);
}
function get_ImGuiViewportP_BuildWorkOffsetMax(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 192);
}
function get_ImGuiStackLevelInfo_ID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiStackLevelInfo_ID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiStackLevelInfo_QueryFrameCount(s: c_ptr): c_schar {
  "inline";
  return _sh_ptr_read_c_schar(s, 4);
}
function set_ImGuiStackLevelInfo_QueryFrameCount(s: c_ptr, v: c_schar): void {
  "inline";
  _sh_ptr_write_c_schar(s, 4, v);
}
function get_ImGuiStackLevelInfo_QuerySuccess(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 5);
}
function set_ImGuiStackLevelInfo_QuerySuccess(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 5, v);
}
function get_ImGuiStackLevelInfo_DataType(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 6);
}
function set_ImGuiStackLevelInfo_DataType(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 6, v);
}
function get_ImGuiStackLevelInfo_Desc(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 7);
}
function set_ImGuiStackLevelInfo_Desc(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 7, v);
}
function get_ImGuiStackTool_LastActiveFrame(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImGuiStackTool_LastActiveFrame(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImGuiStackTool_StackLevel(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImGuiStackTool_StackLevel(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImGuiStackTool_QueryId(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 8);
}
function set_ImGuiStackTool_QueryId(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 8, v);
}
function get_ImGuiStackTool_Results(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImGuiStackTool_CopyToClipboardOnCtrlC(s: c_ptr): c_bool {
  "inline";
  return _sh_ptr_read_c_bool(s, 32);
}
function set_ImGuiStackTool_CopyToClipboardOnCtrlC(s: c_ptr, v: c_bool): void {
  "inline";
  _sh_ptr_write_c_bool(s, 32, v);
}
function get_ImGuiStackTool_CopyToClipboardLastTime(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 36);
}
function set_ImGuiStackTool_CopyToClipboardLastTime(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 36, v);
}
function get_ImVector_ImGuiStackLevelInfo_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiStackLevelInfo_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiStackLevelInfo_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiStackLevelInfo_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiStackLevelInfo_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiStackLevelInfo_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiInputEvent_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiInputEvent_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiInputEvent_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiInputEvent_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiInputEvent_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiInputEvent_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiWindowPtr_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiWindowPtr_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiWindowPtr_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiWindowPtr_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiWindowPtr_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiWindowPtr_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiWindowStackData_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiWindowStackData_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiWindowStackData_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiWindowStackData_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiWindowStackData_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiWindowStackData_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiColorMod_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiColorMod_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiColorMod_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiColorMod_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiColorMod_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiColorMod_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiStyleMod_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiStyleMod_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiStyleMod_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiStyleMod_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiStyleMod_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiStyleMod_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiID_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiID_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiID_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiID_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiID_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiID_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiItemFlags_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiItemFlags_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiItemFlags_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiItemFlags_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiItemFlags_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiItemFlags_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiGroupData_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiGroupData_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiGroupData_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiGroupData_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiGroupData_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiGroupData_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiPopupData_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiPopupData_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiPopupData_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiPopupData_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiPopupData_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiPopupData_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiNavTreeNodeData_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiNavTreeNodeData_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiNavTreeNodeData_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiNavTreeNodeData_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiNavTreeNodeData_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiNavTreeNodeData_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiViewportPPtr_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiViewportPPtr_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiViewportPPtr_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiViewportPPtr_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiViewportPPtr_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiViewportPPtr_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_unsigned_char_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_unsigned_char_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_unsigned_char_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_unsigned_char_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_unsigned_char_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_unsigned_char_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiListClipperData_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiListClipperData_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiListClipperData_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiListClipperData_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiListClipperData_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiListClipperData_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiTableTempData_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiTableTempData_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiTableTempData_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiTableTempData_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiTableTempData_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiTableTempData_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiTable_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiTable_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiTable_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiTable_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiTable_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiTable_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImPool_ImGuiTable_Buf(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImPool_ImGuiTable_Map(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImPool_ImGuiTable_FreeIdx(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 32);
}
function set_ImPool_ImGuiTable_FreeIdx(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 32, v);
}
function get_ImPool_ImGuiTable_AliveCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 36);
}
function set_ImPool_ImGuiTable_AliveCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 36, v);
}
function get_ImVector_ImGuiTabBar_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiTabBar_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiTabBar_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiTabBar_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiTabBar_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiTabBar_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImPool_ImGuiTabBar_Buf(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImPool_ImGuiTabBar_Map(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 16);
}
function get_ImPool_ImGuiTabBar_FreeIdx(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 32);
}
function set_ImPool_ImGuiTabBar_FreeIdx(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 32, v);
}
function get_ImPool_ImGuiTabBar_AliveCount(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 36);
}
function set_ImPool_ImGuiTabBar_AliveCount(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 36, v);
}
function get_ImVector_ImGuiPtrOrIndex_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiPtrOrIndex_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiPtrOrIndex_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiPtrOrIndex_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiPtrOrIndex_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiPtrOrIndex_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiShrinkWidthItem_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiShrinkWidthItem_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiShrinkWidthItem_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiShrinkWidthItem_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiShrinkWidthItem_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiShrinkWidthItem_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiSettingsHandler_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiSettingsHandler_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiSettingsHandler_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiSettingsHandler_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiSettingsHandler_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiSettingsHandler_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImChunkStream_ImGuiWindowSettings_Buf(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImChunkStream_ImGuiTableSettings_Buf(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_ImVector_ImGuiContextHook_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiContextHook_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiContextHook_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiContextHook_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiContextHook_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiContextHook_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiOldColumns_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiOldColumns_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiOldColumns_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiOldColumns_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiOldColumns_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiOldColumns_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiTabItem_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiTabItem_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiTabItem_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiTabItem_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiTabItem_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiTabItem_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiTableCellData_BgColor(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_ImGuiTableCellData_BgColor(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_ImGuiTableCellData_Column(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 4);
}
function set_ImGuiTableCellData_Column(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 4, v);
}
function get_ImSpan_ImGuiTableColumn_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImSpan_ImGuiTableColumn_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImSpan_ImGuiTableColumn_DataEnd(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImSpan_ImGuiTableColumn_DataEnd(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImSpan_ImGuiTableColumnIdx_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImSpan_ImGuiTableColumnIdx_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImSpan_ImGuiTableColumnIdx_DataEnd(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImSpan_ImGuiTableColumnIdx_DataEnd(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImSpan_ImGuiTableCellData_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_ImSpan_ImGuiTableCellData_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_ImSpan_ImGuiTableCellData_DataEnd(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImSpan_ImGuiTableCellData_DataEnd(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiTableInstanceData_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiTableInstanceData_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiTableInstanceData_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiTableInstanceData_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiTableInstanceData_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiTableInstanceData_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImVector_ImGuiTableColumnSortSpecs_Size(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_ImVector_ImGuiTableColumnSortSpecs_Size(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_ImVector_ImGuiTableColumnSortSpecs_Capacity(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_ImVector_ImGuiTableColumnSortSpecs_Capacity(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_ImVector_ImGuiTableColumnSortSpecs_Data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_ImVector_ImGuiTableColumnSortSpecs_Data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_ImGuiTableColumnSettings_WidthOrWeight(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 0);
}
function set_ImGuiTableColumnSettings_WidthOrWeight(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 0, v);
}
function get_ImGuiTableColumnSettings_UserID(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 4);
}
function set_ImGuiTableColumnSettings_UserID(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 4, v);
}
function get_ImGuiTableColumnSettings_Index(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 8);
}
function set_ImGuiTableColumnSettings_Index(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 8, v);
}
function get_ImGuiTableColumnSettings_DisplayOrder(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 10);
}
function set_ImGuiTableColumnSettings_DisplayOrder(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 10, v);
}
function get_ImGuiTableColumnSettings_SortOrder(s: c_ptr): c_short {
  "inline";
  return _sh_ptr_read_c_short(s, 12);
}
function set_ImGuiTableColumnSettings_SortOrder(s: c_ptr, v: c_short): void {
  "inline";
  _sh_ptr_write_c_short(s, 12, v);
}
function get_ImGuiTableColumnSettings_SortDirection(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 14);
}
function set_ImGuiTableColumnSettings_SortDirection(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 14, v);
}
function get_ImGuiTableColumnSettings_IsEnabled(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 14);
}
function set_ImGuiTableColumnSettings_IsEnabled(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 14, v);
}
function get_ImGuiTableColumnSettings_IsStretch(s: c_ptr): c_uchar {
  "inline";
  return _sh_ptr_read_c_uchar(s, 14);
}
function set_ImGuiTableColumnSettings_IsStretch(s: c_ptr, v: c_uchar): void {
  "inline";
  _sh_ptr_write_c_uchar(s, 14, v);
}
function get_simgui_image_t_id(s: c_ptr): c_uint {
  "inline";
  return _sh_ptr_read_c_uint(s, 0);
}
function set_simgui_image_t_id(s: c_ptr, v: c_uint): void {
  "inline";
  _sh_ptr_write_c_uint(s, 0, v);
}
function get_simgui_allocator_t_alloc_fn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_simgui_allocator_t_alloc_fn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_simgui_allocator_t_free_fn(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_simgui_allocator_t_free_fn(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_simgui_allocator_t_user_data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 16);
}
function set_simgui_allocator_t_user_data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 16, v);
}
function get_simgui_logger_t_func(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_simgui_logger_t_func(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_simgui_logger_t_user_data(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 8);
}
function set_simgui_logger_t_user_data(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 8, v);
}
function get_simgui_frame_desc_t_width(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_simgui_frame_desc_t_width(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_simgui_frame_desc_t_height(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 4);
}
function set_simgui_frame_desc_t_height(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 4, v);
}
function get_simgui_frame_desc_t_delta_time(s: c_ptr): c_double {
  "inline";
  return _sh_ptr_read_c_double(s, 8);
}
function set_simgui_frame_desc_t_delta_time(s: c_ptr, v: c_double): void {
  "inline";
  _sh_ptr_write_c_double(s, 8, v);
}
function get_simgui_frame_desc_t_dpi_scale(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 16);
}
function set_simgui_frame_desc_t_dpi_scale(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 16, v);
}
function get_None_BackupInt(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_None_BackupInt(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_None_BackupFloat(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_None_BackupFloat(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_None_val_i(s: c_ptr): c_int {
  "inline";
  return _sh_ptr_read_c_int(s, 0);
}
function set_None_val_i(s: c_ptr, v: c_int): void {
  "inline";
  _sh_ptr_write_c_int(s, 0, v);
}
function get_None_val_f(s: c_ptr): c_float {
  "inline";
  return _sh_ptr_read_c_float(s, 0);
}
function set_None_val_f(s: c_ptr, v: c_float): void {
  "inline";
  _sh_ptr_write_c_float(s, 0, v);
}
function get_None_val_p(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_read_c_ptr(s, 0);
}
function set_None_val_p(s: c_ptr, v: c_ptr): void {
  "inline";
  _sh_ptr_write_c_ptr(s, 0, v);
}
function get_None_MousePos(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_None_MouseWheel(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_None_MouseButton(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_None_Key(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_None_Text(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
function get_None_AppFocused(s: c_ptr): c_ptr {
  "inline";
  return _sh_ptr_add(s, 0);
}
const _ImGuiWindowFlags_None = 0;
const _ImGuiWindowFlags_NoTitleBar = 1;
const _ImGuiWindowFlags_NoResize = 2;
const _ImGuiWindowFlags_NoMove = 4;
const _ImGuiWindowFlags_NoScrollbar = 8;
const _ImGuiWindowFlags_NoScrollWithMouse = 16;
const _ImGuiWindowFlags_NoCollapse = 32;
const _ImGuiWindowFlags_AlwaysAutoResize = 64;
const _ImGuiWindowFlags_NoBackground = 128;
const _ImGuiWindowFlags_NoSavedSettings = 256;
const _ImGuiWindowFlags_NoMouseInputs = 512;
const _ImGuiWindowFlags_MenuBar = 1024;
const _ImGuiWindowFlags_HorizontalScrollbar = 2048;
const _ImGuiWindowFlags_NoFocusOnAppearing = 4096;
const _ImGuiWindowFlags_NoBringToFrontOnFocus = 8192;
const _ImGuiWindowFlags_AlwaysVerticalScrollbar = 16384;
const _ImGuiWindowFlags_AlwaysHorizontalScrollbar = 32768;
const _ImGuiWindowFlags_AlwaysUseWindowPadding = 65536;
const _ImGuiWindowFlags_NoNavInputs = 262144;
const _ImGuiWindowFlags_NoNavFocus = 524288;
const _ImGuiWindowFlags_UnsavedDocument = 1048576;
const _ImGuiWindowFlags_NoNav = 786432;
const _ImGuiWindowFlags_NoDecoration = 43;
const _ImGuiWindowFlags_NoInputs = 786944;
const _ImGuiWindowFlags_NavFlattened = 8388608;
const _ImGuiWindowFlags_ChildWindow = 16777216;
const _ImGuiWindowFlags_Tooltip = 33554432;
const _ImGuiWindowFlags_Popup = 67108864;
const _ImGuiWindowFlags_Modal = 134217728;
const _ImGuiWindowFlags_ChildMenu = 268435456;
const _ImGuiInputTextFlags_None = 0;
const _ImGuiInputTextFlags_CharsDecimal = 1;
const _ImGuiInputTextFlags_CharsHexadecimal = 2;
const _ImGuiInputTextFlags_CharsUppercase = 4;
const _ImGuiInputTextFlags_CharsNoBlank = 8;
const _ImGuiInputTextFlags_AutoSelectAll = 16;
const _ImGuiInputTextFlags_EnterReturnsTrue = 32;
const _ImGuiInputTextFlags_CallbackCompletion = 64;
const _ImGuiInputTextFlags_CallbackHistory = 128;
const _ImGuiInputTextFlags_CallbackAlways = 256;
const _ImGuiInputTextFlags_CallbackCharFilter = 512;
const _ImGuiInputTextFlags_AllowTabInput = 1024;
const _ImGuiInputTextFlags_CtrlEnterForNewLine = 2048;
const _ImGuiInputTextFlags_NoHorizontalScroll = 4096;
const _ImGuiInputTextFlags_AlwaysOverwrite = 8192;
const _ImGuiInputTextFlags_ReadOnly = 16384;
const _ImGuiInputTextFlags_Password = 32768;
const _ImGuiInputTextFlags_NoUndoRedo = 65536;
const _ImGuiInputTextFlags_CharsScientific = 131072;
const _ImGuiInputTextFlags_CallbackResize = 262144;
const _ImGuiInputTextFlags_CallbackEdit = 524288;
const _ImGuiInputTextFlags_EscapeClearsAll = 1048576;
const _ImGuiTreeNodeFlags_None = 0;
const _ImGuiTreeNodeFlags_Selected = 1;
const _ImGuiTreeNodeFlags_Framed = 2;
const _ImGuiTreeNodeFlags_AllowOverlap = 4;
const _ImGuiTreeNodeFlags_NoTreePushOnOpen = 8;
const _ImGuiTreeNodeFlags_NoAutoOpenOnLog = 16;
const _ImGuiTreeNodeFlags_DefaultOpen = 32;
const _ImGuiTreeNodeFlags_OpenOnDoubleClick = 64;
const _ImGuiTreeNodeFlags_OpenOnArrow = 128;
const _ImGuiTreeNodeFlags_Leaf = 256;
const _ImGuiTreeNodeFlags_Bullet = 512;
const _ImGuiTreeNodeFlags_FramePadding = 1024;
const _ImGuiTreeNodeFlags_SpanAvailWidth = 2048;
const _ImGuiTreeNodeFlags_SpanFullWidth = 4096;
const _ImGuiTreeNodeFlags_NavLeftJumpsBackHere = 8192;
const _ImGuiTreeNodeFlags_CollapsingHeader = 26;
const _ImGuiPopupFlags_None = 0;
const _ImGuiPopupFlags_MouseButtonLeft = 0;
const _ImGuiPopupFlags_MouseButtonRight = 1;
const _ImGuiPopupFlags_MouseButtonMiddle = 2;
const _ImGuiPopupFlags_MouseButtonMask_ = 31;
const _ImGuiPopupFlags_MouseButtonDefault_ = 1;
const _ImGuiPopupFlags_NoOpenOverExistingPopup = 32;
const _ImGuiPopupFlags_NoOpenOverItems = 64;
const _ImGuiPopupFlags_AnyPopupId = 128;
const _ImGuiPopupFlags_AnyPopupLevel = 256;
const _ImGuiPopupFlags_AnyPopup = 384;
const _ImGuiSelectableFlags_None = 0;
const _ImGuiSelectableFlags_DontClosePopups = 1;
const _ImGuiSelectableFlags_SpanAllColumns = 2;
const _ImGuiSelectableFlags_AllowDoubleClick = 4;
const _ImGuiSelectableFlags_Disabled = 8;
const _ImGuiSelectableFlags_AllowOverlap = 16;
const _ImGuiComboFlags_None = 0;
const _ImGuiComboFlags_PopupAlignLeft = 1;
const _ImGuiComboFlags_HeightSmall = 2;
const _ImGuiComboFlags_HeightRegular = 4;
const _ImGuiComboFlags_HeightLarge = 8;
const _ImGuiComboFlags_HeightLargest = 16;
const _ImGuiComboFlags_NoArrowButton = 32;
const _ImGuiComboFlags_NoPreview = 64;
const _ImGuiComboFlags_HeightMask_ = 30;
const _ImGuiTabBarFlags_None = 0;
const _ImGuiTabBarFlags_Reorderable = 1;
const _ImGuiTabBarFlags_AutoSelectNewTabs = 2;
const _ImGuiTabBarFlags_TabListPopupButton = 4;
const _ImGuiTabBarFlags_NoCloseWithMiddleMouseButton = 8;
const _ImGuiTabBarFlags_NoTabListScrollingButtons = 16;
const _ImGuiTabBarFlags_NoTooltip = 32;
const _ImGuiTabBarFlags_FittingPolicyResizeDown = 64;
const _ImGuiTabBarFlags_FittingPolicyScroll = 128;
const _ImGuiTabBarFlags_FittingPolicyMask_ = 192;
const _ImGuiTabBarFlags_FittingPolicyDefault_ = 64;
const _ImGuiTabItemFlags_None = 0;
const _ImGuiTabItemFlags_UnsavedDocument = 1;
const _ImGuiTabItemFlags_SetSelected = 2;
const _ImGuiTabItemFlags_NoCloseWithMiddleMouseButton = 4;
const _ImGuiTabItemFlags_NoPushId = 8;
const _ImGuiTabItemFlags_NoTooltip = 16;
const _ImGuiTabItemFlags_NoReorder = 32;
const _ImGuiTabItemFlags_Leading = 64;
const _ImGuiTabItemFlags_Trailing = 128;
const _ImGuiTableFlags_None = 0;
const _ImGuiTableFlags_Resizable = 1;
const _ImGuiTableFlags_Reorderable = 2;
const _ImGuiTableFlags_Hideable = 4;
const _ImGuiTableFlags_Sortable = 8;
const _ImGuiTableFlags_NoSavedSettings = 16;
const _ImGuiTableFlags_ContextMenuInBody = 32;
const _ImGuiTableFlags_RowBg = 64;
const _ImGuiTableFlags_BordersInnerH = 128;
const _ImGuiTableFlags_BordersOuterH = 256;
const _ImGuiTableFlags_BordersInnerV = 512;
const _ImGuiTableFlags_BordersOuterV = 1024;
const _ImGuiTableFlags_BordersH = 384;
const _ImGuiTableFlags_BordersV = 1536;
const _ImGuiTableFlags_BordersInner = 640;
const _ImGuiTableFlags_BordersOuter = 1280;
const _ImGuiTableFlags_Borders = 1920;
const _ImGuiTableFlags_NoBordersInBody = 2048;
const _ImGuiTableFlags_NoBordersInBodyUntilResize = 4096;
const _ImGuiTableFlags_SizingFixedFit = 8192;
const _ImGuiTableFlags_SizingFixedSame = 16384;
const _ImGuiTableFlags_SizingStretchProp = 24576;
const _ImGuiTableFlags_SizingStretchSame = 32768;
const _ImGuiTableFlags_NoHostExtendX = 65536;
const _ImGuiTableFlags_NoHostExtendY = 131072;
const _ImGuiTableFlags_NoKeepColumnsVisible = 262144;
const _ImGuiTableFlags_PreciseWidths = 524288;
const _ImGuiTableFlags_NoClip = 1048576;
const _ImGuiTableFlags_PadOuterX = 2097152;
const _ImGuiTableFlags_NoPadOuterX = 4194304;
const _ImGuiTableFlags_NoPadInnerX = 8388608;
const _ImGuiTableFlags_ScrollX = 16777216;
const _ImGuiTableFlags_ScrollY = 33554432;
const _ImGuiTableFlags_SortMulti = 67108864;
const _ImGuiTableFlags_SortTristate = 134217728;
const _ImGuiTableFlags_SizingMask_ = 57344;
const _ImGuiTableColumnFlags_None = 0;
const _ImGuiTableColumnFlags_Disabled = 1;
const _ImGuiTableColumnFlags_DefaultHide = 2;
const _ImGuiTableColumnFlags_DefaultSort = 4;
const _ImGuiTableColumnFlags_WidthStretch = 8;
const _ImGuiTableColumnFlags_WidthFixed = 16;
const _ImGuiTableColumnFlags_NoResize = 32;
const _ImGuiTableColumnFlags_NoReorder = 64;
const _ImGuiTableColumnFlags_NoHide = 128;
const _ImGuiTableColumnFlags_NoClip = 256;
const _ImGuiTableColumnFlags_NoSort = 512;
const _ImGuiTableColumnFlags_NoSortAscending = 1024;
const _ImGuiTableColumnFlags_NoSortDescending = 2048;
const _ImGuiTableColumnFlags_NoHeaderLabel = 4096;
const _ImGuiTableColumnFlags_NoHeaderWidth = 8192;
const _ImGuiTableColumnFlags_PreferSortAscending = 16384;
const _ImGuiTableColumnFlags_PreferSortDescending = 32768;
const _ImGuiTableColumnFlags_IndentEnable = 65536;
const _ImGuiTableColumnFlags_IndentDisable = 131072;
const _ImGuiTableColumnFlags_IsEnabled = 16777216;
const _ImGuiTableColumnFlags_IsVisible = 33554432;
const _ImGuiTableColumnFlags_IsSorted = 67108864;
const _ImGuiTableColumnFlags_IsHovered = 134217728;
const _ImGuiTableColumnFlags_WidthMask_ = 24;
const _ImGuiTableColumnFlags_IndentMask_ = 196608;
const _ImGuiTableColumnFlags_StatusMask_ = 251658240;
const _ImGuiTableColumnFlags_NoDirectResize_ = 1073741824;
const _ImGuiTableRowFlags_None = 0;
const _ImGuiTableRowFlags_Headers = 1;
const _ImGuiTableBgTarget_None = 0;
const _ImGuiTableBgTarget_RowBg0 = 1;
const _ImGuiTableBgTarget_RowBg1 = 2;
const _ImGuiTableBgTarget_CellBg = 3;
const _ImGuiFocusedFlags_None = 0;
const _ImGuiFocusedFlags_ChildWindows = 1;
const _ImGuiFocusedFlags_RootWindow = 2;
const _ImGuiFocusedFlags_AnyWindow = 4;
const _ImGuiFocusedFlags_NoPopupHierarchy = 8;
const _ImGuiFocusedFlags_RootAndChildWindows = 3;
const _ImGuiHoveredFlags_None = 0;
const _ImGuiHoveredFlags_ChildWindows = 1;
const _ImGuiHoveredFlags_RootWindow = 2;
const _ImGuiHoveredFlags_AnyWindow = 4;
const _ImGuiHoveredFlags_NoPopupHierarchy = 8;
const _ImGuiHoveredFlags_AllowWhenBlockedByPopup = 32;
const _ImGuiHoveredFlags_AllowWhenBlockedByActiveItem = 128;
const _ImGuiHoveredFlags_AllowWhenOverlappedByItem = 256;
const _ImGuiHoveredFlags_AllowWhenOverlappedByWindow = 512;
const _ImGuiHoveredFlags_AllowWhenDisabled = 1024;
const _ImGuiHoveredFlags_NoNavOverride = 2048;
const _ImGuiHoveredFlags_AllowWhenOverlapped = 768;
const _ImGuiHoveredFlags_RectOnly = 928;
const _ImGuiHoveredFlags_RootAndChildWindows = 3;
const _ImGuiHoveredFlags_ForTooltip = 4096;
const _ImGuiHoveredFlags_Stationary = 8192;
const _ImGuiHoveredFlags_DelayNone = 16384;
const _ImGuiHoveredFlags_DelayShort = 32768;
const _ImGuiHoveredFlags_DelayNormal = 65536;
const _ImGuiHoveredFlags_NoSharedDelay = 131072;
const _ImGuiDragDropFlags_None = 0;
const _ImGuiDragDropFlags_SourceNoPreviewTooltip = 1;
const _ImGuiDragDropFlags_SourceNoDisableHover = 2;
const _ImGuiDragDropFlags_SourceNoHoldToOpenOthers = 4;
const _ImGuiDragDropFlags_SourceAllowNullID = 8;
const _ImGuiDragDropFlags_SourceExtern = 16;
const _ImGuiDragDropFlags_SourceAutoExpirePayload = 32;
const _ImGuiDragDropFlags_AcceptBeforeDelivery = 1024;
const _ImGuiDragDropFlags_AcceptNoDrawDefaultRect = 2048;
const _ImGuiDragDropFlags_AcceptNoPreviewTooltip = 4096;
const _ImGuiDragDropFlags_AcceptPeekOnly = 3072;
const _ImGuiDataType_S8 = 0;
const _ImGuiDataType_U8 = 1;
const _ImGuiDataType_S16 = 2;
const _ImGuiDataType_U16 = 3;
const _ImGuiDataType_S32 = 4;
const _ImGuiDataType_U32 = 5;
const _ImGuiDataType_S64 = 6;
const _ImGuiDataType_U64 = 7;
const _ImGuiDataType_Float = 8;
const _ImGuiDataType_Double = 9;
const _ImGuiDataType_COUNT = 10;
const _ImGuiDir_None = -1;
const _ImGuiDir_Left = 0;
const _ImGuiDir_Right = 1;
const _ImGuiDir_Up = 2;
const _ImGuiDir_Down = 3;
const _ImGuiDir_COUNT = 4;
const _ImGuiSortDirection_None = 0;
const _ImGuiSortDirection_Ascending = 1;
const _ImGuiSortDirection_Descending = 2;
const _ImGuiKey_None = 0;
const _ImGuiKey_Tab = 512;
const _ImGuiKey_LeftArrow = 513;
const _ImGuiKey_RightArrow = 514;
const _ImGuiKey_UpArrow = 515;
const _ImGuiKey_DownArrow = 516;
const _ImGuiKey_PageUp = 517;
const _ImGuiKey_PageDown = 518;
const _ImGuiKey_Home = 519;
const _ImGuiKey_End = 520;
const _ImGuiKey_Insert = 521;
const _ImGuiKey_Delete = 522;
const _ImGuiKey_Backspace = 523;
const _ImGuiKey_Space = 524;
const _ImGuiKey_Enter = 525;
const _ImGuiKey_Escape = 526;
const _ImGuiKey_LeftCtrl = 527;
const _ImGuiKey_LeftShift = 528;
const _ImGuiKey_LeftAlt = 529;
const _ImGuiKey_LeftSuper = 530;
const _ImGuiKey_RightCtrl = 531;
const _ImGuiKey_RightShift = 532;
const _ImGuiKey_RightAlt = 533;
const _ImGuiKey_RightSuper = 534;
const _ImGuiKey_Menu = 535;
const _ImGuiKey_0 = 536;
const _ImGuiKey_1 = 537;
const _ImGuiKey_2 = 538;
const _ImGuiKey_3 = 539;
const _ImGuiKey_4 = 540;
const _ImGuiKey_5 = 541;
const _ImGuiKey_6 = 542;
const _ImGuiKey_7 = 543;
const _ImGuiKey_8 = 544;
const _ImGuiKey_9 = 545;
const _ImGuiKey_A = 546;
const _ImGuiKey_B = 547;
const _ImGuiKey_C = 548;
const _ImGuiKey_D = 549;
const _ImGuiKey_E = 550;
const _ImGuiKey_F = 551;
const _ImGuiKey_G = 552;
const _ImGuiKey_H = 553;
const _ImGuiKey_I = 554;
const _ImGuiKey_J = 555;
const _ImGuiKey_K = 556;
const _ImGuiKey_L = 557;
const _ImGuiKey_M = 558;
const _ImGuiKey_N = 559;
const _ImGuiKey_O = 560;
const _ImGuiKey_P = 561;
const _ImGuiKey_Q = 562;
const _ImGuiKey_R = 563;
const _ImGuiKey_S = 564;
const _ImGuiKey_T = 565;
const _ImGuiKey_U = 566;
const _ImGuiKey_V = 567;
const _ImGuiKey_W = 568;
const _ImGuiKey_X = 569;
const _ImGuiKey_Y = 570;
const _ImGuiKey_Z = 571;
const _ImGuiKey_F1 = 572;
const _ImGuiKey_F2 = 573;
const _ImGuiKey_F3 = 574;
const _ImGuiKey_F4 = 575;
const _ImGuiKey_F5 = 576;
const _ImGuiKey_F6 = 577;
const _ImGuiKey_F7 = 578;
const _ImGuiKey_F8 = 579;
const _ImGuiKey_F9 = 580;
const _ImGuiKey_F10 = 581;
const _ImGuiKey_F11 = 582;
const _ImGuiKey_F12 = 583;
const _ImGuiKey_Apostrophe = 584;
const _ImGuiKey_Comma = 585;
const _ImGuiKey_Minus = 586;
const _ImGuiKey_Period = 587;
const _ImGuiKey_Slash = 588;
const _ImGuiKey_Semicolon = 589;
const _ImGuiKey_Equal = 590;
const _ImGuiKey_LeftBracket = 591;
const _ImGuiKey_Backslash = 592;
const _ImGuiKey_RightBracket = 593;
const _ImGuiKey_GraveAccent = 594;
const _ImGuiKey_CapsLock = 595;
const _ImGuiKey_ScrollLock = 596;
const _ImGuiKey_NumLock = 597;
const _ImGuiKey_PrintScreen = 598;
const _ImGuiKey_Pause = 599;
const _ImGuiKey_Keypad0 = 600;
const _ImGuiKey_Keypad1 = 601;
const _ImGuiKey_Keypad2 = 602;
const _ImGuiKey_Keypad3 = 603;
const _ImGuiKey_Keypad4 = 604;
const _ImGuiKey_Keypad5 = 605;
const _ImGuiKey_Keypad6 = 606;
const _ImGuiKey_Keypad7 = 607;
const _ImGuiKey_Keypad8 = 608;
const _ImGuiKey_Keypad9 = 609;
const _ImGuiKey_KeypadDecimal = 610;
const _ImGuiKey_KeypadDivide = 611;
const _ImGuiKey_KeypadMultiply = 612;
const _ImGuiKey_KeypadSubtract = 613;
const _ImGuiKey_KeypadAdd = 614;
const _ImGuiKey_KeypadEnter = 615;
const _ImGuiKey_KeypadEqual = 616;
const _ImGuiKey_GamepadStart = 617;
const _ImGuiKey_GamepadBack = 618;
const _ImGuiKey_GamepadFaceLeft = 619;
const _ImGuiKey_GamepadFaceRight = 620;
const _ImGuiKey_GamepadFaceUp = 621;
const _ImGuiKey_GamepadFaceDown = 622;
const _ImGuiKey_GamepadDpadLeft = 623;
const _ImGuiKey_GamepadDpadRight = 624;
const _ImGuiKey_GamepadDpadUp = 625;
const _ImGuiKey_GamepadDpadDown = 626;
const _ImGuiKey_GamepadL1 = 627;
const _ImGuiKey_GamepadR1 = 628;
const _ImGuiKey_GamepadL2 = 629;
const _ImGuiKey_GamepadR2 = 630;
const _ImGuiKey_GamepadL3 = 631;
const _ImGuiKey_GamepadR3 = 632;
const _ImGuiKey_GamepadLStickLeft = 633;
const _ImGuiKey_GamepadLStickRight = 634;
const _ImGuiKey_GamepadLStickUp = 635;
const _ImGuiKey_GamepadLStickDown = 636;
const _ImGuiKey_GamepadRStickLeft = 637;
const _ImGuiKey_GamepadRStickRight = 638;
const _ImGuiKey_GamepadRStickUp = 639;
const _ImGuiKey_GamepadRStickDown = 640;
const _ImGuiKey_MouseLeft = 641;
const _ImGuiKey_MouseRight = 642;
const _ImGuiKey_MouseMiddle = 643;
const _ImGuiKey_MouseX1 = 644;
const _ImGuiKey_MouseX2 = 645;
const _ImGuiKey_MouseWheelX = 646;
const _ImGuiKey_MouseWheelY = 647;
const _ImGuiKey_ReservedForModCtrl = 648;
const _ImGuiKey_ReservedForModShift = 649;
const _ImGuiKey_ReservedForModAlt = 650;
const _ImGuiKey_ReservedForModSuper = 651;
const _ImGuiKey_COUNT = 652;
const _ImGuiMod_None = 0;
const _ImGuiMod_Ctrl = 4096;
const _ImGuiMod_Shift = 8192;
const _ImGuiMod_Alt = 16384;
const _ImGuiMod_Super = 32768;
const _ImGuiMod_Shortcut = 2048;
const _ImGuiMod_Mask_ = 63488;
const _ImGuiKey_NamedKey_BEGIN = 512;
const _ImGuiKey_NamedKey_END = 652;
const _ImGuiKey_NamedKey_COUNT = 140;
const _ImGuiKey_KeysData_SIZE = 652;
const _ImGuiKey_KeysData_OFFSET = 0;
const _ImGuiNavInput_Activate = 0;
const _ImGuiNavInput_Cancel = 1;
const _ImGuiNavInput_Input = 2;
const _ImGuiNavInput_Menu = 3;
const _ImGuiNavInput_DpadLeft = 4;
const _ImGuiNavInput_DpadRight = 5;
const _ImGuiNavInput_DpadUp = 6;
const _ImGuiNavInput_DpadDown = 7;
const _ImGuiNavInput_LStickLeft = 8;
const _ImGuiNavInput_LStickRight = 9;
const _ImGuiNavInput_LStickUp = 10;
const _ImGuiNavInput_LStickDown = 11;
const _ImGuiNavInput_FocusPrev = 12;
const _ImGuiNavInput_FocusNext = 13;
const _ImGuiNavInput_TweakSlow = 14;
const _ImGuiNavInput_TweakFast = 15;
const _ImGuiNavInput_COUNT = 16;
const _ImGuiConfigFlags_None = 0;
const _ImGuiConfigFlags_NavEnableKeyboard = 1;
const _ImGuiConfigFlags_NavEnableGamepad = 2;
const _ImGuiConfigFlags_NavEnableSetMousePos = 4;
const _ImGuiConfigFlags_NavNoCaptureKeyboard = 8;
const _ImGuiConfigFlags_NoMouse = 16;
const _ImGuiConfigFlags_NoMouseCursorChange = 32;
const _ImGuiConfigFlags_IsSRGB = 1048576;
const _ImGuiConfigFlags_IsTouchScreen = 2097152;
const _ImGuiBackendFlags_None = 0;
const _ImGuiBackendFlags_HasGamepad = 1;
const _ImGuiBackendFlags_HasMouseCursors = 2;
const _ImGuiBackendFlags_HasSetMousePos = 4;
const _ImGuiBackendFlags_RendererHasVtxOffset = 8;
const _ImGuiCol_Text = 0;
const _ImGuiCol_TextDisabled = 1;
const _ImGuiCol_WindowBg = 2;
const _ImGuiCol_ChildBg = 3;
const _ImGuiCol_PopupBg = 4;
const _ImGuiCol_Border = 5;
const _ImGuiCol_BorderShadow = 6;
const _ImGuiCol_FrameBg = 7;
const _ImGuiCol_FrameBgHovered = 8;
const _ImGuiCol_FrameBgActive = 9;
const _ImGuiCol_TitleBg = 10;
const _ImGuiCol_TitleBgActive = 11;
const _ImGuiCol_TitleBgCollapsed = 12;
const _ImGuiCol_MenuBarBg = 13;
const _ImGuiCol_ScrollbarBg = 14;
const _ImGuiCol_ScrollbarGrab = 15;
const _ImGuiCol_ScrollbarGrabHovered = 16;
const _ImGuiCol_ScrollbarGrabActive = 17;
const _ImGuiCol_CheckMark = 18;
const _ImGuiCol_SliderGrab = 19;
const _ImGuiCol_SliderGrabActive = 20;
const _ImGuiCol_Button = 21;
const _ImGuiCol_ButtonHovered = 22;
const _ImGuiCol_ButtonActive = 23;
const _ImGuiCol_Header = 24;
const _ImGuiCol_HeaderHovered = 25;
const _ImGuiCol_HeaderActive = 26;
const _ImGuiCol_Separator = 27;
const _ImGuiCol_SeparatorHovered = 28;
const _ImGuiCol_SeparatorActive = 29;
const _ImGuiCol_ResizeGrip = 30;
const _ImGuiCol_ResizeGripHovered = 31;
const _ImGuiCol_ResizeGripActive = 32;
const _ImGuiCol_Tab = 33;
const _ImGuiCol_TabHovered = 34;
const _ImGuiCol_TabActive = 35;
const _ImGuiCol_TabUnfocused = 36;
const _ImGuiCol_TabUnfocusedActive = 37;
const _ImGuiCol_PlotLines = 38;
const _ImGuiCol_PlotLinesHovered = 39;
const _ImGuiCol_PlotHistogram = 40;
const _ImGuiCol_PlotHistogramHovered = 41;
const _ImGuiCol_TableHeaderBg = 42;
const _ImGuiCol_TableBorderStrong = 43;
const _ImGuiCol_TableBorderLight = 44;
const _ImGuiCol_TableRowBg = 45;
const _ImGuiCol_TableRowBgAlt = 46;
const _ImGuiCol_TextSelectedBg = 47;
const _ImGuiCol_DragDropTarget = 48;
const _ImGuiCol_NavHighlight = 49;
const _ImGuiCol_NavWindowingHighlight = 50;
const _ImGuiCol_NavWindowingDimBg = 51;
const _ImGuiCol_ModalWindowDimBg = 52;
const _ImGuiCol_COUNT = 53;
const _ImGuiStyleVar_Alpha = 0;
const _ImGuiStyleVar_DisabledAlpha = 1;
const _ImGuiStyleVar_WindowPadding = 2;
const _ImGuiStyleVar_WindowRounding = 3;
const _ImGuiStyleVar_WindowBorderSize = 4;
const _ImGuiStyleVar_WindowMinSize = 5;
const _ImGuiStyleVar_WindowTitleAlign = 6;
const _ImGuiStyleVar_ChildRounding = 7;
const _ImGuiStyleVar_ChildBorderSize = 8;
const _ImGuiStyleVar_PopupRounding = 9;
const _ImGuiStyleVar_PopupBorderSize = 10;
const _ImGuiStyleVar_FramePadding = 11;
const _ImGuiStyleVar_FrameRounding = 12;
const _ImGuiStyleVar_FrameBorderSize = 13;
const _ImGuiStyleVar_ItemSpacing = 14;
const _ImGuiStyleVar_ItemInnerSpacing = 15;
const _ImGuiStyleVar_IndentSpacing = 16;
const _ImGuiStyleVar_CellPadding = 17;
const _ImGuiStyleVar_ScrollbarSize = 18;
const _ImGuiStyleVar_ScrollbarRounding = 19;
const _ImGuiStyleVar_GrabMinSize = 20;
const _ImGuiStyleVar_GrabRounding = 21;
const _ImGuiStyleVar_TabRounding = 22;
const _ImGuiStyleVar_ButtonTextAlign = 23;
const _ImGuiStyleVar_SelectableTextAlign = 24;
const _ImGuiStyleVar_SeparatorTextBorderSize = 25;
const _ImGuiStyleVar_SeparatorTextAlign = 26;
const _ImGuiStyleVar_SeparatorTextPadding = 27;
const _ImGuiStyleVar_COUNT = 28;
const _ImGuiButtonFlags_None = 0;
const _ImGuiButtonFlags_MouseButtonLeft = 1;
const _ImGuiButtonFlags_MouseButtonRight = 2;
const _ImGuiButtonFlags_MouseButtonMiddle = 4;
const _ImGuiButtonFlags_MouseButtonMask_ = 7;
const _ImGuiButtonFlags_MouseButtonDefault_ = 1;
const _ImGuiColorEditFlags_None = 0;
const _ImGuiColorEditFlags_NoAlpha = 2;
const _ImGuiColorEditFlags_NoPicker = 4;
const _ImGuiColorEditFlags_NoOptions = 8;
const _ImGuiColorEditFlags_NoSmallPreview = 16;
const _ImGuiColorEditFlags_NoInputs = 32;
const _ImGuiColorEditFlags_NoTooltip = 64;
const _ImGuiColorEditFlags_NoLabel = 128;
const _ImGuiColorEditFlags_NoSidePreview = 256;
const _ImGuiColorEditFlags_NoDragDrop = 512;
const _ImGuiColorEditFlags_NoBorder = 1024;
const _ImGuiColorEditFlags_AlphaBar = 65536;
const _ImGuiColorEditFlags_AlphaPreview = 131072;
const _ImGuiColorEditFlags_AlphaPreviewHalf = 262144;
const _ImGuiColorEditFlags_HDR = 524288;
const _ImGuiColorEditFlags_DisplayRGB = 1048576;
const _ImGuiColorEditFlags_DisplayHSV = 2097152;
const _ImGuiColorEditFlags_DisplayHex = 4194304;
const _ImGuiColorEditFlags_Uint8 = 8388608;
const _ImGuiColorEditFlags_Float = 16777216;
const _ImGuiColorEditFlags_PickerHueBar = 33554432;
const _ImGuiColorEditFlags_PickerHueWheel = 67108864;
const _ImGuiColorEditFlags_InputRGB = 134217728;
const _ImGuiColorEditFlags_InputHSV = 268435456;
const _ImGuiColorEditFlags_DefaultOptions_ = 177209344;
const _ImGuiColorEditFlags_DisplayMask_ = 7340032;
const _ImGuiColorEditFlags_DataTypeMask_ = 25165824;
const _ImGuiColorEditFlags_PickerMask_ = 100663296;
const _ImGuiColorEditFlags_InputMask_ = 402653184;
const _ImGuiSliderFlags_None = 0;
const _ImGuiSliderFlags_AlwaysClamp = 16;
const _ImGuiSliderFlags_Logarithmic = 32;
const _ImGuiSliderFlags_NoRoundToFormat = 64;
const _ImGuiSliderFlags_NoInput = 128;
const _ImGuiSliderFlags_InvalidMask_ = 1879048207;
const _ImGuiMouseButton_Left = 0;
const _ImGuiMouseButton_Right = 1;
const _ImGuiMouseButton_Middle = 2;
const _ImGuiMouseButton_COUNT = 5;
const _ImGuiMouseCursor_None = -1;
const _ImGuiMouseCursor_Arrow = 0;
const _ImGuiMouseCursor_TextInput = 1;
const _ImGuiMouseCursor_ResizeAll = 2;
const _ImGuiMouseCursor_ResizeNS = 3;
const _ImGuiMouseCursor_ResizeEW = 4;
const _ImGuiMouseCursor_ResizeNESW = 5;
const _ImGuiMouseCursor_ResizeNWSE = 6;
const _ImGuiMouseCursor_Hand = 7;
const _ImGuiMouseCursor_NotAllowed = 8;
const _ImGuiMouseCursor_COUNT = 9;
const _ImGuiMouseSource_Mouse = 0;
const _ImGuiMouseSource_TouchScreen = 1;
const _ImGuiMouseSource_Pen = 2;
const _ImGuiMouseSource_COUNT = 3;
const _ImGuiCond_None = 0;
const _ImGuiCond_Always = 1;
const _ImGuiCond_Once = 2;
const _ImGuiCond_FirstUseEver = 4;
const _ImGuiCond_Appearing = 8;
const _ImDrawFlags_None = 0;
const _ImDrawFlags_Closed = 1;
const _ImDrawFlags_RoundCornersTopLeft = 16;
const _ImDrawFlags_RoundCornersTopRight = 32;
const _ImDrawFlags_RoundCornersBottomLeft = 64;
const _ImDrawFlags_RoundCornersBottomRight = 128;
const _ImDrawFlags_RoundCornersNone = 256;
const _ImDrawFlags_RoundCornersTop = 48;
const _ImDrawFlags_RoundCornersBottom = 192;
const _ImDrawFlags_RoundCornersLeft = 80;
const _ImDrawFlags_RoundCornersRight = 160;
const _ImDrawFlags_RoundCornersAll = 240;
const _ImDrawFlags_RoundCornersDefault_ = 240;
const _ImDrawFlags_RoundCornersMask_ = 496;
const _ImDrawListFlags_None = 0;
const _ImDrawListFlags_AntiAliasedLines = 1;
const _ImDrawListFlags_AntiAliasedLinesUseTex = 2;
const _ImDrawListFlags_AntiAliasedFill = 4;
const _ImDrawListFlags_AllowVtxOffset = 8;
const _ImFontAtlasFlags_None = 0;
const _ImFontAtlasFlags_NoPowerOfTwoHeight = 1;
const _ImFontAtlasFlags_NoMouseCursors = 2;
const _ImFontAtlasFlags_NoBakedLines = 4;
const _ImGuiViewportFlags_None = 0;
const _ImGuiViewportFlags_IsPlatformWindow = 1;
const _ImGuiViewportFlags_IsPlatformMonitor = 2;
const _ImGuiViewportFlags_OwnedByApp = 4;
const _ImGuiItemFlags_None = 0;
const _ImGuiItemFlags_NoTabStop = 1;
const _ImGuiItemFlags_ButtonRepeat = 2;
const _ImGuiItemFlags_Disabled = 4;
const _ImGuiItemFlags_NoNav = 8;
const _ImGuiItemFlags_NoNavDefaultFocus = 16;
const _ImGuiItemFlags_SelectableDontClosePopup = 32;
const _ImGuiItemFlags_MixedValue = 64;
const _ImGuiItemFlags_ReadOnly = 128;
const _ImGuiItemFlags_NoWindowHoverableCheck = 256;
const _ImGuiItemFlags_AllowOverlap = 512;
const _ImGuiItemFlags_Inputable = 1024;
const _ImGuiItemStatusFlags_None = 0;
const _ImGuiItemStatusFlags_HoveredRect = 1;
const _ImGuiItemStatusFlags_HasDisplayRect = 2;
const _ImGuiItemStatusFlags_Edited = 4;
const _ImGuiItemStatusFlags_ToggledSelection = 8;
const _ImGuiItemStatusFlags_ToggledOpen = 16;
const _ImGuiItemStatusFlags_HasDeactivated = 32;
const _ImGuiItemStatusFlags_Deactivated = 64;
const _ImGuiItemStatusFlags_HoveredWindow = 128;
const _ImGuiItemStatusFlags_FocusedByTabbing = 256;
const _ImGuiItemStatusFlags_Visible = 512;
const _ImGuiHoveredFlags_DelayMask_ = 245760;
const _ImGuiHoveredFlags_AllowedMaskForIsWindowHovered = 12463;
const _ImGuiHoveredFlags_AllowedMaskForIsItemHovered = 262048;
const _ImGuiInputTextFlags_Multiline = 67108864;
const _ImGuiInputTextFlags_NoMarkEdited = 134217728;
const _ImGuiInputTextFlags_MergedItem = 268435456;
const _ImGuiButtonFlags_PressedOnClick = 16;
const _ImGuiButtonFlags_PressedOnClickRelease = 32;
const _ImGuiButtonFlags_PressedOnClickReleaseAnywhere = 64;
const _ImGuiButtonFlags_PressedOnRelease = 128;
const _ImGuiButtonFlags_PressedOnDoubleClick = 256;
const _ImGuiButtonFlags_PressedOnDragDropHold = 512;
const _ImGuiButtonFlags_Repeat = 1024;
const _ImGuiButtonFlags_FlattenChildren = 2048;
const _ImGuiButtonFlags_AllowOverlap = 4096;
const _ImGuiButtonFlags_DontClosePopups = 8192;
const _ImGuiButtonFlags_AlignTextBaseLine = 32768;
const _ImGuiButtonFlags_NoKeyModifiers = 65536;
const _ImGuiButtonFlags_NoHoldingActiveId = 131072;
const _ImGuiButtonFlags_NoNavFocus = 262144;
const _ImGuiButtonFlags_NoHoveredOnFocus = 524288;
const _ImGuiButtonFlags_NoSetKeyOwner = 1048576;
const _ImGuiButtonFlags_NoTestKeyOwner = 2097152;
const _ImGuiButtonFlags_PressedOnMask_ = 1008;
const _ImGuiButtonFlags_PressedOnDefault_ = 32;
const _ImGuiComboFlags_CustomPreview = 1048576;
const _ImGuiSliderFlags_Vertical = 1048576;
const _ImGuiSliderFlags_ReadOnly = 2097152;
const _ImGuiSelectableFlags_NoHoldingActiveID = 1048576;
const _ImGuiSelectableFlags_SelectOnNav = 2097152;
const _ImGuiSelectableFlags_SelectOnClick = 4194304;
const _ImGuiSelectableFlags_SelectOnRelease = 8388608;
const _ImGuiSelectableFlags_SpanAvailWidth = 16777216;
const _ImGuiSelectableFlags_SetNavIdOnHover = 33554432;
const _ImGuiSelectableFlags_NoPadWithHalfSpacing = 67108864;
const _ImGuiSelectableFlags_NoSetKeyOwner = 134217728;
const _ImGuiTreeNodeFlags_ClipLabelForTrailingButton = 1048576;
const _ImGuiTreeNodeFlags_UpsideDownArrow = 2097152;
const _ImGuiSeparatorFlags_None = 0;
const _ImGuiSeparatorFlags_Horizontal = 1;
const _ImGuiSeparatorFlags_Vertical = 2;
const _ImGuiSeparatorFlags_SpanAllColumns = 4;
const _ImGuiFocusRequestFlags_None = 0;
const _ImGuiFocusRequestFlags_RestoreFocusedChild = 1;
const _ImGuiFocusRequestFlags_UnlessBelowModal = 2;
const _ImGuiTextFlags_None = 0;
const _ImGuiTextFlags_NoWidthForLargeClippedText = 1;
const _ImGuiTooltipFlags_None = 0;
const _ImGuiTooltipFlags_OverridePrevious = 2;
const _ImGuiLayoutType_Horizontal = 0;
const _ImGuiLayoutType_Vertical = 1;
const _ImGuiLogType_None = 0;
const _ImGuiLogType_TTY = 1;
const _ImGuiLogType_File = 2;
const _ImGuiLogType_Buffer = 3;
const _ImGuiLogType_Clipboard = 4;
const _ImGuiAxis_None = -1;
const _ImGuiAxis_X = 0;
const _ImGuiAxis_Y = 1;
const _ImGuiPlotType_Lines = 0;
const _ImGuiPlotType_Histogram = 1;
const _ImGuiPopupPositionPolicy_Default = 0;
const _ImGuiPopupPositionPolicy_ComboBox = 1;
const _ImGuiPopupPositionPolicy_Tooltip = 2;
const _ImGuiDataType_String = 11;
const _ImGuiDataType_Pointer = 12;
const _ImGuiDataType_ID = 13;
const _ImGuiNextWindowDataFlags_None = 0;
const _ImGuiNextWindowDataFlags_HasPos = 1;
const _ImGuiNextWindowDataFlags_HasSize = 2;
const _ImGuiNextWindowDataFlags_HasContentSize = 4;
const _ImGuiNextWindowDataFlags_HasCollapsed = 8;
const _ImGuiNextWindowDataFlags_HasSizeConstraint = 16;
const _ImGuiNextWindowDataFlags_HasFocus = 32;
const _ImGuiNextWindowDataFlags_HasBgAlpha = 64;
const _ImGuiNextWindowDataFlags_HasScroll = 128;
const _ImGuiNextItemDataFlags_None = 0;
const _ImGuiNextItemDataFlags_HasWidth = 1;
const _ImGuiNextItemDataFlags_HasOpen = 2;
const _ImGuiInputEventType_None = 0;
const _ImGuiInputEventType_MousePos = 1;
const _ImGuiInputEventType_MouseWheel = 2;
const _ImGuiInputEventType_MouseButton = 3;
const _ImGuiInputEventType_Key = 4;
const _ImGuiInputEventType_Text = 5;
const _ImGuiInputEventType_Focus = 6;
const _ImGuiInputEventType_COUNT = 7;
const _ImGuiInputSource_None = 0;
const _ImGuiInputSource_Mouse = 1;
const _ImGuiInputSource_Keyboard = 2;
const _ImGuiInputSource_Gamepad = 3;
const _ImGuiInputSource_Clipboard = 4;
const _ImGuiInputSource_COUNT = 5;
const _ImGuiInputFlags_None = 0;
const _ImGuiInputFlags_Repeat = 1;
const _ImGuiInputFlags_RepeatRateDefault = 2;
const _ImGuiInputFlags_RepeatRateNavMove = 4;
const _ImGuiInputFlags_RepeatRateNavTweak = 8;
const _ImGuiInputFlags_RepeatRateMask_ = 14;
const _ImGuiInputFlags_CondHovered = 16;
const _ImGuiInputFlags_CondActive = 32;
const _ImGuiInputFlags_CondDefault_ = 48;
const _ImGuiInputFlags_CondMask_ = 48;
const _ImGuiInputFlags_LockThisFrame = 64;
const _ImGuiInputFlags_LockUntilRelease = 128;
const _ImGuiInputFlags_RouteFocused = 256;
const _ImGuiInputFlags_RouteGlobalLow = 512;
const _ImGuiInputFlags_RouteGlobal = 1024;
const _ImGuiInputFlags_RouteGlobalHigh = 2048;
const _ImGuiInputFlags_RouteMask_ = 3840;
const _ImGuiInputFlags_RouteAlways = 4096;
const _ImGuiInputFlags_RouteUnlessBgFocused = 8192;
const _ImGuiInputFlags_RouteExtraMask_ = 12288;
const _ImGuiInputFlags_SupportedByIsKeyPressed = 15;
const _ImGuiInputFlags_SupportedByShortcut = 16143;
const _ImGuiInputFlags_SupportedBySetKeyOwner = 192;
const _ImGuiInputFlags_SupportedBySetItemKeyOwner = 240;
const _ImGuiActivateFlags_None = 0;
const _ImGuiActivateFlags_PreferInput = 1;
const _ImGuiActivateFlags_PreferTweak = 2;
const _ImGuiActivateFlags_TryToPreserveState = 4;
const _ImGuiScrollFlags_None = 0;
const _ImGuiScrollFlags_KeepVisibleEdgeX = 1;
const _ImGuiScrollFlags_KeepVisibleEdgeY = 2;
const _ImGuiScrollFlags_KeepVisibleCenterX = 4;
const _ImGuiScrollFlags_KeepVisibleCenterY = 8;
const _ImGuiScrollFlags_AlwaysCenterX = 16;
const _ImGuiScrollFlags_AlwaysCenterY = 32;
const _ImGuiScrollFlags_NoScrollParent = 64;
const _ImGuiScrollFlags_MaskX_ = 21;
const _ImGuiScrollFlags_MaskY_ = 42;
const _ImGuiNavHighlightFlags_None = 0;
const _ImGuiNavHighlightFlags_TypeDefault = 1;
const _ImGuiNavHighlightFlags_TypeThin = 2;
const _ImGuiNavHighlightFlags_AlwaysDraw = 4;
const _ImGuiNavHighlightFlags_NoRounding = 8;
const _ImGuiNavMoveFlags_None = 0;
const _ImGuiNavMoveFlags_LoopX = 1;
const _ImGuiNavMoveFlags_LoopY = 2;
const _ImGuiNavMoveFlags_WrapX = 4;
const _ImGuiNavMoveFlags_WrapY = 8;
const _ImGuiNavMoveFlags_WrapMask_ = 15;
const _ImGuiNavMoveFlags_AllowCurrentNavId = 16;
const _ImGuiNavMoveFlags_AlsoScoreVisibleSet = 32;
const _ImGuiNavMoveFlags_ScrollToEdgeY = 64;
const _ImGuiNavMoveFlags_Forwarded = 128;
const _ImGuiNavMoveFlags_DebugNoResult = 256;
const _ImGuiNavMoveFlags_FocusApi = 512;
const _ImGuiNavMoveFlags_IsTabbing = 1024;
const _ImGuiNavMoveFlags_IsPageMove = 2048;
const _ImGuiNavMoveFlags_Activate = 4096;
const _ImGuiNavMoveFlags_NoSelect = 8192;
const _ImGuiNavMoveFlags_NoSetNavHighlight = 16384;
const _ImGuiNavLayer_Main = 0;
const _ImGuiNavLayer_Menu = 1;
const _ImGuiNavLayer_COUNT = 2;
const _ImGuiOldColumnFlags_None = 0;
const _ImGuiOldColumnFlags_NoBorder = 1;
const _ImGuiOldColumnFlags_NoResize = 2;
const _ImGuiOldColumnFlags_NoPreserveWidths = 4;
const _ImGuiOldColumnFlags_NoForceWithinWindow = 8;
const _ImGuiOldColumnFlags_GrowParentContentsSize = 16;
const _ImGuiLocKey_VersionStr = 0;
const _ImGuiLocKey_TableSizeOne = 1;
const _ImGuiLocKey_TableSizeAllFit = 2;
const _ImGuiLocKey_TableSizeAllDefault = 3;
const _ImGuiLocKey_TableResetOrder = 4;
const _ImGuiLocKey_WindowingMainMenuBar = 5;
const _ImGuiLocKey_WindowingPopup = 6;
const _ImGuiLocKey_WindowingUntitled = 7;
const _ImGuiLocKey_COUNT = 8;
const _ImGuiDebugLogFlags_None = 0;
const _ImGuiDebugLogFlags_EventActiveId = 1;
const _ImGuiDebugLogFlags_EventFocus = 2;
const _ImGuiDebugLogFlags_EventPopup = 4;
const _ImGuiDebugLogFlags_EventNav = 8;
const _ImGuiDebugLogFlags_EventClipper = 16;
const _ImGuiDebugLogFlags_EventSelection = 32;
const _ImGuiDebugLogFlags_EventIO = 64;
const _ImGuiDebugLogFlags_EventMask_ = 127;
const _ImGuiDebugLogFlags_OutputToTTY = 1024;
const _ImGuiContextHookType_NewFramePre = 0;
const _ImGuiContextHookType_NewFramePost = 1;
const _ImGuiContextHookType_EndFramePre = 2;
const _ImGuiContextHookType_EndFramePost = 3;
const _ImGuiContextHookType_RenderPre = 4;
const _ImGuiContextHookType_RenderPost = 5;
const _ImGuiContextHookType_Shutdown = 6;
const _ImGuiContextHookType_PendingRemoval_ = 7;
const _ImGuiTabBarFlags_DockNode = 1048576;
const _ImGuiTabBarFlags_IsFocused = 2097152;
const _ImGuiTabBarFlags_SaveSettings = 4194304;
const _ImGuiTabItemFlags_SectionMask_ = 192;
const _ImGuiTabItemFlags_NoCloseButton = 1048576;
const _ImGuiTabItemFlags_Button = 2097152;
const _SIMGUI_INVALID_ID = 0;
const _SIMGUI_LOGITEM_OK = 0;
const _SIMGUI_LOGITEM_MALLOC_FAILED = 1;
const _SIMGUI_LOGITEM_IMAGE_POOL_EXHAUSTED = 2;
