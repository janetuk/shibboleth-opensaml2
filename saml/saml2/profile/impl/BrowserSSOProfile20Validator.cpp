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
 * BrowserSSOProfile20Validator.cpp
 *
 * SAML 2.0 Browser SSO Profile Assertion Validator.
 */

#include "internal.h"
#include "saml2/core/Assertions.h"
#include "saml2/profile/BrowserSSOProfileValidator.h"

#include <xmltooling/logging.h>
#include <xmltooling/XMLToolingConfig.h>
#include <xmltooling/util/NDC.h>

using namespace opensaml::saml2;
using namespace xmltooling::logging;
using namespace xmltooling;
using namespace std;

BrowserSSOProfileValidator::BrowserSSOProfileValidator(
    const XMLCh* recipient,
    const vector<const XMLCh*>* audiences,
    time_t ts,
    const char* destination,
    const char* requestID
    ) : AssertionValidator(recipient, audiences, ts), m_destination(destination), m_requestID(requestID)
{
}

BrowserSSOProfileValidator::~BrowserSSOProfileValidator()
{
}

void BrowserSSOProfileValidator::validateAssertion(const Assertion& assertion) const
{
#ifdef _DEBUG
    xmltooling::NDC ndc("validate");
#endif
    Category& log = Category::getInstance(SAML_LOGCAT".AssertionValidator");

    // The assertion MUST have proper confirmation requirements.
    const char* msg="assertion is missing bearer SubjectConfirmation";
    const Subject* subject = assertion.getSubject();
    if (subject) {
        const vector<SubjectConfirmation*>& confs = subject->getSubjectConfirmations();
        for (vector<SubjectConfirmation*>::const_iterator sc = confs.begin(); sc!=confs.end(); ++sc) {
            if (XMLString::equals((*sc)->getMethod(), SubjectConfirmation::BEARER)) {
                const SubjectConfirmationDataType* data = dynamic_cast<const SubjectConfirmationDataType*>((*sc)->getSubjectConfirmationData());

                if (m_destination.get()) {
                    if (!XMLString::equals(m_destination.get(), data ? data->getRecipient() : nullptr)) {
                        msg = "bearer confirmation failed with recipient mismatch";
                        continue;
                    }
                }

                if (m_requestID.get()) {
                    if (!XMLString::equals(m_requestID.get(), data ? data->getInResponseTo() : nullptr)) {
                        msg = "bearer confirmation failed with request correlation mismatch";
                        continue;
                    }
                }

                if (m_ts) {
                    if (!data || !data->getNotOnOrAfter()) {
                        msg = "bearer confirmation missing NotOnOrAfter attribute";
                        continue;
                    }
                    else if (data->getNotOnOrAfterEpoch() <= m_ts - XMLToolingConfig::getConfig().clock_skew_secs) {
                        msg = "bearer confirmation has expired";
                        continue;
                    }
                }

                // Save off client address.
                if (data) {
                    auto_ptr_char ip(data->getAddress());
                    if (ip.get())
                        m_address = ip.get();
                }

                // Pass up for additional checking.
                return AssertionValidator::validateAssertion(assertion);
            }
        }
    }

    log.error(msg ? msg : "no error message");
    throw ValidationException("Unable to locate satisfiable bearer SubjectConfirmation in assertion.");
}

const char* BrowserSSOProfileValidator::getAddress() const
{
    return m_address.c_str();
}
