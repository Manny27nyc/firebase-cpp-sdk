/*
 * Copyright 2021 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "admob/src/android/response_info_android.h"

#include "admob/src/android/ad_result_android.h"
#include "admob/src/android/adapter_response_info_android.h"
#include "admob/src/android/admob_android.h"
#include "admob/src/common/admob_common.h"
#include "admob/src/include/firebase/admob.h"

namespace firebase {
namespace admob {

METHOD_LOOKUP_DEFINITION(response_info,
                         PROGUARD_KEEP_CLASS
                         "com/google/android/gms/ads/ResponseInfo",
                         RESPONSEINFO_METHODS);

ResponseInfo::ResponseInfo() {
  to_string_ = "This ResponseInfo has not been initialized.";
}

ResponseInfo::ResponseInfo(const ResponseInfoInternal& response_info_internal) {
  FIREBASE_ASSERT(response_info_internal.j_response_info);

  const jobject j_response_info = response_info_internal.j_response_info;
  JNIEnv* env = GetJNI();
  FIREBASE_ASSERT(env);

  const jobject j_adapter_response_info_list = env->CallObjectMethod(
      j_response_info,
      response_info::GetMethodId(response_info::kGetAdapterResponses));
  FIREBASE_ASSERT(j_adapter_response_info_list);

  const int list_size = (int)env->CallIntMethod(
      j_adapter_response_info_list, util::list::GetMethodId(util::list::kSize));
  for (int i = 0; i < list_size; ++i) {
    jobject j_adapter_response_info =
        env->CallObjectMethod(j_adapter_response_info_list,
                              util::list::GetMethodId(util::list::kGet), i);
    FIREBASE_ASSERT(j_adapter_response_info);
    AdapterResponseInfoInternal adapter_response_internal;
    adapter_response_internal.j_adapter_response_info = j_adapter_response_info;
    adapter_responses_.push_back(
        AdapterResponseInfo(adapter_response_internal));
    env->DeleteLocalRef(j_adapter_response_info);
  }

  const jobject j_medation_adapter_classname = env->CallObjectMethod(
      j_response_info,
      response_info::GetMethodId(response_info::kGetMediationAdapterClassName));
  FIREBASE_ASSERT(j_medation_adapter_classname);
  mediation_adapter_class_name_ =
      util::JStringToString(env, j_medation_adapter_classname);
  env->DeleteLocalRef(j_medation_adapter_classname);

  const jobject j_response_id = env->CallObjectMethod(
      j_response_info,
      response_info::GetMethodId(response_info::kGetResponseId));
  FIREBASE_ASSERT(j_response_id);
  mediation_adapter_class_name_ = util::JStringToString(env, j_response_id);
  env->DeleteLocalRef(j_response_id);

  const jobject j_to_string = env->CallObjectMethod(
      j_response_info, response_info::GetMethodId(response_info::kToString));
  FIREBASE_ASSERT(j_to_string);
  to_string_ = util::JStringToString(env, j_to_string);
  env->DeleteLocalRef(j_to_string);
}

}  // namespace admob
}  // namespace firebase