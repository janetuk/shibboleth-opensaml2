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

#include "internal.h"
#include <saml/saml2/core/Protocols.h>

using namespace opensaml::saml2p;

class RequesterID20Test : public CxxTest::TestSuite, public SAMLObjectBaseTestCase {
    XMLCh* expectedContent;

public:
    void setUp() {
        expectedContent=XMLString::transcode("urn:string:requester");
        singleElementFile = data_path + "saml2/core/impl/RequesterID.xml";
        SAMLObjectBaseTestCase::setUp();
    }
    
    void tearDown() {
        XMLString::release(&expectedContent);
        SAMLObjectBaseTestCase::tearDown();
    }

    void testSingleElementUnmarshall() {
        auto_ptr<XMLObject> xo(unmarshallElement(singleElementFile));
        RequesterID* reqid = dynamic_cast<RequesterID*>(xo.get());
        TS_ASSERT(reqid!=NULL);
        assertEquals("RequesterID text content", expectedContent, reqid->getRequesterID());
    }

    void testSingleElementMarshall() {
        RequesterID * reqid = RequesterIDBuilder::buildRequesterID();
        reqid->setRequesterID(expectedContent);
        assertEquals(expectedDOM, reqid);
    }


};
