/**
 * Licensed to the University Corporation for Advanced Internet
 * Development, Inc. (UCAID) under one or more contributor license
 * agreements. See the NOTICE file distributed with this work for
 * additional information regarding copyright ownership.
 *
 * UCAID licenses this file to you under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the
 * License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

/**
 * SAML2SOAPEncoder.cpp
 * 
 * SAML 2.0 SOAP binding message encoder.
 */

#include "internal.h"
#include "exceptions.h"
#include "binding/MessageEncoder.h"
#include "signature/ContentReference.h"
#include "saml2/core/Protocols.h"

#include <sstream>
#include <xmltooling/logging.h>
#include <xmltooling/io/HTTPResponse.h>
#include <xmltooling/util/NDC.h>
#include <xmltooling/signature/Signature.h>
#include <xmltooling/soap/SOAP.h>

using namespace opensaml::saml2p;
using namespace opensaml::saml2md;
using namespace opensaml;
using namespace xmlsignature;
using namespace soap11;
using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

namespace opensaml {
    namespace saml2p {              
        class SAML_DLLLOCAL SAML2SOAPEncoder : public MessageEncoder
        {
        public:
            SAML2SOAPEncoder() {}
            virtual ~SAML2SOAPEncoder() {}

            bool isUserAgentPresent() const {
                return false;
            }

            const XMLCh* getProtocolFamily() const {
                return samlconstants::SAML20P_NS;
            }

            long encode(
                GenericResponse& genericResponse,
                XMLObject* xmlObject,
                const char* destination,
                const EntityDescriptor* recipient=nullptr,
                const char* relayState=nullptr,
                const ArtifactGenerator* artifactGenerator=nullptr,
                const Credential* credential=nullptr,
                const XMLCh* signatureAlg=nullptr,
                const XMLCh* digestAlg=nullptr
                ) const;
        };

        MessageEncoder* SAML_DLLLOCAL SAML2SOAPEncoderFactory(const pair<const DOMElement*,const XMLCh*>& p)
        {
            return new SAML2SOAPEncoder();
        }
    };
};

long SAML2SOAPEncoder::encode(
    GenericResponse& genericResponse,
    XMLObject* xmlObject,
    const char* destination,
    const EntityDescriptor* recipient,
    const char* relayState,
    const ArtifactGenerator* artifactGenerator,
    const Credential* credential,
    const XMLCh* signatureAlg,
    const XMLCh* digestAlg
    ) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("encode");
#endif
    Category& log = Category::getInstance(SAML_LOGCAT".MessageEncoder.SAML2SOAP");

    log.debug("validating input");
    if (xmlObject->getParent())
        throw BindingException("Cannot encode XML content with parent.");

    genericResponse.setContentType("text/xml");
    HTTPResponse* httpResponse = dynamic_cast<HTTPResponse*>(&genericResponse);
    if (httpResponse) {
        httpResponse->setResponseHeader("Expires", "01-Jan-1997 12:00:00 GMT");
        httpResponse->setResponseHeader("Cache-Control", "no-cache, no-store, must-revalidate, private");
        httpResponse->setResponseHeader("Pragma", "no-cache");
    }

    bool detachOnFailure = false;
    DOMElement* rootElement = nullptr;

    // Check for a naked message.
    SignableObject* msg = dynamic_cast<SignableObject*>(xmlObject);
    if (msg) {
        // Wrap it in a SOAP envelope and point xmlObject at that.
        detachOnFailure = true;
        Envelope* env = EnvelopeBuilder::buildEnvelope();
        Body* body = BodyBuilder::buildBody();
        env->setBody(body);
        body->getUnknownXMLObjects().push_back(msg);
        xmlObject = env;
    }

    Envelope* env = dynamic_cast<Envelope*>(xmlObject);
    if (env) {
        if (!msg) {
            msg = (env->getBody() && env->getBody()->hasChildren()) ?
                dynamic_cast<SignableObject*>(env->getBody()->getUnknownXMLObjects().front()) : nullptr;
        }
        try {
            if (msg && credential) {
                if (msg->getSignature()) {
                    log.debug("message already signed, skipping signature operation");
                    rootElement = env->marshall();
                }
                else {
                    log.debug("signing the message and marshalling the envelope");
        
                    // Build a Signature.
                    Signature* sig = SignatureBuilder::buildSignature();
                    msg->setSignature(sig);    
                    if (signatureAlg)
                        sig->setSignatureAlgorithm(signatureAlg);
                    if (digestAlg) {
                        opensaml::ContentReference* cr = dynamic_cast<opensaml::ContentReference*>(sig->getContentReference());
                        if (cr)
                            cr->setDigestAlgorithm(digestAlg);
                    }
            
                    // Sign message while marshalling.
                    vector<Signature*> sigs(1,sig);
                    rootElement = env->marshall((DOMDocument*)nullptr,&sigs,credential);
                }
            }
            else {
                log.debug("marshalling the envelope");
                rootElement = env->marshall();
            }

            stringstream s;
            s << *rootElement;
            
            if (log.isDebugEnabled())
                log.debug("marshalled envelope:\n%s", s.str().c_str());

            log.debug("sending serialized envelope");
            bool error = (!msg && env->getBody() && env->getBody()->hasChildren() &&
                dynamic_cast<Fault*>(env->getBody()->getUnknownXMLObjects().front()));
            long ret = error ? genericResponse.sendError(s) : genericResponse.sendResponse(s);
        
            // Cleanup by destroying XML.
            delete env;
            return ret;
        }
        catch (XMLToolingException&) {
            if (msg && detachOnFailure) {
                // A bit weird...we have to "revert" things so that the message is isolated
                // so the caller can free it.
                if (msg->getParent()) {
                    msg->getParent()->detach();
                    msg->detach();
                }
            }
            throw;
        }
    }

    Fault* fault = dynamic_cast<Fault*>(xmlObject);
    if (fault) {
        try {
            log.debug("building envelope and marshalling fault");
            Envelope* env = EnvelopeBuilder::buildEnvelope();
            Body* body = BodyBuilder::buildBody();
            env->setBody(body);
            body->getUnknownXMLObjects().push_back(fault);
            rootElement = env->marshall();
    
            stringstream s;
            s << *rootElement;
            
            if (log.isDebugEnabled())
                log.debug("marshalled envelope:\n%s", s.str().c_str());

            log.debug("sending serialized envelope");
            long ret = genericResponse.sendError(s);
        
            // Cleanup by destroying XML.
            delete env;
            return ret;
        }
        catch (XMLToolingException&) {
            // A bit weird...we have to "revert" things so that the fault is isolated
            // so the caller can free it.
            if (fault->getParent()) {
                fault->getParent()->detach();
                fault->detach();
            }
            throw;
        }
    }

    throw BindingException("XML content for SAML 2.0 SOAP Encoder must be a SAML 2.0 message or SOAP Fault/Envelope.");
}
