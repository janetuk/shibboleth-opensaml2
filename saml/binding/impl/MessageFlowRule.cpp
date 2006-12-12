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
 * MessageFlowRule.cpp
 * 
 * SAML replay and freshness checking SecurityPolicyRule
 */

#include "internal.h"
#include "exceptions.h"
#include "binding/MessageFlowRule.h"

#include <log4cpp/Category.hh>
#include <xmltooling/util/ReplayCache.h>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace opensaml;
using namespace xmltooling;
using namespace log4cpp;
using namespace std;

namespace opensaml {
    SecurityPolicyRule* SAML_DLLLOCAL MessageFlowRuleFactory(const DOMElement* const & e)
    {
        return new MessageFlowRule(e);
    }
};

static const XMLCh checkReplay[] = UNICODE_LITERAL_11(c,h,e,c,k,R,e,p,l,a,y);
static const XMLCh expires[] = UNICODE_LITERAL_7(e,x,p,i,r,e,s);

MessageFlowRule::MessageFlowRule(const DOMElement* e)
    : m_checkReplay(true), m_expires(XMLToolingConfig::getConfig().clock_skew_secs)
{
    if (e) {
        const XMLCh* attr = e->getAttributeNS(NULL, checkReplay);
        if (attr && (*attr==chLatin_f || *attr==chDigit_0))
            m_checkReplay = false;
        attr = e->getAttributeNS(NULL, expires);
        if (attr)
            m_expires = XMLString::parseInt(attr);
    }
}

void MessageFlowRule::evaluate(const XMLObject& message, const GenericRequest* request, SecurityPolicy& policy) const
{
    Category& log=Category::getInstance(SAML_LOGCAT".SecurityPolicyRule.MessageFlow");
    log.debug("evaluating message flow policy (replay checking %s, expiration %lu)", m_checkReplay ? "on" : "off", m_expires);

    time_t issueInstant = policy.getIssueInstant();
    if (issueInstant == 0) {
        log.error("unknown message timestamp");
        throw BindingException("Message rejected, no timestamp available.");
    }
    
    time_t skew = XMLToolingConfig::getConfig().clock_skew_secs;
    time_t now = time(NULL);
    if (issueInstant > now + skew) {
        log.errorStream() << "rejected not-yet-valid message, timestamp (" << issueInstant <<
            "), newest allowed (" << now + skew << ")" << CategoryStream::ENDLINE;
        throw BindingException("Message rejected, was issued in the future.");
    }
    else if (issueInstant < now - skew - m_expires) {
        log.errorStream() << "rejected expired message, timestamp (" << issueInstant <<
            "), oldest allowed (" << (now - skew - m_expires) << ")" << CategoryStream::ENDLINE;
        throw BindingException("Message expired, was issued too long ago.");
    }
    
    // Check replay.
    if (m_checkReplay) {
        const XMLCh* id = policy.getMessageID();
        ReplayCache* replayCache = XMLToolingConfig::getConfig().getReplayCache();
        if (!replayCache)
            throw BindingException("Message rejected, no ReplayCache instance available.");
        else if (!id)
            throw BindingException("Message rejected, did not contain an identifier.");
        auto_ptr_char temp(id);
        if (!replayCache->check("SAML", temp.get(), issueInstant + skew + m_expires)) {
            log.error("replay detected of message ID (%s)", temp.get());
            throw BindingException("Rejecting replayed message ID ($1).", params(1,temp.get()));
        }
    }
}
