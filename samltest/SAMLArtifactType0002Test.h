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
#include <saml/SAMLConfig.h>
#include <saml/saml1/binding/SAMLArtifactType0002.h>

using namespace opensaml::saml1p;
using namespace opensaml;
using namespace std;

class SAMLArtifactType0002Test : public CxxTest::TestSuite
{
public:
    string providerIdStr;

    void setUp() {
        providerIdStr = "https://idp.org/SAML";
    }
    
    void testSAMLArtifactType0002(void) {
        auto_ptr<SAMLArtifactType0002> artifact(new SAMLArtifactType0002(providerIdStr));
        auto_ptr<SAMLArtifact> tempArtifact(SAMLArtifact::parse(artifact->encode().c_str()));
        
        TS_ASSERT_EQUALS(artifact->getSource(),tempArtifact->getSource());
        TS_ASSERT_EQUALS(artifact->getMessageHandle(),tempArtifact->getMessageHandle());

        TS_ASSERT_THROWS(auto_ptr<SAMLArtifact> bogus1(new SAMLArtifactType0002(providerIdStr, artifact->getMessageHandle() + artifact->getMessageHandle())), ArtifactException);
    }
};
