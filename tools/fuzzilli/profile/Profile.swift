// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import Fuzzilli

struct Profile {
    let processArguments: [String]
    let processEnv: [String : String]
    let codePrefix: String
    let codeSuffix: String
    let ecmaVersion: ECMAScriptVersion

    // JavaScript code snippets that cause a crash in the target engine.
    // Used to verify that crashes can be detected.
    let crashTests: [String]

    let additionalCodeGenerators: WeightedList<CodeGenerator>
    let additionalProgramTemplates: WeightedList<ProgramTemplate>

    let disabledCodeGenerators: [String]

    let additionalBuiltins: [String: Type]
}

let profiles = [
    "hermes": hermesProfile,
]
