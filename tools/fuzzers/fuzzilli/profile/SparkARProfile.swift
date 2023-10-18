/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import Fuzzilli

let sparkARProfile = Profile(
    getProcessArguments:  { (randomizingArguments: Bool) -> [String] in
      var args = ["--reprl"]

      return args
    },

    processEnv: ["UBSAN_OPTIONS": "handle_segv=0"],

    maxExecsBeforeRespawn: 1000,

    timeout: 2000,

    codePrefix: """
                function main(){
                  const aclip0 = Animation.animationClips.findFirst("aclip0");
                  const acontroller0 = Animation.playbackControllers.findFirst("acontroller0");
                  const ablockproto0 = Blocks.assets.findFirst("ablockproto0");
                  const efont0 = Fonts.findFirst("efont0");
                  const lfont0 = Fonts.findFirst("lfont0");
                  const scene0 = Scene.root.findFirst("scene0");
                  const mat0 = Materials.findFirst("mat0");
                  const svg0 = Svgs.findFirst("svg0");
                  const texture0 = Textures.findFirst("texture0");
                  const prefab0 = Prefabs.findFirst("prefab0");
                  // START FUZZILLI INPUT
                """,

    codeSuffix: """
                  // END FUZZILLI INPUT
                }; main();
                """,

    ecmaVersion: ECMAScriptVersion.es6,

    crashTests: ["fuzzilli('FUZZILLI_CRASH', 0)", "fuzzilli('FUZZILLI_CRASH', 1)", "fuzzilli('FUZZILLI_CRASH', 2)"],

    additionalCodeGenerators: [],

    additionalProgramTemplates: WeightedList<ProgramTemplate>([]),

    disabledCodeGenerators: ["AsyncArrowFunctionGenerator", "AsyncGeneratorFunctionGenerator", "ClassGenerator", "WithStatementGenerator", "JITFunctionGenerator", "GrowableSharedArrayBufferGenerator"],

    additionalBuiltins: [
        // Hermes builtins
        "gc"                                             : .function([] => .undefined),
        "print"                                          : .function([.plain(.anything)] => .undefined),
        "HermesInternal.enqueueJob"                      : .function([.plain(.function())] => .undefined),
        "HermesInternal.setPromiseRejectionTrackingHook" : .function([.plain(.function())] => .undefined),
        "HermesInternal.enablePromiseRejectionTracker"   : .function([.plain(.anything)] => .anything),
        "HermesInternal.getEpilogues"                    : .function([] => .iterable),
        "HermesInternal.getInstrumentedStats"            : .function([] => .object(ofGroup: "Object", withProperties: ["js_VMExperiments", "js_numGCs", "js_gcCPUTime", "js_gcTime", "js_totalAllocatedBytes", "js_allocatedBytes", "js_heapSize", "js_mallocSizeEstimate", "js_vaSize", "js_markStackOverflows"])),
        "HermesInternal.getRuntimeProperties"            : .function([] => .object(ofGroup: "Object", withProperties: ["Snapshot VM", "Bytecode Version", "Builtins Frozen", "VM Experiments", "Build", "GC", "OSS Release Version", "CommonJS Modules"])),
        "HermesInternal.ttiReached"                      : .function([] => .undefined),
        "HermesInternal.getFunctionLocation"             : .function([.plain(.function())] => .object(ofGroup: "Object", withProperties: ["isNative", "lineNumber", "columnNumber", "fileName"])),

        // The methods below are disabled since they are not very interesting to fuzz
        // "HermesInternal.hasPromise"                   : .function([] => .boolean),
        // "HermesInternal.useEngineQueue"               : .function([] => .boolean),
        // "HermesInternal.ttrcReached"                  : .function([] => .undefined),

        // Injected AR scripting resources
        "aclip0"                                         : .jsPromise,
        "acontroller0"                                   : .jsPromise,
        "ablockproto0"                                   : .jsPromise,
        "efont0"                                         : .jsPromise,
        "lfont0"                                         : .jsPromise,
        "scene0"                                         : .jsPromise,
        "mat0"                                           : .jsPromise,
        "svg0"                                           : .jsPromise,
        "texture0"                                       : .jsPromise,
        "prefab0"                                        : .jsPromise,

        // Scene https://spark.meta.com/learn/reference/classes/scenemodule
        "Scene"                                          : .object(ofGroup: "Object", withProperties: ["root"], withMethods: ["clone", "create", "destroy", "projectToScreen", "unprojectToFocalPlane", "worldTransform"]),
        "Scene.root"                                     : .object(ofGroup: "Object", withProperties: ["hidden", "outputVisibility", "worldTransform"]),
        "Scene.root.addChild"                            : .function([.plain(.anything)] => .jsPromise),
        "Scene.root.findAll"                             : .function([.plain(.string), .opt(.object(ofGroup: "Object", withProperties: ["recursive"]))] => .jsPromise),
        "Scene.root.findByAllTags"                       : .function([.plain(.jsArray), .opt(.object(ofGroup: "Object", withProperties: ["limit"]))] => .jsPromise),
        "Scene.root.findByAnyTags"                       : .function([.plain(.jsArray), .opt(.object(ofGroup: "Object", withProperties: ["limit"]))] => .jsPromise),
        "Scene.root.findByPath"                          : .function([.plain(.string), .opt(.object(ofGroup: "Object", withProperties: ["limit"]))] => .jsPromise),
        "Scene.root.findByTag"                           : .function([.plain(.string), .opt(.object(ofGroup: "Object", withProperties: ["limit"]))] => .jsPromise),
        "Scene.root.findFirst"                           : .function([.plain(.string), .opt(.object(ofGroup: "Object", withProperties: ["recursive"]))] => .jsPromise),
        "Scene.root.removeChild"                         : .function([.plain(.anything)] => .jsPromise),

        // Units https://spark.meta.com/learn/reference/classes/UnitsModule
        "Units"                                          : .object(ofGroup: "Object", withProperties: [], withMethods: ["cm", "ft","in","m","mm","yd"]),

        // CameraInfo https://spark.meta.com/learn/reference/classes/CameraInfoModule
        "CameraInfo"                                     : .object(ofGroup: "Object", withProperties: ["captureDevicePosition", "effectSafeAreaInsets", "isCapturingPhoto", "isRecordingVideo", "previewScreenScale", "previewSize", "viewMatrix"]),

        // Blocks https://spark.meta.com/learn/reference/classes/blocksmodule
        "Blocks"                                        : .object(ofGroup: "Object", withProperties: ["assets", "inputs", "modulesConfigExtras", "outputs"], withMethods: ["download", "instantiate"]),

        // Fonts https://spark.meta.com/learn/reference/classes/fontsmodule
        "Fonts"                                          : .object(ofGroup: "Object", withMethods: ["findFirst", "findUsingPattern", "getAll"]),

        // Svgs https://spark.meta.com/learn/reference/classes/svgsmodule
        "Svgs"                                            : .object(ofGroup: "Object", withMethods: ["findFirst", "findUsingPattern", "getAll"]),

        // Animation https://spark.meta.com/learn/reference/classes/animationmodule
        "Animation"                                       : .object(ofGroup: "Object", withProperties: ["animationClips", "playbackControllers", "samplers"], withMethods: ["animate", "animationBlend", "blendInput", "timeDriver", "valueDriver"]),

        // Diagnostic https://spark.meta.com/learn/reference/classes/diagnosticsmodule
        "Diagnostics"                                     : .object(ofGroup: "Object", withProperties: ["typeSystem"], withMethods: ["error", "log", "warn", "watch"]),

        // Materials https://spark.meta.com/learn/reference/classes/materialsmodule
        "Materials"                                       : .object(ofGroup: "Object", withMethods: ["clone", "create", "destroy", "findFirst", "findUsingPattern", "getAll"]),

        // Shaders https://spark.meta.com/learn/reference/classes/ShadersModule
        "Shaders"                                         : .object(ofGroup: "Object", withMethods: [
          "blend", "colorSpaceConvert", "composition", "derivative", "fallback", "fragmentStage", "functionScalar", "functionVec2", "functionVec3", "functionVec4", "gradient",
          "renderTargetSize", "sdfAnnular", "sdfCircle", "sdfComplement", "sdfDifference", "sdfEllipse", "sdfFlip", "sdfHalfPlane", "sdfIntersection", "sdfLine", "sdfMix", "sdfPolygon",
          "sdfRectangle", "sdfRepeat", "sdfRotation", "sdfRotationalRepeat","sdfRound", "sdfScale", "sdfShear", "sdfSmoothDifference", "sdfSmoothIntersection", "sdfSmoothUnion",
          "sdfStar", "sdfTranslation", "sdfTwist", "sdfUnion", "textureSampler", "textureTransform", "vertexAttribute", "vertexTransform"]),

        // Time https://spark.meta.com/learn/reference/classes/TimeModule
        "Time"                                            : .object(ofGroup: "Object", withProperties: ["deltaTime", "ms"], withMethods: ["clearInterval", "clearTimeout", "setInterval", "setIntervalWithSnapshot", "setTimeout", "setTimeoutWithSnapshot"]),

        // Prefabs https://spark.meta.com/learn/reference/classes/PrefabsModule
        "Prefabs"                                         : .object(ofGroup: "Object", withMethods: ["findFirst", "findUsingPattern", "getAll"]),

        // Textures https://spark.meta.com/learn/reference/classes/texturesmodule
        "Textures"                                        : .object(ofGroup: "Object", withMethods: ["clone", "create", "destroy", "findFirst", "findUsingPattern", "getAll"]),

        // Reactive https://spark.meta.com/learn/reference/classes/reactivemodule
        "R"                                               : .object(ofGroup: "Object", withProperties: ["Box2D", "Box3D", "Color", "ColorModel", "Mat4", "Rotation", "Vec2", "Vec3", "Vec4"], withMethods: [
          "abs", "acos", "add", "and", "andList", "antiderivative", "asin", "atan", "atan2", "boolSignalSource", "boundingBox", "box2d", "box3d", "ceil", "clamp",
          "concat", "cos", "cross", "derivative", "distance", "div", "dot", "eq", "eventEmitter", "exp", "expSmooth", "floor", "fromRange", "ge", "gt", "HSVA",
          "le", "log", "lookAt", "lt", "magnitude", "magnitudeSquared", "max", "min", "mix", "mod", "monitorMany", "mul", "mulList", "ne", "neg", "normalize", "not",
          "once", "or", "orList", "pack2", "pack3", "pack4", "point", "point2d", "pow", "quaternion", "quaternionFromAngleAxis", "quaternionFromEuler",
          "quaternionFromTo", "quaternionIdentity", "quaternionLookAt", "reflect", "RGBA", "rotation", "round", "scalarSignalSource", "scale", "schmittTrigger", "sign",
          "sin", "smoothStep", "sqrt", "step", "stringSignalSource", "sub", "sum", "sumList", "switch", "tan", "toRange", "transform", "val", "vector", "xor", "xorList",
        ])
    ]
)
