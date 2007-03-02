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
 * @file saml/saml2/binding/SAML2SOAPEncoder.h
 * 
 * SAML 2.0 SOAP binding message encoder
 */

#include <saml/binding/MessageEncoder.h>


namespace opensaml {
    namespace saml2p {

        /**
         * SAML 2.0 POST binding message encoder
         */
        class SAML_API SAML2SOAPEncoder : public MessageEncoder
        {
        public:
            SAML2SOAPEncoder(const DOMElement* e);
            virtual ~SAML2SOAPEncoder() {}
            
            long encode(
                GenericResponse& genericResponse,
                xmltooling::XMLObject* xmlObject,
                const char* destination,
                const char* recipientID=NULL,
                const char* relayState=NULL,
                const xmltooling::CredentialResolver* credResolver=NULL,
                const XMLCh* sigAlgorithm=NULL
                ) const;
        };

    };
};
