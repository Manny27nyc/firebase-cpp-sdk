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

#import <GoogleMobileAds/GoogleMobileAds.h>

#include "admob/src/include/firebase/admob.h"
#include "admob/src/include/firebase/admob/types.h"
#include "admob/src/ios/ad_result_ios.h"
#include "admob/src/ios/response_info_ios.h"
#include "app/src/util_ios.h"

namespace firebase {
namespace admob {

LoadAdResult::LoadAdResult() : AdResult(), response_info_() {}

LoadAdResult::LoadAdResult(const AdResultInternal& ad_result_internal)
  : AdResult(ad_result_internal),
    response_info_(ResponseInfoInternal( {nil} ) ) {

  if (!ad_result_internal.is_successful &&
      !ad_result_internal.is_wrapper_error) {

    FIREBASE_ASSERT(ad_result_internal.ios_error);
    ResponseInfoInternal response_info_internal = ResponseInfoInternal( {
      ad_result_internal.ios_error.userInfo[GADErrorUserInfoKeyResponseInfo]
    });
    response_info_ = ResponseInfo(response_info_internal);
  }
}

LoadAdResult::LoadAdResult(const LoadAdResult& load_ad_result)
    : AdResult(load_ad_result) {
  response_info_ = load_ad_result.response_info_;
}

}  // namespace admob
}  // namespace firebase
