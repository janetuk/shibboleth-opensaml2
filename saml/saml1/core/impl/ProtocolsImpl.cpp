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
 * ProtocolsImpl.cpp
 *
 * Implementation classes for SAML 1.x Protocols schema.
 */

#include "internal.h"
#include "exceptions.h"
#include "saml1/core/Assertions.h"
#include "saml1/core/Protocols.h"
#include "signature/ContentReference.h"

#include <xmltooling/AbstractComplexElement.h>
#include <xmltooling/AbstractSimpleElement.h>
#include <xmltooling/impl/AnyElement.h>
#include <xmltooling/io/AbstractXMLObjectMarshaller.h>
#include <xmltooling/io/AbstractXMLObjectUnmarshaller.h>
#include <xmltooling/signature/Signature.h>
#include <xmltooling/util/DateTime.h>
#include <xmltooling/util/XMLHelper.h>

#include <ctime>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/if.hpp>
#include <boost/lambda/lambda.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace opensaml::saml1p;
using namespace xmltooling;
using namespace std;
using xmlconstants::XMLSIG_NS;
using xmlconstants::XML_ONE;
using samlconstants::SAML1P_NS;
using samlconstants::SAML1_NS;
using samlconstants::SAML1P_PREFIX;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace opensaml {
    namespace saml1p {

        DECL_XMLOBJECTIMPL_SIMPLE(SAML_DLLLOCAL,AssertionArtifact);
        DECL_XMLOBJECTIMPL_SIMPLE(SAML_DLLLOCAL,StatusMessage);

        class SAML_DLLLOCAL RespondWithImpl : public virtual RespondWith,
            public AbstractSimpleElement,
            public AbstractDOMCachingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
            mutable xmltooling::QName* m_qname;
        public:
            virtual ~RespondWithImpl() {
                delete m_qname;
            }

            RespondWithImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_qname(nullptr) {}

            RespondWithImpl(const RespondWithImpl& src)
                    : AbstractXMLObject(src), AbstractSimpleElement(src), AbstractDOMCachingXMLObject(src), m_qname(nullptr) {
                IMPL_CLONE_ATTRIB(QName);   // not really an attribute, but it gets the job done
            }

            xmltooling::QName* getQName() const {
                if (!m_qname && getDOM() && getDOM()->getTextContent()) {
                    m_qname = XMLHelper::getNodeValueAsQName(getDOM());
                }
                return m_qname;
            }

            void setQName(const xmltooling::QName* qname) {
                m_qname=prepareForAssignment(m_qname,qname);
                if (m_qname) {
                    auto_ptr_XMLCh temp(m_qname->toString().c_str());
                    setTextContent(temp.get());
                }
                else {
                    setTextContent(nullptr);
                }
            }

            IMPL_XMLOBJECT_CLONE(RespondWith);
        };

        class SAML_DLLLOCAL QueryImpl : public virtual Query, public AnyElementImpl
        {
        public:
            virtual ~QueryImpl() {}

            QueryImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {}

            QueryImpl(const QueryImpl& src) : AbstractXMLObject(src), AnyElementImpl(src) {}

            IMPL_XMLOBJECT_CLONE_EX(Query);
        };

        class SAML_DLLLOCAL SubjectQueryImpl : public virtual SubjectQuery,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
            void init() {
                m_Subject=nullptr;
                m_children.push_back(nullptr);
                m_pos_Subject=m_children.begin();
            }

        protected:
            SubjectQueryImpl() {
                init();
            }

        public:
            virtual ~SubjectQueryImpl() {}

            SubjectQueryImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }

            SubjectQueryImpl(const SubjectQueryImpl& src)
                    : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
                init();
            }

            void _clone(const SubjectQueryImpl& src) {
                IMPL_CLONE_TYPED_CHILD(Subject);
            }

            SubjectQuery* cloneSubjectQuery() const {
                return dynamic_cast<SubjectQuery*>(clone());
            }

            Query* cloneQuery() const {
                return dynamic_cast<Query*>(clone());
            }

            IMPL_TYPED_FOREIGN_CHILD(Subject,saml1);

        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_FOREIGN_CHILD(Subject,saml1,SAML1_NS,true);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }
        };

        class SAML_DLLLOCAL AuthenticationQueryImpl : public virtual AuthenticationQuery, public SubjectQueryImpl
        {
            void init() {
                m_AuthenticationMethod=nullptr;
            }

        public:
            virtual ~AuthenticationQueryImpl() {
                XMLString::release(&m_AuthenticationMethod);
            }

            AuthenticationQueryImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                    : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }

            AuthenticationQueryImpl(const AuthenticationQueryImpl& src) : AbstractXMLObject(src), SubjectQueryImpl(src) {
                init();
            }

            void _clone(const AuthenticationQueryImpl& src) {
                SubjectQueryImpl::_clone(src);
                IMPL_CLONE_ATTRIB(AuthenticationMethod);
            }

            IMPL_XMLOBJECT_CLONE_EX(AuthenticationQuery);
            IMPL_STRING_ATTRIB(AuthenticationMethod);

        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_STRING_ATTRIB(AuthenticationMethod,AUTHENTICATIONMETHOD,nullptr);
                SubjectQueryImpl::marshallAttributes(domElement);
            }

            void processAttribute(const DOMAttr* attribute) {
                PROC_STRING_ATTRIB(AuthenticationMethod,AUTHENTICATIONMETHOD,nullptr);
                SubjectQueryImpl::processAttribute(attribute);
            }
        };

        class SAML_DLLLOCAL AttributeQueryImpl : public virtual AttributeQuery, public SubjectQueryImpl
        {
            void init() {
                m_Resource=nullptr;
            }

        public:
            virtual ~AttributeQueryImpl() {
                XMLString::release(&m_Resource);
            }

            AttributeQueryImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                    : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }

            AttributeQueryImpl(const AttributeQueryImpl& src) : AbstractXMLObject(src), SubjectQueryImpl(src) {
                init();
            }

            void _clone(const AttributeQueryImpl& src) {
                SubjectQueryImpl::_clone(src);
                IMPL_CLONE_ATTRIB(Resource);
                IMPL_CLONE_TYPED_FOREIGN_CHILDREN(AttributeDesignator,saml1);
            }

            IMPL_XMLOBJECT_CLONE_EX(AttributeQuery);
            IMPL_STRING_ATTRIB(Resource);
            IMPL_TYPED_FOREIGN_CHILDREN(AttributeDesignator,saml1,m_children.end());

        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_STRING_ATTRIB(Resource,RESOURCE,nullptr);
                SubjectQueryImpl::marshallAttributes(domElement);
            }

            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_FOREIGN_CHILDREN(AttributeDesignator,saml1,SAML1_NS,true);
                SubjectQueryImpl::processChildElement(childXMLObject,root);
            }

            void processAttribute(const DOMAttr* attribute) {
                PROC_STRING_ATTRIB(Resource,RESOURCE,nullptr);
                SubjectQueryImpl::processAttribute(attribute);
            }
        };

        class SAML_DLLLOCAL AuthorizationDecisionQueryImpl : public virtual AuthorizationDecisionQuery, public SubjectQueryImpl
        {
            void init() {
                m_Resource=nullptr;
                m_Evidence=nullptr;
                m_children.push_back(nullptr);
                m_pos_Evidence=m_pos_Subject;
                ++m_pos_Evidence;
            }

        public:
            virtual ~AuthorizationDecisionQueryImpl() {
                XMLString::release(&m_Resource);
            }

            AuthorizationDecisionQueryImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                    : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }

            AuthorizationDecisionQueryImpl(const AuthorizationDecisionQueryImpl& src) : AbstractXMLObject(src), SubjectQueryImpl(src) {
                init();
            }

            void _clone(const AuthorizationDecisionQueryImpl& src) {
                SubjectQueryImpl::_clone(src);
                IMPL_CLONE_ATTRIB(Resource);
                IMPL_CLONE_TYPED_FOREIGN_CHILDREN(Action,saml1);
                IMPL_CLONE_TYPED_CHILD(Evidence);
            }

            IMPL_XMLOBJECT_CLONE_EX(AuthorizationDecisionQuery);
            IMPL_STRING_ATTRIB(Resource);
            IMPL_TYPED_FOREIGN_CHILDREN(Action,saml1,m_pos_Evidence);
            IMPL_TYPED_FOREIGN_CHILD(Evidence,saml1);

        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_STRING_ATTRIB(Resource,RESOURCE,nullptr);
                SubjectQueryImpl::marshallAttributes(domElement);
            }

            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_FOREIGN_CHILD(Evidence,saml1,SAML1_NS,false);
                PROC_TYPED_FOREIGN_CHILDREN(Action,saml1,SAML1_NS,false);
                SubjectQueryImpl::processChildElement(childXMLObject,root);
            }

            void processAttribute(const DOMAttr* attribute) {
                PROC_STRING_ATTRIB(Resource,RESOURCE,nullptr);
                SubjectQueryImpl::processAttribute(attribute);
            }
        };

        class SAML_DLLLOCAL RequestAbstractTypeImpl : public virtual RequestAbstractType,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
            void init() {
                m_MinorVersion=nullptr;
                m_RequestID=nullptr;
                m_IssueInstant=nullptr;
                m_children.push_back(nullptr);
                m_Signature=nullptr;
                m_pos_Signature=m_children.begin();
            }

        protected:
            RequestAbstractTypeImpl() {
                init();
            }
        public:
            virtual ~RequestAbstractTypeImpl() {
                XMLString::release(&m_MinorVersion);
                XMLString::release(&m_RequestID);
                delete m_IssueInstant;
            }

            RequestAbstractTypeImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }

            RequestAbstractTypeImpl(const RequestAbstractTypeImpl& src)
                    : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
                init();
            }

            //IMPL_TYPED_CHILD(Signature);
            // Need customized setter.

        protected:
            void _clone(const RequestAbstractTypeImpl& src) {
                IMPL_CLONE_INTEGER_ATTRIB(MinorVersion);
                IMPL_CLONE_ATTRIB(RequestID);
                IMPL_CLONE_ATTRIB(IssueInstant);
                IMPL_CLONE_TYPED_CHILD(Signature);
                IMPL_CLONE_TYPED_CHILDREN(RespondWith);
            }

            xmlsignature::Signature* m_Signature;
            list<XMLObject*>::iterator m_pos_Signature;

        public:
            xmlsignature::Signature* getSignature() const {
                return m_Signature;
            }

            void setSignature(xmlsignature::Signature* sig) {
                prepareForAssignment(m_Signature,sig);
                *m_pos_Signature=m_Signature=sig;
                // Sync content reference back up.
                if (m_Signature)
                    m_Signature->setContentReference(new opensaml::ContentReference(*this));
            }

            RequestAbstractType* cloneRequestAbstractType() const {
                return dynamic_cast<RequestAbstractType*>(clone());
            }

            IMPL_INTEGER_ATTRIB(MinorVersion);
            IMPL_STRING_ATTRIB(RequestID);    // have to special-case getXMLID
            const XMLCh* getXMLID() const {
                pair<bool,int> v = getMinorVersion();
                return (!v.first || v.second > 0) ? m_RequestID : nullptr;
            }
            const XMLCh* getID() const {
                return getRequestID();
            }
            void releaseDOM() const {
                if (getDOM())
                    getDOM()->removeAttributeNS(nullptr, REQUESTID_ATTRIB_NAME);
                AbstractDOMCachingXMLObject::releaseDOM();
            }
            IMPL_DATETIME_ATTRIB(IssueInstant,0);
            IMPL_TYPED_CHILDREN(RespondWith,m_pos_Signature);

        protected:
            void prepareForMarshalling() const {
                if (m_Signature)
                    declareNonVisibleNamespaces();
            }

            void marshallAttributes(DOMElement* domElement) const {
                static const XMLCh MAJORVERSION[] = UNICODE_LITERAL_12(M,a,j,o,r,V,e,r,s,i,o,n);
                domElement->setAttributeNS(nullptr,MAJORVERSION,XML_ONE);
                if (!m_MinorVersion)
                    const_cast<RequestAbstractTypeImpl*>(this)->m_MinorVersion=XMLString::replicate(XML_ONE);
                MARSHALL_INTEGER_ATTRIB(MinorVersion,MINORVERSION,nullptr);
                if (!m_RequestID)
                    const_cast<RequestAbstractTypeImpl*>(this)->m_RequestID=SAMLConfig::getConfig().generateIdentifier();
                domElement->setAttributeNS(nullptr, REQUESTID_ATTRIB_NAME, m_RequestID);
                if (*m_MinorVersion!=chDigit_0) {
#ifdef XMLTOOLING_XERCESC_BOOLSETIDATTRIBUTE
                    domElement->setIdAttributeNS(nullptr, REQUESTID_ATTRIB_NAME, true);
#else
                    domElement->setIdAttributeNS(nullptr, REQUESTID_ATTRIB_NAME);
#endif
                }
                if (!m_IssueInstant) {
                    const_cast<RequestAbstractTypeImpl*>(this)->m_IssueInstantEpoch=time(nullptr);
                    const_cast<RequestAbstractTypeImpl*>(this)->m_IssueInstant=new DateTime(m_IssueInstantEpoch);
                }
                MARSHALL_DATETIME_ATTRIB(IssueInstant,ISSUEINSTANT,nullptr);
            }

            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILDREN(RespondWith,SAML1P_NS,false);
                PROC_TYPED_FOREIGN_CHILD(Signature,xmlsignature,XMLSIG_NS,false);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }

            void unmarshallAttributes(const DOMElement* domElement) {
                // Standard processing, but then we check IDness.
                AbstractXMLObjectUnmarshaller::unmarshallAttributes(domElement);
                if (m_RequestID && (!m_MinorVersion || *m_MinorVersion!=chDigit_0)) {
#ifdef XMLTOOLING_XERCESC_BOOLSETIDATTRIBUTE
                    const_cast<DOMElement*>(domElement)->setIdAttributeNS(nullptr, REQUESTID_ATTRIB_NAME, true);
#else
                    const_cast<DOMElement*>(domElement)->setIdAttributeNS(nullptr, REQUESTID_ATTRIB_NAME);
#endif
                }
            }

            void processAttribute(const DOMAttr* attribute) {
                static const XMLCh MAJORVERSION[] = UNICODE_LITERAL_12(M,a,j,o,r,V,e,r,s,i,o,n);
                if (XMLHelper::isNodeNamed(attribute,nullptr,MAJORVERSION)) {
                    if (!XMLString::equals(attribute->getValue(),XML_ONE))
                        throw UnmarshallingException("Request has invalid major version.");
                }
                PROC_INTEGER_ATTRIB(MinorVersion,MINORVERSION,nullptr);
                PROC_STRING_ATTRIB(RequestID,REQUESTID,nullptr);
                PROC_DATETIME_ATTRIB(IssueInstant,ISSUEINSTANT,nullptr);
            }
        };

        class SAML_DLLLOCAL RequestImpl : public virtual Request, public RequestAbstractTypeImpl
        {
            void init() {
                m_children.push_back(nullptr);
                m_Query=nullptr;
                m_pos_Query=m_pos_Signature;
                ++m_pos_Query;
            }

        public:
            virtual ~RequestImpl() {}

            RequestImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }

            RequestImpl(const RequestImpl& src) : AbstractXMLObject(src), RequestAbstractTypeImpl(src) {
                init();
            }

            void _clone(const RequestImpl& src) {
                RequestAbstractTypeImpl::_clone(src);
                IMPL_CLONE_TYPED_CHILD(Query);
                IMPL_CLONE_TYPED_FOREIGN_CHILDREN(AssertionIDReference,saml1);
                IMPL_CLONE_TYPED_CHILDREN(AssertionArtifact);
            }

            IMPL_XMLOBJECT_CLONE_EX(Request);
            IMPL_TYPED_CHILD(Query);

            SubjectQuery* getSubjectQuery() const {
                return dynamic_cast<SubjectQuery*>(getQuery());
            }
            AuthenticationQuery* getAuthenticationQuery() const {
                return dynamic_cast<AuthenticationQuery*>(getQuery());
            }
            AttributeQuery* getAttributeQuery() const {
                return dynamic_cast<AttributeQuery*>(getQuery());
            }
            AuthorizationDecisionQuery* getAuthorizationDecisionQuery() const {
                return dynamic_cast<AuthorizationDecisionQuery*>(getQuery());
            }

            void setSubjectQuery(SubjectQuery* q) {
                setQuery(q);
            }
            void setAuthenticationQuery(AuthenticationQuery* q) {
                setQuery(q);
            }
            void setAttributeQuery(AttributeQuery* q) {
                setQuery(q);
            }
            void setAuthorizationDecisionQuery(AuthorizationDecisionQuery* q) {
                setQuery(q);
            }

            IMPL_TYPED_FOREIGN_CHILDREN(AssertionIDReference,saml1,m_children.end());
            IMPL_TYPED_CHILDREN(AssertionArtifact,m_children.end());

        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILD(Query,SAML1P_NS,true);
                PROC_TYPED_FOREIGN_CHILDREN(AssertionIDReference,saml1,SAML1_NS,false);
                PROC_TYPED_CHILDREN(AssertionArtifact,SAML1P_NS,false);
                RequestAbstractTypeImpl::processChildElement(childXMLObject,root);
            }
        };

        class SAML_DLLLOCAL StatusCodeImpl : public virtual StatusCode,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
            void init() {
                m_Value=nullptr;
                m_children.push_back(nullptr);
                m_StatusCode=nullptr;
                m_pos_StatusCode=m_children.begin();
            }

        public:
            virtual ~StatusCodeImpl() {
                delete m_Value;
            }

            StatusCodeImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }

            StatusCodeImpl(const StatusCodeImpl& src)
                    : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
                init();
                IMPL_CLONE_ATTRIB(Value);
                IMPL_CLONE_TYPED_CHILD(StatusCode);
            }

            IMPL_XMLOBJECT_CLONE(StatusCode);
            IMPL_XMLOBJECT_ATTRIB(Value,xmltooling::QName);
            IMPL_TYPED_CHILD(StatusCode);

        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_QNAME_ATTRIB(Value,VALUE,nullptr);
            }

            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILD(StatusCode,SAML1P_NS,true);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }

            void processAttribute(const DOMAttr* attribute) {
                PROC_QNAME_ATTRIB(Value,VALUE,nullptr);
            }
        };

        class SAML_DLLLOCAL StatusDetailImpl : public virtual StatusDetail,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~StatusDetailImpl() {}

            StatusDetailImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {}

            StatusDetailImpl(const StatusDetailImpl& src)
                    : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
                IMPL_CLONE_XMLOBJECT_CHILDREN(UnknownXMLObject);
            }

            IMPL_XMLOBJECT_CLONE(StatusDetail);
            IMPL_XMLOBJECT_CHILDREN(UnknownXMLObject,m_children.end());

        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                getUnknownXMLObjects().push_back(childXMLObject);
            }
        };

        class SAML_DLLLOCAL StatusImpl : public virtual Status,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
            void init() {
                m_children.push_back(nullptr);
                m_children.push_back(nullptr);
                m_children.push_back(nullptr);
                m_StatusCode=nullptr;
                m_pos_StatusCode=m_children.begin();
                m_StatusMessage=nullptr;
                m_pos_StatusMessage=m_pos_StatusCode;
                ++m_pos_StatusMessage;
                m_StatusDetail=nullptr;
                m_pos_StatusDetail=m_pos_StatusMessage;
                ++m_pos_StatusDetail;
            }

        public:
            virtual ~StatusImpl() {}

            StatusImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }

            StatusImpl(const StatusImpl& src)
                    : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
                init();
                IMPL_CLONE_TYPED_CHILD(StatusCode);
                IMPL_CLONE_TYPED_CHILD(StatusMessage);
                IMPL_CLONE_TYPED_CHILD(StatusDetail);
            }

            IMPL_XMLOBJECT_CLONE(Status);
            IMPL_TYPED_CHILD(StatusCode);
            IMPL_TYPED_CHILD(StatusMessage);
            IMPL_TYPED_CHILD(StatusDetail);

            // Base class methods.
            const XMLCh* getTopStatus() const {
                const xmltooling::QName* code = getStatusCode() ? getStatusCode()->getValue() : nullptr;
                return code ? code->getLocalPart() : nullptr;
            }
            const XMLCh* getSubStatus() const {
                const StatusCode* sc = getStatusCode() ? getStatusCode()->getStatusCode() : nullptr;
                if (sc)
                    return sc->getValue() ? sc->getValue()->getLocalPart() : nullptr;
                return nullptr;
            }
            bool hasAdditionalStatus() const {
                return (getStatusCode() && getStatusCode()->getStatusCode() && getStatusCode()->getStatusCode()->getStatusCode());
            }
            const XMLCh* getMessage() const {
                return getStatusMessage() ? getStatusMessage()->getMessage() : nullptr;
            }

        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILD(StatusCode,SAML1P_NS,false);
                PROC_TYPED_CHILD(StatusMessage,SAML1P_NS,false);
                PROC_TYPED_CHILD(StatusDetail,SAML1P_NS,false);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }
        };

        class SAML_DLLLOCAL ResponseAbstractTypeImpl : public virtual ResponseAbstractType,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
            void init() {
                m_MinorVersion=nullptr;
                m_ResponseID=nullptr;
                m_InResponseTo=nullptr;
                m_IssueInstant=nullptr;
                m_Recipient=nullptr;
                m_children.push_back(nullptr);
                m_Signature=nullptr;
                m_pos_Signature=m_children.begin();
            }

        protected:
            ResponseAbstractTypeImpl() {
                init();
            }

        public:
            virtual ~ResponseAbstractTypeImpl() {
                XMLString::release(&m_MinorVersion);
                XMLString::release(&m_ResponseID);
                XMLString::release(&m_InResponseTo);
                XMLString::release(&m_Recipient);
                delete m_IssueInstant;
            }

            ResponseAbstractTypeImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                    : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }

            ResponseAbstractTypeImpl(const ResponseAbstractTypeImpl& src)
                    : AbstractXMLObject(src), AbstractComplexElement(src), AbstractDOMCachingXMLObject(src) {
                init();
            }

            //IMPL_TYPED_CHILD(Signature);
            // Need customized setter.
        protected:
            void _clone(const ResponseAbstractTypeImpl& src) {
                IMPL_CLONE_INTEGER_ATTRIB(MinorVersion);
                IMPL_CLONE_ATTRIB(ResponseID);
                IMPL_CLONE_ATTRIB(InResponseTo);
                IMPL_CLONE_ATTRIB(IssueInstant);
                IMPL_CLONE_ATTRIB(Recipient);
                IMPL_CLONE_TYPED_CHILD(Signature);
            }

            xmlsignature::Signature* m_Signature;
            list<XMLObject*>::iterator m_pos_Signature;
        public:
            xmlsignature::Signature* getSignature() const {
                return m_Signature;
            }

            void setSignature(xmlsignature::Signature* sig) {
                prepareForAssignment(m_Signature,sig);
                *m_pos_Signature=m_Signature=sig;
                // Sync content reference back up.
                if (m_Signature)
                    m_Signature->setContentReference(new opensaml::ContentReference(*this));
            }

            ResponseAbstractType* cloneResponseAbstractType() const {
                return dynamic_cast<ResponseAbstractType*>(clone());
            }

            IMPL_INTEGER_ATTRIB(MinorVersion);
            IMPL_STRING_ATTRIB(ResponseID);    // have to special-case getXMLID
            const XMLCh* getXMLID() const {
                pair<bool,int> v = getMinorVersion();
                return (!v.first || v.second > 0) ? m_ResponseID : nullptr;
            }
            const XMLCh* getID() const {
                return getResponseID();
            }
            void releaseDOM() const {
                if (getDOM())
                    getDOM()->removeAttributeNS(nullptr, RESPONSEID_ATTRIB_NAME);
                AbstractDOMCachingXMLObject::releaseDOM();
            }
            IMPL_STRING_ATTRIB(InResponseTo);
            IMPL_DATETIME_ATTRIB(IssueInstant,0);
            IMPL_STRING_ATTRIB(Recipient);

        protected:
            void prepareForMarshalling() const {
                if (m_Signature)
                    declareNonVisibleNamespaces();
            }

            void marshallAttributes(DOMElement* domElement) const {
                static const XMLCh MAJORVERSION[] = UNICODE_LITERAL_12(M,a,j,o,r,V,e,r,s,i,o,n);
                domElement->setAttributeNS(nullptr,MAJORVERSION,XML_ONE);
                if (!m_MinorVersion)
                    const_cast<ResponseAbstractTypeImpl*>(this)->m_MinorVersion=XMLString::replicate(XML_ONE);
                MARSHALL_INTEGER_ATTRIB(MinorVersion,MINORVERSION,nullptr);
                if (!m_ResponseID)
                    const_cast<ResponseAbstractTypeImpl*>(this)->m_ResponseID=SAMLConfig::getConfig().generateIdentifier();
                domElement->setAttributeNS(nullptr, RESPONSEID_ATTRIB_NAME, m_ResponseID);
                if (*m_MinorVersion!=chDigit_0) {
#ifdef XMLTOOLING_XERCESC_BOOLSETIDATTRIBUTE
                    domElement->setIdAttributeNS(nullptr, RESPONSEID_ATTRIB_NAME, true);
#else
                    domElement->setIdAttributeNS(nullptr, RESPONSEID_ATTRIB_NAME);
#endif
                }
                MARSHALL_STRING_ATTRIB(InResponseTo,INRESPONSETO,nullptr);
                if (!m_IssueInstant) {
                    const_cast<ResponseAbstractTypeImpl*>(this)->m_IssueInstantEpoch=time(nullptr);
                    const_cast<ResponseAbstractTypeImpl*>(this)->m_IssueInstant=new DateTime(m_IssueInstantEpoch);
                }
                MARSHALL_DATETIME_ATTRIB(IssueInstant,ISSUEINSTANT,nullptr);
                MARSHALL_STRING_ATTRIB(Recipient,RECIPIENT,nullptr);
            }

            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_FOREIGN_CHILD(Signature,xmlsignature,XMLSIG_NS,false);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }

            void unmarshallAttributes(const DOMElement* domElement) {
                // Standard processing, but then we check IDness.
                AbstractXMLObjectUnmarshaller::unmarshallAttributes(domElement);
                if (m_ResponseID && (!m_MinorVersion || *m_MinorVersion!=chDigit_0)) {
#ifdef XMLTOOLING_XERCESC_BOOLSETIDATTRIBUTE
                    const_cast<DOMElement*>(domElement)->setIdAttributeNS(nullptr, RESPONSEID_ATTRIB_NAME, true);
#else
                    const_cast<DOMElement*>(domElement)->setIdAttributeNS(nullptr, RESPONSEID_ATTRIB_NAME);
#endif
                }
            }

            void processAttribute(const DOMAttr* attribute) {
                static const XMLCh MAJORVERSION[] = UNICODE_LITERAL_12(M,a,j,o,r,V,e,r,s,i,o,n);
                if (XMLHelper::isNodeNamed(attribute,nullptr,MAJORVERSION)) {
                    if (!XMLString::equals(attribute->getValue(),XML_ONE))
                        throw UnmarshallingException("Response has invalid major version.");
                }
                PROC_INTEGER_ATTRIB(MinorVersion,MINORVERSION,nullptr);
                PROC_STRING_ATTRIB(ResponseID,RESPONSEID,nullptr);
                PROC_STRING_ATTRIB(InResponseTo,INRESPONSETO,nullptr);
                PROC_DATETIME_ATTRIB(IssueInstant,ISSUEINSTANT,nullptr);
                PROC_STRING_ATTRIB(Recipient,RECIPIENT,nullptr);
            }
        };

        class SAML_DLLLOCAL ResponseImpl : public virtual Response, public ResponseAbstractTypeImpl
        {
            void init() {
                m_children.push_back(nullptr);
                m_Status=nullptr;
                m_pos_Status=m_pos_Signature;
                ++m_pos_Status;
            }

        public:
            virtual ~ResponseImpl() {}

            ResponseImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const xmltooling::QName* schemaType)
                    : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }

            ResponseImpl(const ResponseImpl& src) : AbstractXMLObject(src), ResponseAbstractTypeImpl(src) {
                init();
            }

            void _clone(const ResponseImpl& src) {
                ResponseAbstractTypeImpl::_clone(src);
                IMPL_CLONE_TYPED_CHILD(Status);
                IMPL_CLONE_TYPED_FOREIGN_CHILDREN(Assertion,saml1);
            }

            IMPL_XMLOBJECT_CLONE_EX(Response);
            IMPL_TYPED_CHILD(Status);
            IMPL_TYPED_FOREIGN_CHILDREN(Assertion,saml1,m_children.end());

        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILD(Status,SAML1P_NS,false);
                PROC_TYPED_FOREIGN_CHILDREN(Assertion,saml1,SAML1_NS,true);
                ResponseAbstractTypeImpl::processChildElement(childXMLObject,root);
            }
        };

    };
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

