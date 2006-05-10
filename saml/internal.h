/*
 *  Copyright 2001-2005 Internet2
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *  internal.h - internally visible classes
 */

#ifndef __saml_internal_h__
#define __saml_internal_h__

#ifdef WIN32
# define _CRT_SECURE_NO_DEPRECATE 1
# define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

// Export public APIs
#define SAML_EXPORTS

// eventually we might be able to support autoconf via cygwin...
#if defined (_MSC_VER) || defined(__BORLANDC__)
# include "config_win32.h"
#else
# include "config.h"
#endif

#include "base.h"
#include "SAMLConfig.h"

#define SAML_LOGCAT "OpenSAML"

namespace opensaml {
    
    /// @cond OFF
    class SAMLInternalConfig : public SAMLConfig
    {
    public:
        SAMLInternalConfig() {}

        static SAMLInternalConfig& getInternalConfig();

        // global per-process setup and shutdown of runtime
        bool init();
        void term();

        void generateRandomBytes(void* buf, unsigned int len);
        void generateRandomBytes(std::string& buf, unsigned int len);
        XMLCh* generateIdentifier();
    private:
    };
    /// @endcond

};

#endif /* __saml_internal_h__ */