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

#include "internal.h"
#include <saml/saml2/core/Protocols.h>
#include <saml/util/SAMLConstants.h>

using namespace opensaml::saml2p;
using namespace opensaml::saml2;

class AuthzDecisionQuery20Test : public CxxTest::TestSuite, public SAMLObjectBaseTestCase {
    XMLCh* expectedID; 
    XMLCh* expectedVersion; 
    XMLCh* expectedConsent; 
    XMLCh* expectedDestination; 
    DateTime* expectedIssueInstant; 
    XMLCh* expectedResource; 

public:
    void setUp() {
        expectedID = XMLString::transcode("abc123");; 
        expectedVersion = XMLString::transcode("2.0"); 
        expectedConsent = XMLString::transcode("urn:string:consent"); 
        expectedDestination = XMLString::transcode("http://idp.example.org/endpoint"); 
        expectedIssueInstant = new DateTime(XMLString::transcode("2006-02-21T16:40:00.000Z"));
        expectedIssueInstant->parseDateTime();
        expectedResource = XMLString::transcode("urn:string:resource");

        singleElementFile = data_path + "saml2/core/impl/AuthzDecisionQuery.xml";
        singleElementOptionalAttributesFile = data_path + "saml2/core/impl/AuthzDecisionQueryOptionalAttributes.xml";
        childElementsFile  = data_path + "saml2/core/impl/AuthzDecisionQueryChildElements.xml";    
        SAMLObjectBaseTestCase::setUp();
    }
    
    void tearDown() {
        XMLString::release(&expectedID);
        XMLString::release(&expectedVersion);
        XMLString::release(&expectedConsent);
        XMLString::release(&expectedDestination);
        XMLString::release(&expectedResource);
        delete expectedIssueInstant;
        SAMLObjectBaseTestCase::tearDown();
    }

    void testSingleElementUnmarshall() {
        auto_ptr<XMLObject> xo(unmarshallElement(singleElementFile));
        AuthzDecisionQuery* query = dynamic_cast<AuthzDecisionQuery*>(xo.get());
        TS_ASSERT(query!=nullptr);
        assertEquals("ID attribute", expectedID, query->getID());
        assertEquals("Version attribute", expectedVersion, query->getVersion());
        TSM_ASSERT_EQUALS("IssueInstant attribute", expectedIssueInstant->getEpoch(), query->getIssueInstant()->getEpoch());
        assertEquals("Resource attribute", expectedResource, query->getResource());

        TS_ASSERT(query->getIssuer()==nullptr);
        TS_ASSERT(query->getSignature()==nullptr);
        TS_ASSERT(query->getExtensions()==nullptr);
        TS_ASSERT(query->getSubject()==nullptr);
        TSM_ASSERT_EQUALS("# of Action child elements", 0, query->getActions().size());
        TS_ASSERT(query->getEvidence()==nullptr);
    }

    void testSingleElementOptionalAttributesUnmarshall() {
        auto_ptr<XMLObject> xo(unmarshallElement(singleElementOptionalAttributesFile));
        AuthzDecisionQuery* query = dynamic_cast<AuthzDecisionQuery*>(xo.get());
        TS_ASSERT(query!=nullptr);
        assertEquals("Consent attribute", expectedConsent, query->getConsent());
        assertEquals("Destination attribute", expectedDestination, query->getDestination());

        TS_ASSERT(query->getIssuer()==nullptr);
        TS_ASSERT(query->getSignature()==nullptr);
        TS_ASSERT(query->getExtensions()==nullptr);
        TS_ASSERT(query->getSubject()==nullptr);
        TSM_ASSERT_EQUALS("# of Action child elements", 0, query->getActions().size());
        TS_ASSERT(query->getEvidence()==nullptr);
    }

    void testChildElementsUnmarshall() {
        auto_ptr<XMLObject> xo(unmarshallElement(childElementsFile));
        AuthzDecisionQuery* query= dynamic_cast<AuthzDecisionQuery*>(xo.get());
        TS_ASSERT(query!=nullptr);
        TS_ASSERT(query->getIssuer()!=nullptr);
        TS_ASSERT(query->getSignature()==nullptr);
        TS_ASSERT(query->getExtensions()==nullptr);
        TS_ASSERT(query->getSubject()!=nullptr);
        TSM_ASSERT_EQUALS("# of Action child elements", 2, query->getActions().size());
        TS_ASSERT(query->getEvidence()!=nullptr);
    }

    void testSingleElementMarshall() {
        AuthzDecisionQuery* query=AuthzDecisionQueryBuilder::buildAuthzDecisionQuery();
        query->setID(expectedID);
        query->setIssueInstant(expectedIssueInstant);
        //query->setVersion(expectedVersion);
        query->setResource(expectedResource);
        assertEquals(expectedDOM, query);
    }

    void testSingleElementOptionalAttributesMarshall() {
        AuthzDecisionQuery* query=AuthzDecisionQueryBuilder::buildAuthzDecisionQuery();
        query->setID(expectedID);
        query->setIssueInstant(expectedIssueInstant);
        //query->setVersion(expectedVersion);
        query->setConsent(expectedConsent);
        query->setDestination(expectedDestination);
        query->setResource(expectedResource);
        assertEquals(expectedOptionalAttributesDOM, query);
    }

    void testChildElementsMarshall() {
        AuthzDecisionQuery* query=AuthzDecisionQueryBuilder::buildAuthzDecisionQuery();
        query->setID(expectedID);
        query->setIssueInstant(expectedIssueInstant);
        // Do this just so don't have to redeclare the saml namespace prefix on every child element in the control XML file
        Namespace* ns = new Namespace(samlconstants::SAML20_NS, samlconstants::SAML20_PREFIX);
        query->addNamespace(*ns);
        query->setIssuer(IssuerBuilder::buildIssuer());
        query->setSubject(SubjectBuilder::buildSubject());
        query->getActions().push_back(ActionBuilder::buildAction());
        query->getActions().push_back(ActionBuilder::buildAction());
        query->setEvidence(EvidenceBuilder::buildEvidence());
        assertEquals(expectedChildElementsDOM, query);
        delete ns;
    }

};