// Builder Implementations

IMPL_XMLOBJECTBUILDER(AssertionArtifact);
IMPL_XMLOBJECTBUILDER(AttributeQuery);
IMPL_XMLOBJECTBUILDER(AuthenticationQuery);
IMPL_XMLOBJECTBUILDER(AuthorizationDecisionQuery);
IMPL_XMLOBJECTBUILDER(Query);
IMPL_XMLOBJECTBUILDER(Request);
IMPL_XMLOBJECTBUILDER(RespondWith);
IMPL_XMLOBJECTBUILDER(Response);
IMPL_XMLOBJECTBUILDER(Status);
IMPL_XMLOBJECTBUILDER(StatusCode);
IMPL_XMLOBJECTBUILDER(StatusDetail);
IMPL_XMLOBJECTBUILDER(StatusMessage);

// Unicode literals
const XMLCh RequestAbstractType::LOCAL_NAME[] =             {chNull};
const XMLCh RequestAbstractType::TYPE_NAME[] =              UNICODE_LITERAL_19(R,e,q,u,e,s,t,A,b,s,t,r,a,c,t,T,y,p,e);
const XMLCh RequestAbstractType::MINORVERSION_ATTRIB_NAME[] =   UNICODE_LITERAL_12(M,i,n,o,r,V,e,r,s,i,o,n);
const XMLCh RequestAbstractType::REQUESTID_ATTRIB_NAME[] =      UNICODE_LITERAL_9(R,e,q,u,e,s,t,I,D);
const XMLCh RequestAbstractType::ISSUEINSTANT_ATTRIB_NAME[] =   UNICODE_LITERAL_12(I,s,s,u,e,I,n,s,t,a,n,t);
const XMLCh ResponseAbstractType::LOCAL_NAME[] =            {chNull};
const XMLCh ResponseAbstractType::TYPE_NAME[] =             UNICODE_LITERAL_20(R,e,s,p,o,n,s,e,A,b,s,t,r,a,c,t,T,y,p,e);
const XMLCh ResponseAbstractType::MINORVERSION_ATTRIB_NAME[] =  UNICODE_LITERAL_12(M,i,n,o,r,V,e,r,s,i,o,n);
const XMLCh ResponseAbstractType::RESPONSEID_ATTRIB_NAME[] =    UNICODE_LITERAL_10(R,e,s,p,o,n,s,e,I,D);
const XMLCh ResponseAbstractType::ISSUEINSTANT_ATTRIB_NAME[] =  UNICODE_LITERAL_12(I,s,s,u,e,I,n,s,t,a,n,t);
const XMLCh ResponseAbstractType::INRESPONSETO_ATTRIB_NAME[] =  UNICODE_LITERAL_12(I,n,R,e,s,p,o,n,s,e,T,o);
const XMLCh ResponseAbstractType::RECIPIENT_ATTRIB_NAME[] =     UNICODE_LITERAL_9(R,e,c,i,p,i,e,n,t);
const XMLCh AssertionArtifact::LOCAL_NAME[] =               UNICODE_LITERAL_17(A,s,s,e,r,t,i,o,n,A,r,t,i,f,a,c,t);
const XMLCh AttributeQuery::LOCAL_NAME[] =                  UNICODE_LITERAL_14(A,t,t,r,i,b,u,t,e,Q,u,e,r,y);
const XMLCh AttributeQuery::TYPE_NAME[] =                   UNICODE_LITERAL_18(A,t,t,r,i,b,u,t,e,Q,u,e,r,y,T,y,p,e);
const XMLCh AttributeQuery::RESOURCE_ATTRIB_NAME[] =        UNICODE_LITERAL_8(R,e,s,o,u,r,c,e);
const XMLCh AuthenticationQuery::LOCAL_NAME[] =             UNICODE_LITERAL_19(A,u,t,h,e,n,t,i,c,a,t,i,o,n,Q,u,e,r,y);
const XMLCh AuthenticationQuery::TYPE_NAME[] =              UNICODE_LITERAL_23(A,u,t,h,e,n,t,i,c,a,t,i,o,n,Q,u,e,r,y,T,y,p,e);
const XMLCh AuthenticationQuery::AUTHENTICATIONMETHOD_ATTRIB_NAME[] =   UNICODE_LITERAL_20(A,u,t,h,e,n,t,i,c,a,t,i,o,n,M,e,t,h,o,d);
const XMLCh AuthorizationDecisionQuery::LOCAL_NAME[] =      UNICODE_LITERAL_26(A,u,t,h,o,r,i,z,a,t,i,o,n,D,e,c,i,s,i,o,n,Q,u,e,r,y);
const XMLCh AuthorizationDecisionQuery::TYPE_NAME[] =       UNICODE_LITERAL_30(A,u,t,h,o,r,i,z,a,t,i,o,n,D,e,c,i,s,i,o,n,Q,u,e,r,y,T,y,p,e);
const XMLCh AuthorizationDecisionQuery::RESOURCE_ATTRIB_NAME[] =        UNICODE_LITERAL_8(R,e,s,o,u,r,c,e);
const XMLCh Query::LOCAL_NAME[] =                           UNICODE_LITERAL_5(Q,u,e,r,y);
const XMLCh Request::LOCAL_NAME[] =                         UNICODE_LITERAL_7(R,e,q,u,e,s,t);
const XMLCh Request::TYPE_NAME[] =                          UNICODE_LITERAL_11(R,e,q,u,e,s,t,T,y,p,e);
const XMLCh RespondWith::LOCAL_NAME[] =                     UNICODE_LITERAL_11(R,e,s,p,o,n,d,W,i,t,h);
const XMLCh Response::LOCAL_NAME[] =                        UNICODE_LITERAL_8(R,e,s,p,o,n,s,e);
const XMLCh Response::TYPE_NAME[] =                         UNICODE_LITERAL_12(R,e,s,p,o,n,s,e,T,y,p,e);
const XMLCh Status::LOCAL_NAME[] =                          UNICODE_LITERAL_6(S,t,a,t,u,s);
const XMLCh Status::TYPE_NAME[] =                           UNICODE_LITERAL_10(S,t,a,t,u,s,T,y,p,e);
const XMLCh StatusCode::LOCAL_NAME[] =                      UNICODE_LITERAL_10(S,t,a,t,u,s,C,o,d,e);
const XMLCh StatusCode::TYPE_NAME[] =                       UNICODE_LITERAL_14(S,t,a,t,u,s,C,o,d,e,T,y,p,e);
const XMLCh StatusCode::VALUE_ATTRIB_NAME[] =               UNICODE_LITERAL_5(V,a,l,u,e);
const XMLCh StatusDetail::LOCAL_NAME[] =                    UNICODE_LITERAL_12(S,t,a,t,u,s,D,e,t,a,i,l);
const XMLCh StatusDetail::TYPE_NAME[] =                     UNICODE_LITERAL_16(S,t,a,t,u,s,D,e,t,a,i,l,T,y,p,e);
const XMLCh StatusMessage::LOCAL_NAME[] =                   UNICODE_LITERAL_13(S,t,a,t,u,s,M,e,s,s,a,g,e);
const XMLCh SubjectQuery::LOCAL_NAME[] =                    UNICODE_LITERAL_12(S,u,b,j,e,c,t,Q,u,e,r,y);

#define XCH(ch) chLatin_##ch
#define XNUM(d) chDigit_##d

const XMLCh _SUCCESS[] =                                    UNICODE_LITERAL_7(S,u,c,c,e,s,s);
const XMLCh _REQUESTER[] =                                  UNICODE_LITERAL_9(R,e,q,u,e,s,t,e,r);
const XMLCh _RESPONDER[] =                                  UNICODE_LITERAL_9(R,e,s,p,o,n,d,e,r);
const XMLCh _VERSIONMISMATCH[] =                            UNICODE_LITERAL_15(V,e,r,s,i,o,n,M,i,s,m,a,t,c,h);

xmltooling::QName StatusCode::SUCCESS(SAML1P_NS,_SUCCESS,SAML1P_PREFIX);
xmltooling::QName StatusCode::REQUESTER(SAML1P_NS,_REQUESTER,SAML1P_PREFIX);
xmltooling::QName StatusCode::RESPONDER(SAML1P_NS,_RESPONDER,SAML1P_PREFIX);
xmltooling::QName StatusCode::VERSIONMISMATCH(SAML1P_NS,_VERSIONMISMATCH,SAML1P_PREFIX);
