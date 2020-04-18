// Copyright 2011 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: jdtang@google.com (Jonathan Tang)

#ifndef GUMBO_TEST_UTILS_H_
#define GUMBO_TEST_UTILS_H_

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "gumbo.h"

int clean_text_main(int argc, char** argv);
int find_links_main(int argc, char** argv);
int get_title_main(int argc, const char** argv);
int positions_of_class_main(int argc, char** argv);
int prettyprint_main(int argc, char** argv);
int serialize_main(int argc, char** argv);


#endif  // GUMBO_TEST_UTILS_H_
