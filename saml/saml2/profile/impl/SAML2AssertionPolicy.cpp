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
 * SAML2AssertionPolicy.cpp
 *
 * Policy subclass to track SAML 2.0 Assertion SubjectConfirmation.
 */

#include "internal.h"
#include "saml2/profile/SAML2AssertionPolicy.h"

using namespace opensaml::saml2md;
using namespace opensaml::saml2;
using namespace opensaml;
using namespace xmltooling;

SAML2AssertionPolicy::SAML2AssertionPolicy(
    const MetadataProvider* metadataProvider, const xmltooling::QName* role, const TrustEngine* trustEngine, bool validate
    ) : SecurityPolicy(metadataProvider, role, trustEngine, validate), m_confirmation(nullptr)
{
}

SAML2AssertionPolicy::~SAML2AssertionPolicy()
{
}

void SAML2AssertionPolicy::reset(bool messageOnly)
{
    SecurityPolicy::reset(messageOnly);
    SAML2AssertionPolicy::_reset(messageOnly);
}

void SAML2AssertionPolicy::_reset(bool messageOnly)
{
    m_confirmation = nullptr;
}

const SubjectConfirmation* SAML2AssertionPolicy::getSubjectConfirmation() const
{
    return m_confirmation;
}

void SAML2AssertionPolicy::setSubjectConfirmation(const SubjectConfirmation* confirmation)
{
    m_confirmation = confirmation;
}
