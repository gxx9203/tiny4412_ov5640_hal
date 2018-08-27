/*
 * Copyright (C) 2012 The Android Open Source Project
 * Copyright (C) 2015 NamelessRom
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

#define IP_SERVICE_DEBUG 1

#include "ExynosIPService.h"

namespace android {

ANDROID_SINGLETON_STATIC_INSTANCE(ExynosIPService);

ExynosIPService::ExynosIPService() :
    mIPService(NULL)
{
   ALOGD_IF(IP_SERVICE_DEBUG, "ExynosIPService Constructor is called");
}

ExynosIPService::~ExynosIPService()
{
   ALOGD_IF(IP_SERVICE_DEBUG, "ExynosIPService Destructor is called");
}

int ExynosIPService::createServiceLocked()
{
    ALOGD_IF(IP_SERVICE_DEBUG, "%s::", __func__);
    sp<IServiceManager> sm = defaultServiceManager();
    sm->addService(String16("Exynos.IPService"), mIPService);
    if (sm->checkService(String16("Exynos.IPService")) != NULL) {
        ALOGD_IF(IP_SERVICE_DEBUG, "adding Exynos.IPService succeeded");
        return 0;
    } else {
        ALOGE_IF(IP_SERVICE_DEBUG, "adding Exynos.IPService failed");
        return -1;
    }
}

ExynosIPService *ExynosIPService::getExynosIPService()
{
    ALOGD_IF(IP_SERVICE_DEBUG, "%s::", __func__);
    ExynosIPService& instance = ExynosIPService::getInstance();
    Mutex::Autolock _l(instance.mLock);
    if (instance.mIPService == NULL) {
        instance.mIPService = &instance;

        int status = ExynosIPService::getInstance().createServiceLocked();
        if (status != 0) {
            ALOGE_IF(IP_SERVICE_DEBUG, "getExynosIPService failed");
        }
    }
    return instance.mIPService;
}
}
