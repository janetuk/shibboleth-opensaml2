/*
 *  Copyright 2001-2007 Internet2
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

/**
 * @file saml/Assertion.h
 * 
 * Base class for SAML assertions.
 */

#ifndef __saml_assertion_h__
#define __saml_assertion_h__

#include <saml/RootObject.h>

namespace opensaml {

    /**
     * Base class for SAML assertions.
     * Currently just a marker interface.
     */
    class SAML_API Assertion : public virtual RootObject
    {
    public:
        virtual ~Assertion();
    protected:
        Assertion();
    };

};

#endif /* __saml_assertion_h__ */