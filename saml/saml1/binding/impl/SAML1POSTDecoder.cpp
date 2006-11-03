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
 * SAML1POSTDecoder.cpp
 * 
 * SAML 1.x POST binding/profile message decoder
 */

#include "internal.h"
#include "exceptions.h"
#include "binding/HTTPRequest.h"
#include "saml1/core/Assertions.h"
#include "saml1/binding/SAML1POSTDecoder.h"
#include "saml2/metadata/Metadata.h"
#include "saml2/metadata/MetadataProvider.h"

#include <log4cpp/Category.hh>
#include <xercesc/util/Base64.hpp>
#include <xmltooling/util/NDC.h>
#include <xmltooling/validation/ValidatorSuite.h>

using namespace opensaml::saml2md;
using namespace opensaml::saml1p;
using namespace opensaml::saml1;
using namespace opensaml;
using namespace xmltooling;
using namespace log4cpp;
using namespace std;

namespace opensaml {
    namespace saml1p {              
        MessageDecoder* SAML_DLLLOCAL SAML1POSTDecoderFactory(const DOMElement* const & e)
        {
            return new SAML1POSTDecoder(e);
        }
    };
};

SAML1POSTDecoder::SAML1POSTDecoder(const DOMElement* e) {}

SAML1POSTDecoder::~SAML1POSTDecoder() {}

Response* SAML1POSTDecoder::decode(
    string& relayState,
    const GenericRequest& genericRequest,
    SecurityPolicy& policy
    ) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("decode");
#endif
    Category& log = Category::getInstance(SAML_LOGCAT".MessageDecoder.SAML1POST");

    log.debug("validating input");
    const HTTPRequest* httpRequest=dynamic_cast<const HTTPRequest*>(&genericRequest);
    if (!httpRequest) {
        log.error("unable to cast request to HTTPRequest type");
        return NULL;
    }
    if (strcmp(httpRequest->getMethod(),"POST"))
        return NULL;
    const char* samlResponse = httpRequest->getParameter("SAMLResponse");
    const char* TARGET = httpRequest->getParameter("TARGET");
    if (!samlResponse || !TARGET)
        return NULL;
    relayState = TARGET;

    // Decode the base64 into SAML.
    unsigned int x;
    XMLByte* decoded=Base64::decode(reinterpret_cast<const XMLByte*>(samlResponse),&x);
    if (!decoded)
        throw BindingException("Unable to decode base64 in POST profile response.");
    log.debug("decoded SAML response:\n%s", decoded);
    istringstream is(reinterpret_cast<char*>(decoded));
    XMLString::release(&decoded);
    
    // Parse and bind the document into an XMLObject.
    DOMDocument* doc = (m_validate ? XMLToolingConfig::getConfig().getValidatingParser()
        : XMLToolingConfig::getConfig().getParser()).parse(is); 
    XercesJanitor<DOMDocument> janitor(doc);
    auto_ptr<XMLObject> xmlObject(XMLObjectBuilder::buildOneFromElement(doc->getDocumentElement(), true));
    janitor.release();

    Response* response = dynamic_cast<Response*>(xmlObject.get());
    if (!response)
        throw BindingException("Decoded message was not a SAML 1.x Response.");

    try {
        if (!m_validate)
            SchemaValidators.validate(xmlObject.get());
        
        // Run through the policy.
        policy.evaluate(genericRequest, *response);
    }
    catch (XMLToolingException& ex) {
        // This is just to maximize the likelihood of attaching a source to the message for support purposes.
        if (policy.getIssuerMetadata())
            annotateException(&ex,policy.getIssuerMetadata()); // throws it
          
        // Check for an Issuer.
        const EntityDescriptor* provider=NULL;
        const vector<Assertion*>& assertions=const_cast<const Response*>(response)->getAssertions();
        if (!assertions.empty() || !policy.getMetadataProvider() ||
                !(provider=policy.getMetadataProvider()->getEntityDescriptor(assertions.front()->getIssuer(), false))) {
            // Just record it.
            auto_ptr_char iname(assertions.front()->getIssuer());
            if (iname.get())
                ex.addProperty("entityID", iname.get());
            throw;
        }
        if (policy.getRole()) {
            pair<bool,int> minor = response->getMinorVersion();
            const RoleDescriptor* roledesc=provider->getRoleDescriptor(
                *(policy.getRole()),
                (minor.first && minor.second==0) ? samlconstants::SAML10_PROTOCOL_ENUM : samlconstants::SAML11_PROTOCOL_ENUM
                );
            if (roledesc) annotateException(&ex,roledesc); // throws it
        }
        annotateException(&ex,provider);  // throws it
    }

    xmlObject.release();
    return response;
}