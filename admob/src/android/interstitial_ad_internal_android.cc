/*
 * Copyright 2016 Google LLC
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

#include "admob/src/android/interstitial_ad_internal_android.h"

#include <assert.h>
#include <jni.h>

#include <cstdarg>
#include <cstddef>

#include "admob/admob_resources.h"
#include "admob/src/android/ad_request_converter.h"
#include "admob/src/android/admob_android.h"
#include "admob/src/common/admob_common.h"
#include "admob/src/include/firebase/admob.h"
#include "admob/src/include/firebase/admob/interstitial_ad.h"
#include "admob/src/include/firebase/admob/types.h"
#include "app/src/assert.h"
#include "app/src/util_android.h"

namespace firebase {
namespace admob {

METHOD_LOOKUP_DEFINITION(
    interstitial_ad_helper,
    "com/google/firebase/admob/internal/cpp/InterstitialAdHelper",
    INTERSTITIALADHELPER_METHODS);

namespace internal {

InterstitialAdInternalAndroid::InterstitialAdInternalAndroid(
    InterstitialAd* base)
    : InterstitialAdInternal(base), helper_(nullptr), initialized_(false) {
  JNIEnv* env = ::firebase::admob::GetJNI();
  jobject helper_ref = env->NewObject(
      interstitial_ad_helper::GetClass(),
      interstitial_ad_helper::GetMethodId(interstitial_ad_helper::kConstructor),
      reinterpret_cast<jlong>(this));

  FIREBASE_ASSERT(helper_ref);
  helper_ = env->NewGlobalRef(helper_ref);
  FIREBASE_ASSERT(helper_);
  env->DeleteLocalRef(helper_ref);
}

InterstitialAdInternalAndroid::~InterstitialAdInternalAndroid() {
  JNIEnv* env = ::firebase::admob::GetJNI();
  // Since it's currently not possible to destroy the intersitial ad, just
  // disconnect from it so the listener doesn't initiate callbacks with stale
  // data.
  env->CallVoidMethod(helper_, interstitial_ad_helper::GetMethodId(
                                   interstitial_ad_helper::kDisconnect));
  env->DeleteGlobalRef(helper_);
  helper_ = nullptr;
}

Future<void> InterstitialAdInternalAndroid::Initialize(AdParent parent) {
  FutureCallbackData<void>* callback_data =
      CreateVoidFutureCallbackData(kInterstitialAdFnInitialize, &future_data_);
  SafeFutureHandle<void> future_handle = callback_data->future_handle;
  if (initialized_) {
    CompleteFuture(kAdMobErrorAlreadyInitialized,
                   kAdAlreadyInitializedErrorMessage, future_handle,
                   &future_data_);
  } else {
    initialized_ = true;

    JNIEnv* env = ::firebase::admob::GetJNI();
    FIREBASE_ASSERT(env);
    env->CallVoidMethod(helper_,
                        interstitial_ad_helper::GetMethodId(
                            interstitial_ad_helper::kInitialize),
                        reinterpret_cast<jlong>(callback_data), parent);
  }

  return MakeFuture(&future_data_.future_impl, future_handle);
}

Future<LoadAdResult> InterstitialAdInternalAndroid::LoadAd(
    const char* ad_unit_id, const AdRequest& request) {
  admob::AdMobError error = kAdMobErrorNone;
  jobject j_request = GetJavaAdRequestFromCPPAdRequest(request, &error);

  if (j_request == nullptr) {
    if (error == kAdMobErrorNone) {
      error = kAdMobErrorInternalError;
    }
    return CreateAndCompleteFutureWithResult(
        kInterstitialAdFnLoadAd, error, kAdCouldNotParseAdRequestErrorMessage,
        &future_data_, LoadAdResult());
  }
  FutureCallbackData<LoadAdResult>* callback_data =
      CreateLoadAdResultFutureCallbackData(kInterstitialAdFnLoadAd,
                                           &future_data_);
  SafeFutureHandle<LoadAdResult> future_handle = callback_data->future_handle;

  JNIEnv* env = GetJNI();
  FIREBASE_ASSERT(env);

  jstring j_ad_unit_str = env->NewStringUTF(ad_unit_id);
  ::firebase::admob::GetJNI()->CallVoidMethod(
      helper_,
      interstitial_ad_helper::GetMethodId(interstitial_ad_helper::kLoadAd),
      reinterpret_cast<jlong>(callback_data), j_ad_unit_str, j_request);
  env->DeleteLocalRef(j_ad_unit_str);
  env->DeleteLocalRef(j_request);

  return MakeFuture(&future_data_.future_impl, future_handle);
}

Future<void> InterstitialAdInternalAndroid::Show() {
  FutureCallbackData<void>* callback_data =
      CreateVoidFutureCallbackData(kInterstitialAdFnLoadAd, &future_data_);
  SafeFutureHandle<void> future_handle = callback_data->future_handle;

  ::firebase::admob::GetJNI()->CallVoidMethod(
      helper_,
      interstitial_ad_helper::GetMethodId(interstitial_ad_helper::kShow),
      reinterpret_cast<jlong>(callback_data));

  return MakeFuture(&future_data_.future_impl, future_handle);
}

InterstitialAd::PresentationState
InterstitialAdInternalAndroid::GetPresentationState() const {
  jint state = ::firebase::admob::GetJNI()->CallIntMethod(
      helper_, interstitial_ad_helper::GetMethodId(
                   interstitial_ad_helper::kGetPresentationState));
  assert((static_cast<int>(state) >= 0));
  return static_cast<InterstitialAd::PresentationState>(state);
}

}  // namespace internal
}  // namespace admob
}  // namespace firebase
