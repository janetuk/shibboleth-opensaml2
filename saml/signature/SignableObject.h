/*
 *  Copyright 2001-2006 Internet2
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
 * @file SignableObject.h
 * 
 * Base class for SAML objects that can be signed. 
 */

#ifndef __saml_signable_h__
#define __saml_signable_h__

#include <saml/base.h>
#include <saml/signature/ContentReference.h>
#include <xmltooling/XMLObject.h>

namespace opensaml {

    /**
     * Base class for SAML objects that can be signed.
     */
    class SAML_API SignableObject : public virtual xmltooling::XMLObject
    {
    public:
        virtual ~SignableObject() {}

        /**
         * Returns the XML ID for the object being signed, for input to the
         * reference creation process. 
         * 
         * @return      XML ID or NULL if not set yet 
         */
        virtual const XMLCh* getId() const=0;

        /**
         * Gets a new ContentReference object bound to this object.
         * It's lifetime must not outlast this object, so it should
         * generally be set into a Signature owned by the object.
         * 
         * @return  a new ContentReference
         */
        virtual ContentReference* getContentReference() const {
            return new ContentReference(*this);
        }
        
    protected:
        SignableObject() {}
    };

};

#endif /* __saml_signable_h__ */