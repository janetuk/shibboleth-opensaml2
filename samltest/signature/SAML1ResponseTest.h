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

#include "signature/SAMLSignatureTestBase.h"

#include <saml/saml1/core/Protocols.h>

#include <fstream>

using namespace opensaml::saml1p;
using namespace opensaml::saml1;

class SAML1ResponseTest : public CxxTest::TestSuite, public SAMLSignatureTestBase {
public:
    void setUp() {
        childElementsFile  = data_path + "signature/SAML1Response.xml";
        SAMLSignatureTestBase::setUp();
    }

    void tearDown() {
        SAMLSignatureTestBase::tearDown();
    }

    void testSignature() {
        auto_ptr_XMLCh issuer("issuer");
        auto_ptr_XMLCh issueInstant("1970-01-02T01:01:02.100Z");
        auto_ptr_XMLCh aid("aident");
        auto_ptr_XMLCh rid("rident");
        auto_ptr_XMLCh method("method");
        auto_ptr_XMLCh nameid("John Doe");
        
        NameIdentifier* n=NameIdentifierBuilder::buildNameIdentifier();
        n->setName(nameid.get());        
        Subject* subject=SubjectBuilder::buildSubject();
        subject->setNameIdentifier(n);

        AuthenticationStatement* statement=AuthenticationStatementBuilder::buildAuthenticationStatement();
        statement->setAuthenticationInstant(issueInstant.get());
        statement->setAuthenticationMethod(method.get());
        statement->setSubject(subject);
        
        Assertion* assertion=AssertionBuilder::buildAssertion();
        assertion->setAssertionID(aid.get());
        assertion->setIssueInstant(issueInstant.get());
        assertion->setIssuer(issuer.get());
        assertion->getAuthenticationStatements().push_back(statement);

        // Append a Signature.
        assertion->setSignature(SignatureBuilder::buildSignature());
        assertion->getSignature()->setSigningKey(m_key->clone());

        // Build KeyInfo.
        KeyInfo* keyInfo=KeyInfoBuilder::buildKeyInfo();
        X509Data* x509Data=X509DataBuilder::buildX509Data();
        keyInfo->getX509Datas().push_back(x509Data);
        for_each(m_certs.begin(),m_certs.end(),bind1st(_addcert(),x509Data));
        assertion->getSignature()->setKeyInfo(keyInfo);

        // Sign assertion while marshalling.
        vector<Signature*> sigs(1,assertion->getSignature());
        DOMElement* rootElement = NULL;
        try {
            rootElement=assertion->marshall((DOMDocument*)NULL,&sigs);
        }
        catch (XMLToolingException& e) {
            TS_TRACE(e.what());
            delete assertion;
            throw;
        }

        StatusCode* sc=StatusCodeBuilder::buildStatusCode();
        sc->setValue(&StatusCode::SUCCESS);
        Status* status=StatusBuilder::buildStatus();
        status->setStatusCode(sc);

        auto_ptr<Response> response(ResponseBuilder::buildResponse());
        response->setResponseID(rid.get());
        response->setIssueInstant(issueInstant.get());
        response->setStatus(status);
        response->getAssertions().push_back(assertion);
        response->setSignature(SignatureBuilder::buildSignature());
        response->getSignature()->setSigningKey(m_key->clone());
        response->getSignature()->setKeyInfo(keyInfo->cloneKeyInfo());

        // Sign response while marshalling.
        sigs.clear();
        sigs.push_back(response->getSignature());
        rootElement = NULL;
        try {
            rootElement=response->marshall((DOMDocument*)NULL,&sigs);
        }
        catch (XMLToolingException& e) {
            TS_TRACE(e.what());
            throw;
        }

        string buf;
        XMLHelper::serialize(rootElement, buf);
        istringstream in(buf);
        DOMDocument* doc=XMLToolingConfig::getConfig().getParser().parse(in);
        const XMLObjectBuilder* b = XMLObjectBuilder::getBuilder(doc->getDocumentElement());
        
        assertEquals(expectedChildElementsDOM, b->buildFromDocument(doc));
        
        try {
            assertion->getSignature()->registerValidator(new SignatureProfileValidator());
            assertion->getSignature()->registerValidator(new SignatureValidator(new KeyResolver(m_key->clone())));
            assertion->getSignature()->validate(true);
            response->getSignature()->registerValidator(new SignatureProfileValidator());
            response->getSignature()->registerValidator(new SignatureValidator(new KeyResolver(m_key->clone())));
            response->getSignature()->validate(true);
        }
        catch (XMLToolingException& e) {
            TS_TRACE(e.what());
            throw;
        }
    }

};