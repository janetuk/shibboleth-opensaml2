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
 * @file saml/binding/SecurityPolicyRule.h
 * 
 * Policy rules that secure and authenticate bindings.
 */

#ifndef __saml_secrule_h__
#define __saml_secrule_h__

#include <saml/binding/GenericRequest.h>
#include <xmltooling/XMLObject.h>

namespace opensaml {
    class SAML_API TrustEngine;
    
    namespace saml2 {
        class SAML_API Issuer;
    };
    namespace saml2md {
        class SAML_API MetadataProvider;
        class SAML_API RoleDescriptor;
    };
    
    /**
     * A rule that a protocol request and message must meet in order to be valid and secure.
     * 
     * <p>Rules must be stateless and thread-safe across evaluations. Evaluation should not
     * result in an exception if the request/message properties do not apply to the rule
     * (e.g. particular security mechanisms that are not present). 
     */
    class SAML_API SecurityPolicyRule
    {
        MAKE_NONCOPYABLE(SecurityPolicyRule);
    protected:
        SecurityPolicyRule() {}
    public:
        virtual ~SecurityPolicyRule() {}

        /** Allows override of code for extracting saml2:Issuer and protocol information. */
        class SAML_API MessageExtractor {
            MAKE_NONCOPYABLE(MessageExtractor);
        public:
            MessageExtractor() {}
            virtual ~MessageExtractor() {}
            
            /**
             * Examines the message and/or its contents and extracts the issuer's claimed
             * identity along with a protocol identifier. Conventions may be needed
             * to properly encode non-SAML2 issuer information into a compatible form. 
             * 
             * <p>The caller is responsible for freeing the Issuer object.
             * 
             * @param message       message to examine
             * @return  a pair consisting of a SAML 2.0 Issuer object and a protocol constant.
             * @throws std::bad_cast thrown if the message is not of an expected type
             */
            virtual std::pair<saml2::Issuer*,const XMLCh*> getIssuerAndProtocol(const xmltooling::XMLObject& message) const;
        };

        /**
         * Evaluates the rule against the given request and message. If an Issuer is
         * returned, the caller is responsible for freeing the Issuer object.
         * 
         * @param request           the protocol request
         * @param message           the incoming message
         * @param metadataProvider  locked MetadataProvider instance to authenticate the message
         * @param role              identifies the role (generally IdP or SP) of the peer who issued the message 
         * @param trustEngine       TrustEngine to authenticate the message
         * @param extractor         MessageExtractor to use in examining message
         * @return the identity of the message issuer, in two forms, or NULL
         * 
         * @throws BindingException thrown if the request/message do not meet the requirements of this rule
         */
        virtual std::pair<saml2::Issuer*,const saml2md::RoleDescriptor*> evaluate(
            const GenericRequest& request,
            const xmltooling::XMLObject& message,
            const saml2md::MetadataProvider* metadataProvider,
            const xmltooling::QName* role,
            const TrustEngine* trustEngine,
            const MessageExtractor& extractor
            ) const=0;
    };

    /**
     * Registers SecurityPolicyRule plugins into the runtime.
     */
    void SAML_API registerSecurityPolicyRules();

    /**
     * SecurityPolicyRule for TLS client certificate authentication.
     * 
     * Requires that messages carry information about the issuer, and then
     * evaluates the claimed certificates against the issuer's metadata.
     */
    #define CLIENTCERTAUTH_POLICY_RULE  "org.opensaml.binding.ClientCertAuthRule"

    /**
     * SecurityPolicyRule for replay detection and freshness checking.
     * 
     * <p>A ReplayCache instance must be available from the runtime, unless
     * a "checkReplay" XML attribute is set to "0" or "false" when instantiating
     * the policy rule.
     * 
     * <p>Messages must have been issued in the past, but no more than 60 seconds ago,
     * or up to a number of seconds set by an "expires" XML attribute when
     * instantiating the policy rule.
     */
    #define MESSAGEFLOW_POLICY_RULE  "org.opensaml.binding.MessageFlowRule"

    /**
     * SecurityPolicyRule for protocol message "blob" signing.
     * 
     * Allows the message issuer to be authenticated using a non-XML digital signature
     * over the message body. The transport layer is not considered.
     */
    #define SIMPLESIGNING_POLICY_RULE  "org.opensaml.binding.SimpleSigningRule"

    /**
     * SecurityPolicyRule for protocol message XML signing.
     * 
     * Allows the message issuer to be authenticated using an XML digital signature
     * over the message. The transport layer is not considered.
     */
    #define XMLSIGNING_POLICY_RULE  "org.opensaml.binding.XMLSigningRule"
};

#endif /* __saml_secrule_h__ */
