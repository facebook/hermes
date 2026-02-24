"use strict";
(function (){
let a = {};
let b = {};
let c = {};

function s(n, key, value) {
  n[key] = value;
}

function setPropSwitch(n, key, value) {
  switch (key) {
    case "accentHeight": s(n,"accent-height",value); break;
    case "alignmentBaseline": s(n,"alignment-baseline",value); break;
    case "arabicForm": s(n,"arabic-form",value); break;
    case "baselineShift": s(n,"baseline-shift",value); break;
    case "capHeight": s(n,"cap-height",value); break;
    case "clipPath": s(n,"clip-path",value); break;
    case "clipRule": s(n,"clip-rule",value); break;
    case "colorInterpolation": s(n,"color-interpolation",value); break;
    case "colorInterpolationFilters": s(n,"color-interpolation-filters",value); break;
    case "colorProfile": s(n,"color-profile",value); break;
    case "colorRendering": s(n,"color-rendering",value); break;
    case "dominantBaseline": s(n,"dominant-baseline",value); break;
    case "enableBackground": s(n,"enable-background",value); break;
    case "fillOpacity": s(n,"fill-opacity",value); break;
    case "fillRule": s(n,"fill-rule",value); break;
    case "floodColor": s(n,"flood-color",value); break;
    case "floodOpacity": s(n,"flood-opacity",value); break;
    case "fontFamily": s(n,"font-family",value); break;
    case "fontSize": s(n,"font-size",value); break;
    case "fontSizeAdjust": s(n,"font-size-adjust",value); break;
    case "fontStretch": s(n,"font-stretch",value); break;
    case "fontStyle": s(n,"font-style",value); break;
    case "fontVariant": s(n,"font-variant",value); break;
    case "fontWeight": s(n,"font-weight",value); break;
    case "glyphName": s(n,"glyph-name",value); break;
    case "glyphOrientationHorizontal": s(n,"glyph-orientation-horizontal",value); break;
    case "glyphOrientationVertical": s(n,"glyph-orientation-vertical",value); break;
    case "horizAdvX": s(n,"horiz-adv-x",value); break;
    case "horizOriginX": s(n,"horiz-origin-x",value); break;
    case "imageRendering": s(n,"image-rendering",value); break;
    case "letterSpacing": s(n,"letter-spacing",value); break;
    case "lightingColor": s(n,"lighting-color",value); break;
    case "markerEnd": s(n,"marker-end",value); break;
    case "markerMid": s(n,"marker-mid",value); break;
    case "markerStart": s(n,"marker-start",value); break;
    case "overlinePosition": s(n,"overline-position",value); break;
    case "overlineThickness": s(n,"overline-thickness",value); break;
    case "paintOrder": s(n,"paint-order",value); break;
    case "panose-1": s(n,"panose-1",value); break;
    case "pointerEvents": s(n,"pointer-events",value); break;
    case "renderingIntent": s(n,"rendering-intent",value); break;
    case "shapeRendering": s(n,"shape-rendering",value); break;
    case "stopColor": s(n,"stop-color",value); break;
    case "stopOpacity": s(n,"stop-opacity",value); break;
    case "strikethroughPosition": s(n,"strikethrough-position",value); break;
    case "strikethroughThickness": s(n,"strikethrough-thickness",value); break;
    case "strokeDasharray": s(n,"stroke-dasharray",value); break;
    case "strokeDashoffset": s(n,"stroke-dashoffset",value); break;
    case "strokeLinecap": s(n,"stroke-linecap",value); break;
    case "strokeLinejoin": s(n,"stroke-linejoin",value); break;
    case "strokeMiterlimit": s(n,"stroke-miterlimit",value); break;
    case "strokeOpacity": s(n,"stroke-opacity",value); break;
    case "strokeWidth": s(n,"stroke-width",value); break;
    case "textAnchor": s(n,"text-anchor",value); break;
    case "textDecoration": s(n,"text-decoration",value); break;
    case "textRendering": s(n,"text-rendering",value); break;
    case "transformOrigin": s(n,"transform-origin",value); break;
    case "underlinePosition": s(n,"underline-position",value); break;
    case "underlineThickness": s(n,"underline-thickness",value); break;
    case "unicodeBidi": s(n,"unicode-bidi",value); break;
    case "unicodeRange": s(n,"unicode-range",value); break;
    case "unitsPerEm": s(n,"units-per-em",value); break;
    case "vAlphabetic": s(n,"v-alphabetic",value); break;
    case "vHanging": s(n,"v-hanging",value); break;
    case "vIdeographic": s(n,"v-ideographic",value); break;
    case "vMathematical": s(n,"v-mathematical",value); break;
    case "vectorEffect": s(n,"vector-effect",value); break;
    case "vertAdvY": s(n,"vert-adv-y",value); break;
    case "vertOriginX": s(n,"vert-origin-x",value); break;
    case "vertOriginY": s(n,"vert-origin-y",value); break;
    case "wordSpacing": s(n,"word-spacing",value); break;
    case "writingMode": s(n,"writing-mode",value); break;
    case "xmlnsXlink": s(n,"xmlns:xlink",value); break;
    case "xHeight": s(n,"x-height",value); break;
    default: s(n,key,value);
  }
}

const config = {
    "accentHeight": {f:s,k:"accent-height"},
    "alignmentBaseline": {f:s,k:"alignment-baseline"},
    "arabicForm": {f:s,k:"arabic-form"},
    "baselineShift": {f:s,k:"baseline-shift"},
    "capHeight": {f:s,k:"cap-height"},
    "clipPath": {f:s,k:"clip-path"},
    "clipRule": {f:s,k:"clip-rule"},
    "colorInterpolation": {f:s,k:"color-interpolation"},
    "colorInterpolationFilters": {f:s,k:"color-interpolation-filters"},
    "colorProfile": {f:s,k:"color-profile"},
    "colorRendering": {f:s,k:"color-rendering"},
    "dominantBaseline": {f:s,k:"dominant-baseline"},
    "enableBackground": {f:s,k:"enable-background"},
    "fillOpacity": {f:s,k:"fill-opacity"},
    "fillRule": {f:s,k:"fill-rule"},
    "floodColor": {f:s,k:"flood-color"},
    "floodOpacity": {f:s,k:"flood-opacity"},
    "fontFamily": {f:s,k:"font-family"},
    "fontSize": {f:s,k:"font-size"},
    "fontSizeAdjust": {f:s,k:"font-size-adjust"},
    "fontStretch": {f:s,k:"font-stretch"},
    "fontStyle": {f:s,k:"font-style"},
    "fontVariant": {f:s,k:"font-variant"},
    "fontWeight": {f:s,k:"font-weight"},
    "glyphName": {f:s,k:"glyph-name"},
    "glyphOrientationHorizontal": {f:s,k:"glyph-orientation-horizontal"},
    "glyphOrientationVertical": {f:s,k:"glyph-orientation-vertical"},
    "horizAdvX": {f:s,k:"horiz-adv-x"},
    "horizOriginX": {f:s,k:"horiz-origin-x"},
    "imageRendering": {f:s,k:"image-rendering"},
    "letterSpacing": {f:s,k:"letter-spacing"},
    "lightingColor": {f:s,k:"lighting-color"},
    "markerEnd": {f:s,k:"marker-end"},
    "markerMid": {f:s,k:"marker-mid"},
    "markerStart": {f:s,k:"marker-start"},
    "overlinePosition": {f:s,k:"overline-position"},
    "overlineThickness": {f:s,k:"overline-thickness"},
    "paintOrder": {f:s,k:"paint-order"},
    "panose-1": {f:s,k:"panose-1"},
    "pointerEvents": {f:s,k:"pointer-events"},
    "renderingIntent": {f:s,k:"rendering-intent"},
    "shapeRendering": {f:s,k:"shape-rendering"},
    "stopColor": {f:s,k:"stop-color"},
    "stopOpacity": {f:s,k:"stop-opacity"},
    "strikethroughPosition": {f:s,k:"strikethrough-position"},
    "strikethroughThickness": {f:s,k:"strikethrough-thickness"},
    "strokeDasharray": {f:s,k:"stroke-dasharray"},
    "strokeDashoffset": {f:s,k:"stroke-dashoffset"},
    "strokeLinecap": {f:s,k:"stroke-linecap"},
    "strokeLinejoin": {f:s,k:"stroke-linejoin"},
    "strokeMiterlimit": {f:s,k:"stroke-miterlimit"},
    "strokeOpacity": {f:s,k:"stroke-opacity"},
    "strokeWidth": {f:s,k:"stroke-width"},
    "textAnchor": {f:s,k:"text-anchor"},
    "textDecoration": {f:s,k:"text-decoration"},
    "textRendering": {f:s,k:"text-rendering"},
    "transformOrigin": {f:s,k:"transform-origin"},
    "underlinePosition": {f:s,k:"underline-position"},
    "underlineThickness": {f:s,k:"underline-thickness"},
    "unicodeBidi": {f:s,k:"unicode-bidi"},
    "unicodeRange": {f:s,k:"unicode-range"},
    "unitsPerEm": {f:s,k:"units-per-em"},
    "vAlphabetic": {f:s,k:"v-alphabetic"},
    "vHanging": {f:s,k:"v-hanging"},
    "vIdeographic": {f:s,k:"v-ideographic"},
    "vMathematical": {f:s,k:"v-mathematical"},
    "vectorEffect": {f:s,k:"vector-effect"},
    "vertAdvY": {f:s,k:"vert-adv-y"},
    "vertOriginX": {f:s,k:"vert-origin-x"},
    "vertOriginY": {f:s,k:"vert-origin-y"},
    "wordSpacing": {f:s,k:"word-spacing"},
    "writingMode": {f:s,k:"writing-mode"},
    "xmlnsXlink": {f:s,k:"xmlns:xlink"},
    "xHeight": {f:s,k:"x-height"},
}

function setPropObj(n, key, value) {
  var c = config[key];
  if (c) {
    var fn = c.f;
    fn(n, c.k, value);
  } else {
    s(n, key, value);
  }
}

const map = new Map([
  ["accentHeight", {f:s,k:"accent-height"}],
  ["alignmentBaseline", {f:s,k:"alignment-baseline"}],
  ["arabicForm", {f:s,k:"arabic-form"}],
  ["baselineShift", {f:s,k:"baseline-shift"}],
  ["capHeight", {f:s,k:"cap-height"}],
  ["clipPath", {f:s,k:"clip-path"}],
  ["clipRule", {f:s,k:"clip-rule"}],
  ["colorInterpolation", {f:s,k:"color-interpolation"}],
  ["colorInterpolationFilters", {f:s,k:"color-interpolation-filters"}],
  ["colorProfile", {f:s,k:"color-profile"}],
  ["colorRendering", {f:s,k:"color-rendering"}],
  ["dominantBaseline", {f:s,k:"dominant-baseline"}],
  ["enableBackground", {f:s,k:"enable-background"}],
  ["fillOpacity", {f:s,k:"fill-opacity"}],
  ["fillRule", {f:s,k:"fill-rule"}],
  ["floodColor", {f:s,k:"flood-color"}],
  ["floodOpacity", {f:s,k:"flood-opacity"}],
  ["fontFamily", {f:s,k:"font-family"}],
  ["fontSize", {f:s,k:"font-size"}],
  ["fontSizeAdjust", {f:s,k:"font-size-adjust"}],
  ["fontStretch", {f:s,k:"font-stretch"}],
  ["fontStyle", {f:s,k:"font-style"}],
  ["fontVariant", {f:s,k:"font-variant"}],
  ["fontWeight", {f:s,k:"font-weight"}],
  ["glyphName", {f:s,k:"glyph-name"}],
  ["glyphOrientationHorizontal", {f:s,k:"glyph-orientation-horizontal"}],
  ["glyphOrientationVertical", {f:s,k:"glyph-orientation-vertical"}],
  ["horizAdvX", {f:s,k:"horiz-adv-x"}],
  ["horizOriginX", {f:s,k:"horiz-origin-x"}],
  ["imageRendering", {f:s,k:"image-rendering"}],
  ["letterSpacing", {f:s,k:"letter-spacing"}],
  ["lightingColor", {f:s,k:"lighting-color"}],
  ["markerEnd", {f:s,k:"marker-end"}],
  ["markerMid", {f:s,k:"marker-mid"}],
  ["markerStart", {f:s,k:"marker-start"}],
  ["overlinePosition", {f:s,k:"overline-position"}],
  ["overlineThickness", {f:s,k:"overline-thickness"}],
  ["paintOrder", {f:s,k:"paint-order"}],
  ["panose-1", {f:s,k:"panose-1"}],
  ["pointerEvents", {f:s,k:"pointer-events"}],
  ["renderingIntent", {f:s,k:"rendering-intent"}],
  ["shapeRendering", {f:s,k:"shape-rendering"}],
  ["stopColor", {f:s,k:"stop-color"}],
  ["stopOpacity", {f:s,k:"stop-opacity"}],
  ["strikethroughPosition", {f:s,k:"strikethrough-position"}],
  ["strikethroughThickness", {f:s,k:"strikethrough-thickness"}],
  ["strokeDasharray", {f:s,k:"stroke-dasharray"}],
  ["strokeDashoffset", {f:s,k:"stroke-dashoffset"}],
  ["strokeLinecap", {f:s,k:"stroke-linecap"}],
  ["strokeLinejoin", {f:s,k:"stroke-linejoin"}],
  ["strokeMiterlimit", {f:s,k:"stroke-miterlimit"}],
  ["strokeOpacity", {f:s,k:"stroke-opacity"}],
  ["strokeWidth", {f:s,k:"stroke-width"}],
  ["textAnchor", {f:s,k:"text-anchor"}],
  ["textDecoration", {f:s,k:"text-decoration"}],
  ["textRendering", {f:s,k:"text-rendering"}],
  ["transformOrigin", {f:s,k:"transform-origin"}],
  ["underlinePosition", {f:s,k:"underline-position"}],
  ["underlineThickness", {f:s,k:"underline-thickness"}],
  ["unicodeBidi", {f:s,k:"unicode-bidi"}],
  ["unicodeRange", {f:s,k:"unicode-range"}],
  ["unitsPerEm", {f:s,k:"units-per-em"}],
  ["vAlphabetic", {f:s,k:"v-alphabetic"}],
  ["vHanging", {f:s,k:"v-hanging"}],
  ["vIdeographic", {f:s,k:"v-ideographic"}],
  ["vMathematical", {f:s,k:"v-mathematical"}],
  ["vectorEffect", {f:s,k:"vector-effect"}],
  ["vertAdvY", {f:s,k:"vert-adv-y"}],
  ["vertOriginX", {f:s,k:"vert-origin-x"}],
  ["vertOriginY", {f:s,k:"vert-origin-y"}],
  ["wordSpacing", {f:s,k:"word-spacing"}],
  ["writingMode", {f:s,k:"writing-mode"}],
  ["xmlnsXlink", {f:s,k:"xmlns:xlink"}],
  ["xHeight", {f:s,k:"x-height"}],
]);

function setPropMap(n, key, value) {
  var c = map.get(key);
  if (c) {
    var fn = c.f;
    fn(n, c.k, value);
  } else {
    s(n, key, value);
  }
}

let N = 1_000_000;
var st, en;

st = Date.now();
for (var i = 0; i < N; i++) {
  setPropSwitch(a, 'colorRendering', 'sdfgsdfg');
  setPropSwitch(a, 'stopColor', 'sdbvbb');
  setPropSwitch(a, 'vertAdvY', i);
  setPropSwitch(a, 'wordSpacing', 'sdfgbxcvb');
  setPropSwitch(a, 'missing', 'sdfgbxcvb');
  setPropSwitch(a, 'writingMode', true);
  setPropSwitch(a, 'missing', 'sdfgbxcvb');
}
en = Date.now();
print('Switch', en - st);

st = Date.now();
for (var i = 0; i < N; i++) {
  setPropObj(b, 'colorRendering', 'sdfgsdfg');
  setPropObj(b, 'stopColor', 'sdbvbb');
  setPropObj(b, 'vertAdvY', i);
  setPropObj(b, 'wordSpacing', 'sdfgbxcvb');
  setPropObj(b, 'missing', 'sdfgbxcvb');
  setPropObj(b, 'writingMode', true);
  setPropObj(b, 'missing', 'sdfgbxcvb');
}
en = Date.now();
print('Object', en - st);

st = Date.now();
for (var i = 0; i < N; i++) {
  setPropMap(c, 'colorRendering', 'sdfgsdfg');
  setPropMap(c, 'stopColor', 'sdbvbb');
  setPropMap(c, 'vertAdvY', i);
  setPropMap(c, 'wordSpacing', 'sdfgbxcvb');
  setPropMap(c, 'missing', 'sdfgbxcvb');
  setPropMap(c, 'writingMode', true);
  setPropMap(c, 'missing', 'sdfgbxcvb');
}
en = Date.now();
print('Map   ', en - st);

})();
