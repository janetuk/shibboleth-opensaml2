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
 * @file saml/security/AbstractPKIXTrustEngine.h
 * 
 * A trust engine that uses X.509 trust anchors and CRLs associated with a role
 * to perform PKIX validation of signatures and certificates.
 */

#ifndef __saml_pkixtrust_h__
#define __saml_pkixtrust_h__

#include <saml/security/X509TrustEngine.h>
#include <xmltooling/security/XSECCryptoX509CRL.h>

namespace opensaml {

    /**
     * A trust engine that uses X.509 trust anchors and CRLs associated with a role
     * to perform PKIX validation of signatures and certificates.
     */
    class SAML_API AbstractPKIXTrustEngine : public X509TrustEngine
    {
    protected:
        /**
         * Constructor.
         * 
         * If a DOM is supplied, the following XML content is supported:
         * 
         * <ul>
         *  <li>&lt;KeyResolver&gt; elements with a type attribute
         * </ul>
         * 
         * XML namespaces are ignored in the processing of this content.
         * 
         * @param e DOM to supply configuration for provider
         */
        AbstractPKIXTrustEngine(const DOMElement* e=NULL);
        
        /**
         * Checks that either the ID for the entity with the given role or the key names
         * for the given role match the subject or subject alternate names
         * of the entity's certificate.
         * 
         * @param certEE    the credential for the entity to validate
         * @param role      the descriptor of the role the entity is supposed to be acting in
         * 
         * @return true the name check succeeds, false if not
         */
        bool checkEntityNames(XSECCryptoX509* certEE, const saml2md::RoleDescriptor& role) const;
        
        /** An inline KeyResolver for extracting certificates out of a signature. */
        xmlsignature::KeyResolver* m_inlineResolver;
        
    public:
        virtual ~AbstractPKIXTrustEngine();

        virtual bool validate(
            xmlsignature::Signature& sig,
            const saml2md::RoleDescriptor& role,
            const xmlsignature::KeyResolver* keyResolver=NULL
            );

        virtual bool validate(
            XSECCryptoX509* certEE,
            const std::vector<XSECCryptoX509*>& certChain,
            const saml2md::RoleDescriptor& role,
            bool checkName=true,
            const xmlsignature::KeyResolver* keyResolver=NULL
            );

        /**
         * Stateful interface that supplies PKIX validation data to the trust engine.
         * Applications can adapt this TrustEngine to their environment by returning
         * implementations of this interface from the getPKIXValidationInfoIterator
         * method.
         */
        class SAML_API PKIXValidationInfoIterator {
            MAKE_NONCOPYABLE(PKIXValidationInfoIterator);
        protected:
            PKIXValidationInfoIterator() {}
        public:
            virtual ~PKIXValidationInfoIterator() {}
            
            /**
             * Advances to the next set of information, if any.
             * 
             * @return true iff another set of information is available
             */
            virtual bool next()=0;
            
            /**
             * Returns the allowable trust chain verification depth for the
             * validation data in the current position.
             * 
             * @return  allowable trust chain verification depth
             */
            virtual int getVerificationDepth() const=0;
            
            /**
             * Returns the set of trust anchors for the validation data in the
             * current position. Keeping the certificates beyond the lifetime
             * of the iterator or after advancing to the next position requires
             * copying them.
             * 
             * @return  set of trust anchors
             */
            virtual const std::vector<XSECCryptoX509*>& getTrustAnchors() const=0;

            /**
             * Returns the set of CRLs for the validation data in the
             * current position. Keeping the CRLs beyond the lifetime
             * of the iterator or after advancing to the next position requires
             * copying them.
             * 
             * @return  set of CRLs
             */
            virtual const std::vector<xmltooling::XSECCryptoX509CRL*>& getCRLs() const=0;
        };
        
        /**
         * Provides access to the information necessary, for the given role, for
         * PKIX validation of credentials. Each set of validation information returned
         * will be tried, in turn, until one succeeds or no more remain.
         * The caller must free the returned interface when finished with it.
         * 
         * @return interface for obtaining validation data  
         */
        virtual PKIXValidationInfoIterator* getPKIXValidationInfoIterator(const saml2md::RoleDescriptor& role) const=0;
    };
};

#endif /* __saml_pkixtrust_h__ */