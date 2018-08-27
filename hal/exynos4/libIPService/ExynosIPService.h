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

#ifndef ANDROID_EXYNOS_IP_SERVICE_H_
#define ANDROID_EXYNOS_IP_SERVICE_H_

#include <utils/Errors.h>
#include <sys/types.h>
#include <cutils/log.h>
#include <binder/IServiceManager.h>
#include <utils/Singleton.h>
#include <utils/StrongPointer.h>
#include "IExynosIP.h"

namespace android {

class ExynosIPService
    : public BnExynosIPService, Singleton<ExynosIPService> {

public:
    static ExynosIPService* getExynosIPService();
    ~ExynosIPService();
private:
    friend class Singleton<ExynosIPService>;
    ExynosIPService();
    int createServiceLocked();
    ExynosIPService *mIPService;
    Mutex mLock;
};

}
#endif