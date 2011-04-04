/*
 *  Copyright 2001-2010 Internet2
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

#include "internal.h"
#include <saml/saml2/core/Protocols.h>

using namespace opensaml::saml2p;

class StatusCode20Test : public CxxTest::TestSuite, public SAMLObjectBaseTestCase {
    XMLCh* Value;

public:
    void setUp() {
        Value=XMLString::transcode("urn:string");
        singleElementFile = data_path + "saml2/core/impl/StatusCode.xml";
        childElementsFile  = data_path + "saml2/core/impl/StatusCodeChildElements.xml";    
        SAMLObjectBaseTestCase::setUp();
    }
    
    void tearDown() {
        XMLString::release(&Value);
        SAMLObjectBaseTestCase::tearDown();
    }

    void testSingleElementUnmarshall() {
        auto_ptr<XMLObject> xo(unmarshallElement(singleElementFile));
        StatusCode* sc = dynamic_cast<StatusCode*>(xo.get());
        TS_ASSERT(sc!=nullptr);
        assertEquals("Value attribute", Value, sc->getValue());
        TSM_ASSERT("StatusCode child element", sc->getStatusCode()==nullptr);
    }

    void testChildElementsUnmarshall() {
        auto_ptr<XMLObject> xo(unmarshallElement(childElementsFile));
        StatusCode* sc = dynamic_cast<StatusCode*>(xo.get());
        TS_ASSERT(sc!=nullptr);
        TSM_ASSERT("StatusCode child element", sc->getStatusCode()!=nullptr);
    }

    void testSingleElementMarshall() {
        StatusCode* sc=StatusCodeBuilder::buildStatusCode();
        sc->setValue(Value);
        assertEquals(expectedDOM, sc);
    }

    void testChildElementsMarshall() {
        StatusCode* sc=StatusCodeBuilder::buildStatusCode();
        StatusCode* scChild=StatusCodeBuilder::buildStatusCode();
        sc->setStatusCode(scChild);
        assertEquals(expectedChildElementsDOM, sc);
    }

};
