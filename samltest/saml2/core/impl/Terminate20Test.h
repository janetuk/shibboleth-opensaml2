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

using namespace opensaml::saml2p;

class Terminate20Test : public CxxTest::TestSuite, public SAMLObjectBaseTestCase {

public:
    void setUp() {
        singleElementFile = data_path + "saml2/core/impl/Terminate.xml";
        SAMLObjectBaseTestCase::setUp();
    }
    
    void tearDown() {
        SAMLObjectBaseTestCase::tearDown();
    }

    void testSingleElementUnmarshall() {
        auto_ptr<XMLObject> xo(unmarshallElement(singleElementFile));
        Terminate* terminate = dynamic_cast<Terminate*>(xo.get());
        TS_ASSERT(terminate!=nullptr);
    }

    void testSingleElementMarshall() {
        Terminate * terminate = TerminateBuilder::buildTerminate();
        assertEquals(expectedDOM, terminate);
    }


};
