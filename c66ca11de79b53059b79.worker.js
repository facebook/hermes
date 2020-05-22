/******/ (function(modules) { // webpackBootstrap
/******/ 	// The module cache
/******/ 	var installedModules = {};
/******/
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/
/******/ 		// Check if module is in cache
/******/ 		if(installedModules[moduleId]) {
/******/ 			return installedModules[moduleId].exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = installedModules[moduleId] = {
/******/ 			i: moduleId,
/******/ 			l: false,
/******/ 			exports: {}
/******/ 		};
/******/
/******/ 		// Execute the module function
/******/ 		modules[moduleId].call(module.exports, module, module.exports, __webpack_require__);
/******/
/******/ 		// Flag the module as loaded
/******/ 		module.l = true;
/******/
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/
/******/
/******/ 	// expose the modules object (__webpack_modules__)
/******/ 	__webpack_require__.m = modules;
/******/
/******/ 	// expose the module cache
/******/ 	__webpack_require__.c = installedModules;
/******/
/******/ 	// define getter function for harmony exports
/******/ 	__webpack_require__.d = function(exports, name, getter) {
/******/ 		if(!__webpack_require__.o(exports, name)) {
/******/ 			Object.defineProperty(exports, name, { enumerable: true, get: getter });
/******/ 		}
/******/ 	};
/******/
/******/ 	// define __esModule on exports
/******/ 	__webpack_require__.r = function(exports) {
/******/ 		if(typeof Symbol !== 'undefined' && Symbol.toStringTag) {
/******/ 			Object.defineProperty(exports, Symbol.toStringTag, { value: 'Module' });
/******/ 		}
/******/ 		Object.defineProperty(exports, '__esModule', { value: true });
/******/ 	};
/******/
/******/ 	// create a fake namespace object
/******/ 	// mode & 1: value is a module id, require it
/******/ 	// mode & 2: merge all properties of value into the ns
/******/ 	// mode & 4: return value when already ns object
/******/ 	// mode & 8|1: behave like require
/******/ 	__webpack_require__.t = function(value, mode) {
/******/ 		if(mode & 1) value = __webpack_require__(value);
/******/ 		if(mode & 8) return value;
/******/ 		if((mode & 4) && typeof value === 'object' && value && value.__esModule) return value;
/******/ 		var ns = Object.create(null);
/******/ 		__webpack_require__.r(ns);
/******/ 		Object.defineProperty(ns, 'default', { enumerable: true, value: value });
/******/ 		if(mode & 2 && typeof value != 'string') for(var key in value) __webpack_require__.d(ns, key, function(key) { return value[key]; }.bind(null, key));
/******/ 		return ns;
/******/ 	};
/******/
/******/ 	// getDefaultExport function for compatibility with non-harmony modules
/******/ 	__webpack_require__.n = function(module) {
/******/ 		var getter = module && module.__esModule ?
/******/ 			function getDefault() { return module['default']; } :
/******/ 			function getModuleExports() { return module; };
/******/ 		__webpack_require__.d(getter, 'a', getter);
/******/ 		return getter;
/******/ 	};
/******/
/******/ 	// Object.prototype.hasOwnProperty.call
/******/ 	__webpack_require__.o = function(object, property) { return Object.prototype.hasOwnProperty.call(object, property); };
/******/
/******/ 	// __webpack_public_path__
/******/ 	__webpack_require__.p = "/";
/******/
/******/
/******/ 	// Load entry module and return exports
/******/ 	return __webpack_require__(__webpack_require__.s = 0);
/******/ })
/************************************************************************/
/******/ ([
/* 0 */
/***/ (function(module, exports) {

/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */importScripts('hermes.js');var output='';var runRequested=0;var runArgs,runData;var firstInitialization=true;var runtimeInitialized;var app;var counter=0;startRuntimeInitialization(true);var lastTime;function ellapsed(){if(!lastTime){lastTime=new Date();return 0+' ms';}var newTime=new Date();var res=newTime-lastTime;lastTime=newTime;return res+' ms';}function startRuntimeInitialization(first){console.log(ellapsed(),'starting runtime initialization');runtimeInitialized=false;app=createApp({print:handleStdout,printErr:handleStdout,onRuntimeInitialized:first?onFirstRuntimeInitialization:onRuntimeInitialized});}function handleStdout(txt){output+=txt;output+='\n';}function onFirstRuntimeInitialization(){console.log(ellapsed(),'first runtime initialized');console.log(ellapsed(),'preheating hermes...');// Warm up Hermes with a basic input. This will force compile all of asm.js.
var fileName='/tmp/hermes-input.js';app.FS.writeFile(fileName,'var x = 10;');app.callMain(['-O',fileName]);output='';console.log(ellapsed(),'hermes is preheated');startRuntimeInitialization();}function onRuntimeInitialized(){console.log(ellapsed(),'runtime initialized');runtimeInitialized=true;if(runRequested)runHermes();}onmessage=function(e){console.log(ellapsed(),'received a message');switch(e.data[0]){case'run':onRunHermes(e.data[1],e.data[2]);break;}};function onRunHermes(args,data){if(runRequested)return;runArgs=args;runData=data;runRequested=1;console.log(ellapsed(),'run requested');if(runtimeInitialized)runHermes();}function runHermes(){console.log(ellapsed(),'running hermes');runRequested=2;var fileName='/tmp/hermes-input.js';app.FS.writeFile(fileName,runData);runArgs.push('--pretty-json');runArgs.push(fileName);app.callMain(runArgs);sendRunResult(output);runArgs=undefined;runData=undefined;output='';runRequested=0;startRuntimeInitialization();}function sendRunResult(result){console.log(ellapsed(),'sending result');postMessage(['runResult',result]);}

/***/ })
/******/ ]);