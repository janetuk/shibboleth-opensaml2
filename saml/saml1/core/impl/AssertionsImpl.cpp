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
 * AssertionsImpl.cpp
 * 
 * Implementation classes for SAML 1.x Assertions schema
 */

#include "internal.h"
#include "exceptions.h"
#include "saml1/core/Assertions.h"

#include <xmltooling/AbstractChildlessElement.h>
#include <xmltooling/AbstractComplexElement.h>
#include <xmltooling/AbstractElementProxy.h>
#include <xmltooling/AbstractSimpleElement.h>
#include <xmltooling/impl/AnyElement.h>
#include <xmltooling/io/AbstractXMLObjectMarshaller.h>
#include <xmltooling/io/AbstractXMLObjectUnmarshaller.h>
#include <xmltooling/util/XMLHelper.h>
#include <xmltooling/validation/AbstractValidatingXMLObject.h>

#include <ctime>
#include <xercesc/util/XMLUniDefs.hpp>

using namespace opensaml::saml1;
using namespace opensaml;
using namespace xmlsignature;
using namespace xmltooling;
using namespace std;

#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4250 4251 )
#endif

namespace opensaml {
    namespace saml1 {
    
        DECL_XMLOBJECTIMPL_SIMPLE(SAML_DLLLOCAL,AssertionIDReference);
        DECL_XMLOBJECTIMPL_SIMPLE(SAML_DLLLOCAL,Audience);
        DECL_XMLOBJECTIMPL_SIMPLE(SAML_DLLLOCAL,ConfirmationMethod);
        
        class XMLTOOL_DLLLOCAL AudienceRestrictionConditionImpl : public virtual AudienceRestrictionCondition,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~AudienceRestrictionConditionImpl() {}
    
            AudienceRestrictionConditionImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            }
                
            AudienceRestrictionConditionImpl(const AudienceRestrictionConditionImpl& src)
                    : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
                VectorOf(Audience) v=getAudiences();
                for (vector<Audience*>::const_iterator i=src.m_Audiences.begin(); i!=src.m_Audiences.end(); i++) {
                    if (*i) {
                        v.push_back((*i)->cloneAudience());
                    }
                }
            }
            
            IMPL_XMLOBJECT_CLONE(AudienceRestrictionCondition);
            Condition* cloneCondition() const {
                return cloneAudienceRestrictionCondition();
            }
            IMPL_TYPED_CHILDREN(Audience,m_children.end());
    
        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILDREN(Audience,SAMLConstants::SAML1_NS,false);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }
        };

        class XMLTOOL_DLLLOCAL DoNotCacheConditionImpl : public virtual DoNotCacheCondition,
            public AbstractChildlessElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~DoNotCacheConditionImpl() {}
    
            DoNotCacheConditionImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            }
                
            DoNotCacheConditionImpl(const DoNotCacheConditionImpl& src)
                    : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
            }
            
            IMPL_XMLOBJECT_CLONE(DoNotCacheCondition);
            Condition* cloneCondition() const {
                return cloneDoNotCacheCondition();
            }
        };

        class SAML_DLLLOCAL ConditionsImpl : public virtual Conditions,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~ConditionsImpl() {
                delete m_NotBefore;
                delete m_NotOnOrAfter;
            }
    
            ConditionsImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            ConditionsImpl(const ConditionsImpl& src)
                    : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
                init();
                setNotBefore(src.getNotBefore());
                setNotOnOrAfter(src.getNotOnOrAfter());

                for (list<XMLObject*>::const_iterator i=src.m_children.begin(); i!=src.m_children.end(); i++) {
                    if (*i) {
                        AudienceRestrictionCondition* arc=dynamic_cast<AudienceRestrictionCondition*>(*i);
                        if (arc) {
                            getAudienceRestrictionConditions().push_back(arc->cloneAudienceRestrictionCondition());
                            continue;
                        }
    
                        DoNotCacheCondition* dncc=dynamic_cast<DoNotCacheCondition*>(*i);
                        if (dncc) {
                            getDoNotCacheConditions().push_back(dncc->cloneDoNotCacheCondition());
                            continue;
                        }
    
                        Condition* c=dynamic_cast<Condition*>(*i);
                        if (c) {
                            getConditions().push_back(c->cloneCondition());
                            continue;
                        }
                    }
                }
            }
            
            void init() {
                m_NotBefore=m_NotOnOrAfter=NULL;
            }
            
            IMPL_XMLOBJECT_CLONE(Conditions);
            IMPL_DATETIME_ATTRIB(NotBefore);
            IMPL_DATETIME_ATTRIB(NotOnOrAfter);
            IMPL_TYPED_CHILDREN(AudienceRestrictionCondition, m_children.end());
            IMPL_TYPED_CHILDREN(DoNotCacheCondition,m_children.end());
            IMPL_TYPED_CHILDREN(Condition,m_children.end());
    
        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_DATETIME_ATTRIB(NotBefore,NOTBEFORE,NULL);
                MARSHALL_DATETIME_ATTRIB(NotOnOrAfter,NOTONORAFTER,NULL);
            }
    
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILDREN(AudienceRestrictionCondition,SAMLConstants::SAML1_NS,true);
                PROC_TYPED_CHILDREN(DoNotCacheCondition,SAMLConstants::SAML1_NS,true);
                PROC_TYPED_CHILDREN(Condition,SAMLConstants::SAML1_NS,true);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }
    
            void processAttribute(const DOMAttr* attribute) {
                PROC_DATETIME_ATTRIB(NotBefore,NOTBEFORE,NULL);
                PROC_DATETIME_ATTRIB(NotOnOrAfter,NOTONORAFTER,NULL);
            }
        };

        class SAML_DLLLOCAL NameIdentifierImpl : public virtual NameIdentifier,
            public AbstractSimpleElement,
            public AbstractChildlessElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~NameIdentifierImpl() {
                XMLString::release(&m_Format);
                XMLString::release(&m_NameQualifier);
            }
    
            NameIdentifierImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                    : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            NameIdentifierImpl(const NameIdentifierImpl& src)
                    : AbstractXMLObject(src), AbstractSimpleElement(src),
                        AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
                init();
                setFormat(src.getFormat());
                setNameQualifier(src.getNameQualifier());
            }
            
            void init() {
                m_Format=m_NameQualifier=NULL;
            }
            
            IMPL_XMLOBJECT_CLONE(NameIdentifier);
            IMPL_STRING_ATTRIB(Format);
            IMPL_STRING_ATTRIB(NameQualifier);
            IMPL_XMLOBJECT_CONTENT;
    
        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_STRING_ATTRIB(Format,FORMAT,NULL);
                MARSHALL_STRING_ATTRIB(NameQualifier,NAMEQUALIFIER,NULL);
            }

            void processAttribute(const DOMAttr* attribute) {
                PROC_STRING_ATTRIB(Format,FORMAT,NULL);
                PROC_STRING_ATTRIB(NameQualifier,NAMEQUALIFIER,NULL);
            }
        };

        class SAML_DLLLOCAL SubjectConfirmationDataImpl
            : public virtual SubjectConfirmationData, public AnyElementImpl, public AbstractValidatingXMLObject
        {
        public:
            virtual ~SubjectConfirmationDataImpl() {}
    
            SubjectConfirmationDataImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AnyElementImpl(nsURI, localName, prefix, schemaType) {
            }
                
            SubjectConfirmationDataImpl(const SubjectConfirmationDataImpl& src)
                : AnyElementImpl(src), AbstractValidatingXMLObject(src) {
            }
            
            IMPL_XMLOBJECT_CLONE(SubjectConfirmationData);
        };

        class SAML_DLLLOCAL SubjectConfirmationImpl : public virtual SubjectConfirmation,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~SubjectConfirmationImpl() {}
    
            SubjectConfirmationImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            SubjectConfirmationImpl(const SubjectConfirmationImpl& src)
                    : AbstractXMLObject(src),
                        AbstractDOMCachingXMLObject(src),
                        AbstractValidatingXMLObject(src) {
                init();
                if (src.getSubjectConfirmationData())
                    setSubjectConfirmationData(src.getSubjectConfirmationData()->clone());
                if (src.getKeyInfo())
                    setKeyInfo(src.getKeyInfo()->cloneKeyInfo());
                VectorOf(ConfirmationMethod) v=getConfirmationMethods();
                for (vector<ConfirmationMethod*>::const_iterator i=src.m_ConfirmationMethods.begin(); i!=src.m_ConfirmationMethods.end(); i++) {
                    if (*i) {
                        v.push_back((*i)->cloneConfirmationMethod());
                    }
                }
            }
            
            void init() {
                m_SubjectConfirmationData=NULL;
                m_KeyInfo=NULL;
                m_children.push_back(NULL);
                m_children.push_back(NULL);
                m_pos_SubjectConfirmationData=m_children.begin();
                m_pos_KeyInfo=m_pos_SubjectConfirmationData;
                ++m_pos_KeyInfo;
            }

            IMPL_XMLOBJECT_CLONE(SubjectConfirmation);
            IMPL_TYPED_CHILDREN(ConfirmationMethod,m_pos_SubjectConfirmationData);
            IMPL_XMLOBJECT_CHILD(SubjectConfirmationData);
            IMPL_TYPED_CHILD(KeyInfo);
    
        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILDREN(ConfirmationMethod,SAMLConstants::SAML1_NS,false);
                PROC_TYPED_CHILD(KeyInfo,XMLConstants::XMLSIG_NS,false);
                
                // Anything else we'll assume is the data.
                if (getSubjectConfirmationData())
                    throw UnmarshallingException("Invalid child element: $1",params(1,childXMLObject->getElementQName().toString().c_str()));
                setSubjectConfirmationData(childXMLObject);
            }
        };

        class SAML_DLLLOCAL SubjectImpl : public virtual Subject,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~SubjectImpl() {}
    
            SubjectImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            SubjectImpl(const SubjectImpl& src)
                    : AbstractXMLObject(src),
                        AbstractDOMCachingXMLObject(src),
                        AbstractValidatingXMLObject(src) {
                init();
                if (src.getNameIdentifier())
                    setNameIdentifier(src.getNameIdentifier()->cloneNameIdentifier());
                if (src.getSubjectConfirmation())
                    setSubjectConfirmation(src.getSubjectConfirmation()->cloneSubjectConfirmation());
            }
            
            void init() {
                m_NameIdentifier=NULL;
                m_SubjectConfirmation=NULL;
                m_children.push_back(NULL);
                m_children.push_back(NULL);
                m_pos_NameIdentifier=m_children.begin();
                m_pos_SubjectConfirmation=m_pos_NameIdentifier;
                ++m_pos_SubjectConfirmation;
            }

            IMPL_XMLOBJECT_CLONE(Subject);
            IMPL_TYPED_CHILD(NameIdentifier);
            IMPL_TYPED_CHILD(SubjectConfirmation);
    
        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILD(NameIdentifier,SAMLConstants::SAML1_NS,true);
                PROC_TYPED_CHILD(SubjectConfirmation,SAMLConstants::SAML1_NS,true);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }
        };

        class SAML_DLLLOCAL SubjectStatementImpl : public virtual SubjectStatement,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~SubjectStatementImpl() {}
    
            SubjectStatementImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            SubjectStatementImpl(const SubjectStatementImpl& src)
                    : AbstractXMLObject(src),
                        AbstractDOMCachingXMLObject(src),
                        AbstractValidatingXMLObject(src) {
                init();
                if (src.getSubject())
                    setSubject(src.getSubject()->cloneSubject());
            }
            
            void init() {
                m_Subject=NULL;
                m_children.push_back(NULL);
                m_pos_Subject=m_children.begin();
            }

            IMPL_TYPED_CHILD(Subject);
    
        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILD(Subject,SAMLConstants::SAML1_NS,true);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }
        };

        class SAML_DLLLOCAL SubjectLocalityImpl : public virtual SubjectLocality,
            public AbstractChildlessElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~SubjectLocalityImpl() {
                XMLString::release(&m_IPAddress);
                XMLString::release(&m_DNSAddress);
            }
    
            SubjectLocalityImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            SubjectLocalityImpl(const SubjectLocalityImpl& src)
                    : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
                init();
                setIPAddress(src.getIPAddress());
                setDNSAddress(src.getDNSAddress());
            }
            
            void init() {
                m_IPAddress=m_DNSAddress=NULL;
            }
            
            IMPL_XMLOBJECT_CLONE(SubjectLocality);
            IMPL_STRING_ATTRIB(IPAddress);
            IMPL_STRING_ATTRIB(DNSAddress);
    
        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_STRING_ATTRIB(IPAddress,IPADDRESS,NULL);
                MARSHALL_STRING_ATTRIB(DNSAddress,DNSADDRESS,NULL);
            }
    
            void processAttribute(const DOMAttr* attribute) {
                PROC_STRING_ATTRIB(IPAddress,IPADDRESS,NULL);
                PROC_STRING_ATTRIB(DNSAddress,DNSADDRESS,NULL);
            }
        };

        class SAML_DLLLOCAL AuthorityBindingImpl : public virtual AuthorityBinding,
            public AbstractChildlessElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~AuthorityBindingImpl() {
                delete m_AuthorityKind;
                XMLString::release(&m_Location);
                XMLString::release(&m_Binding);
            }
    
            AuthorityBindingImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            AuthorityBindingImpl(const AuthorityBindingImpl& src)
                    : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
                init();
                setAuthorityKind(src.getAuthorityKind());
                setLocation(src.getLocation());
                setBinding(src.getBinding());
            }
            
            void init() {
                m_AuthorityKind=NULL;
                m_Location=m_Binding=NULL;
            }
            
            IMPL_XMLOBJECT_CLONE(AuthorityBinding);
            IMPL_XMLOBJECT_ATTRIB(AuthorityKind,QName);
            IMPL_STRING_ATTRIB(Location);
            IMPL_STRING_ATTRIB(Binding);
    
        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_QNAME_ATTRIB(AuthorityKind,AUTHORITYKIND,NULL);
                MARSHALL_STRING_ATTRIB(Location,LOCATION,NULL);
                MARSHALL_STRING_ATTRIB(Binding,BINDING,NULL);
            }
    
            void processAttribute(const DOMAttr* attribute) {
                PROC_QNAME_ATTRIB(AuthorityKind,AUTHORITYKIND,NULL);
                PROC_STRING_ATTRIB(Location,LOCATION,NULL);
                PROC_STRING_ATTRIB(Binding,BINDING,NULL);
            }
        };

        class SAML_DLLLOCAL AuthenticationStatementImpl : public virtual AuthenticationStatement, public SubjectStatementImpl
        {
        public:
            virtual ~AuthenticationStatementImpl() {
                XMLString::release(&m_AuthenticationMethod);
                delete m_AuthenticationInstant;
            }
    
            AuthenticationStatementImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : SubjectStatementImpl(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            AuthenticationStatementImpl(const AuthenticationStatementImpl& src) : SubjectStatementImpl(src) {
                init();
                setAuthenticationMethod(src.getAuthenticationMethod());
                setAuthenticationInstant(src.getAuthenticationInstant());
                if (src.getSubjectLocality())
                    setSubjectLocality(src.getSubjectLocality()->cloneSubjectLocality());
                VectorOf(AuthorityBinding) v=getAuthorityBindings();
                for (vector<AuthorityBinding*>::const_iterator i=src.m_AuthorityBindings.begin(); i!=src.m_AuthorityBindings.end(); i++) {
                    if (*i) {
                        v.push_back((*i)->cloneAuthorityBinding());
                    }
                }
            }
            
            void init() {
                SubjectStatementImpl::init();
                m_AuthenticationMethod=NULL;
                m_AuthenticationInstant=NULL;
                m_SubjectLocality=NULL;
                m_children.push_back(NULL);
                m_pos_SubjectLocality=m_pos_Subject;
                m_pos_SubjectLocality++;
            }
            
            IMPL_XMLOBJECT_CLONE(AuthenticationStatement);
            SubjectStatement* cloneSubjectStatement() const {
                return cloneAuthenticationStatement();
            }
            Statement* cloneStatement() const {
                return cloneAuthenticationStatement();
            }
            IMPL_STRING_ATTRIB(AuthenticationMethod);
            IMPL_DATETIME_ATTRIB(AuthenticationInstant);
            IMPL_TYPED_CHILD(SubjectLocality);
            IMPL_TYPED_CHILDREN(AuthorityBinding, m_children.end());
    
        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_STRING_ATTRIB(AuthenticationMethod,AUTHENTICATIONMETHOD,NULL);
                MARSHALL_DATETIME_ATTRIB(AuthenticationInstant,AUTHENTICATIONINSTANT,NULL);
                SubjectStatementImpl::marshallAttributes(domElement);
            }
    
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILD(SubjectLocality,SAMLConstants::SAML1_NS,false);
                PROC_TYPED_CHILDREN(AuthorityBinding,SAMLConstants::SAML1_NS,false);
                SubjectStatementImpl::processChildElement(childXMLObject,root);
            }
    
            void processAttribute(const DOMAttr* attribute) {
                PROC_STRING_ATTRIB(AuthenticationMethod,AUTHENTICATIONMETHOD,NULL);
                PROC_DATETIME_ATTRIB(AuthenticationInstant,AUTHENTICATIONINSTANT,NULL);
                SubjectStatementImpl::processAttribute(attribute);
            }
        };

        class SAML_DLLLOCAL ActionImpl : public virtual Action,
            public AbstractSimpleElement,
            public AbstractChildlessElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~ActionImpl() {
                XMLString::release(&m_Namespace);
            }
    
            ActionImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                    : AbstractXMLObject(nsURI, localName, prefix, schemaType), m_Namespace(NULL) {
            }
                
            ActionImpl(const ActionImpl& src)
                    : AbstractXMLObject(src), AbstractSimpleElement(src),
                        AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
                setNamespace(src.getNamespace());
            }
            
            IMPL_XMLOBJECT_CLONE(Action);
            IMPL_STRING_ATTRIB(Namespace);
            IMPL_XMLOBJECT_CONTENT;
    
        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_STRING_ATTRIB(Namespace,NAMESPACE,NULL);
            }

            void processAttribute(const DOMAttr* attribute) {
                PROC_STRING_ATTRIB(Namespace,NAMESPACE,NULL);
            }
        };

        class SAML_DLLLOCAL EvidenceImpl : public virtual Evidence,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~EvidenceImpl() {}
    
            EvidenceImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            }
                
            EvidenceImpl(const EvidenceImpl& src)
                    : AbstractXMLObject(src),
                        AbstractDOMCachingXMLObject(src),
                        AbstractValidatingXMLObject(src) {
    
                for (list<XMLObject*>::const_iterator i=src.m_children.begin(); i!=src.m_children.end(); i++) {
                    if (*i) {
                        AssertionIDReference* ref=dynamic_cast<AssertionIDReference*>(*i);
                        if (ref) {
                            getAssertionIDReferences().push_back(ref->cloneAssertionIDReference());
                            continue;
                        }
    
                        Assertion* assertion=dynamic_cast<Assertion*>(*i);
                        if (assertion) {
                            getAssertions().push_back(assertion->cloneAssertion());
                            continue;
                        }
                    }
                }
            }
            
            IMPL_XMLOBJECT_CLONE(Evidence);
            IMPL_TYPED_CHILDREN(AssertionIDReference,m_children.end());
            IMPL_TYPED_CHILDREN(Assertion,m_children.end());
    
        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILDREN(AssertionIDReference,SAMLConstants::SAML1_NS,false);
                PROC_TYPED_CHILDREN(Assertion,SAMLConstants::SAML1_NS,true);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }
        };

        class SAML_DLLLOCAL AuthorizationDecisionStatementImpl
            : public virtual AuthorizationDecisionStatement, public SubjectStatementImpl
        {
        public:
            virtual ~AuthorizationDecisionStatementImpl() {
                XMLString::release(&m_Resource);
                XMLString::release(&m_Decision);
            }
    
            AuthorizationDecisionStatementImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : SubjectStatementImpl(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            AuthorizationDecisionStatementImpl(const AuthorizationDecisionStatementImpl& src) : SubjectStatementImpl(src) {
                init();
                setResource(src.getResource());
                setDecision(src.getDecision());
                if (src.getEvidence())
                    setEvidence(src.getEvidence()->cloneEvidence());
                VectorOf(Action) v=getActions();
                for (vector<Action*>::const_iterator i=src.m_Actions.begin(); i!=src.m_Actions.end(); i++) {
                    if (*i) {
                        v.push_back((*i)->cloneAction());
                    }
                }
            }
            
            void init() {
                SubjectStatementImpl::init();
                m_Resource=NULL;
                m_Decision=NULL;
                m_Evidence=NULL;
                m_children.push_back(NULL);
                m_pos_Evidence=m_pos_Subject;
                m_pos_Evidence++;
            }
            
            IMPL_XMLOBJECT_CLONE(AuthorizationDecisionStatement);
            SubjectStatement* cloneSubjectStatement() const {
                return cloneAuthorizationDecisionStatement();
            }
            Statement* cloneStatement() const {
                return cloneAuthorizationDecisionStatement();
            }
            IMPL_STRING_ATTRIB(Resource);
            IMPL_STRING_ATTRIB(Decision);
            IMPL_TYPED_CHILD(Evidence);
            IMPL_TYPED_CHILDREN(Action, m_pos_Evidence);
    
        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_STRING_ATTRIB(Resource,RESOURCE,NULL);
                MARSHALL_STRING_ATTRIB(Decision,DECISION,NULL);
                SubjectStatementImpl::marshallAttributes(domElement);
            }
    
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILD(Evidence,SAMLConstants::SAML1_NS,false);
                PROC_TYPED_CHILDREN(Action,SAMLConstants::SAML1_NS,false);
                SubjectStatementImpl::processChildElement(childXMLObject,root);
            }
    
            void processAttribute(const DOMAttr* attribute) {
                PROC_STRING_ATTRIB(Resource,RESOURCE,NULL);
                PROC_STRING_ATTRIB(Decision,DECISION,NULL);
                SubjectStatementImpl::processAttribute(attribute);
            }
        };

        class SAML_DLLLOCAL AttributeDesignatorImpl : public virtual AttributeDesignator,
            public AbstractChildlessElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~AttributeDesignatorImpl() {
                XMLString::release(&m_AttributeName);
                XMLString::release(&m_AttributeNamespace);
            }
    
            AttributeDesignatorImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            AttributeDesignatorImpl(const AttributeDesignatorImpl& src)
                    : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
                init();
                setAttributeName(src.getAttributeName());
                setAttributeNamespace(src.getAttributeNamespace());
            }
            
            void init() {
                m_AttributeName=m_AttributeNamespace=NULL;
            }
            
            IMPL_XMLOBJECT_CLONE(AttributeDesignator);
            IMPL_STRING_ATTRIB(AttributeName);
            IMPL_STRING_ATTRIB(AttributeNamespace);
    
        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_STRING_ATTRIB(AttributeName,ATTRIBUTENAME,NULL);
                MARSHALL_STRING_ATTRIB(AttributeNamespace,ATTRIBUTENAMESPACE,NULL);
            }
    
            void processAttribute(const DOMAttr* attribute) {
                PROC_STRING_ATTRIB(AttributeName,ATTRIBUTENAME,NULL);
                PROC_STRING_ATTRIB(AttributeNamespace,ATTRIBUTENAMESPACE,NULL);
            }
        };

        class SAML_DLLLOCAL AttributeImpl : public virtual Attribute,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~AttributeImpl() {
                XMLString::release(&m_AttributeName);
                XMLString::release(&m_AttributeNamespace);
            }
    
            AttributeImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            AttributeImpl(const AttributeImpl& src)
                    : AbstractXMLObject(src), AbstractDOMCachingXMLObject(src), AbstractValidatingXMLObject(src) {
                init();
                setAttributeName(src.getAttributeName());
                setAttributeNamespace(src.getAttributeNamespace());
                VectorOf(XMLObject) v=getAttributeValues();
                for (vector<XMLObject*>::const_iterator i=src.m_AttributeValues.begin(); i!=src.m_AttributeValues.end(); i++) {
                    if (*i) {
                        v.push_back((*i)->clone());
                    }
                }
            }
            
            void init() {
                m_AttributeName=m_AttributeNamespace=NULL;
            }
            
            IMPL_XMLOBJECT_CLONE(Attribute);
            AttributeDesignator* cloneAttributeDesignator() const {
                return cloneAttribute();
            }
            IMPL_STRING_ATTRIB(AttributeName);
            IMPL_STRING_ATTRIB(AttributeNamespace);
            IMPL_XMLOBJECT_CHILDREN(AttributeValue,m_children.end());
    
        protected:
            void marshallAttributes(DOMElement* domElement) const {
                MARSHALL_STRING_ATTRIB(AttributeName,ATTRIBUTENAME,NULL);
                MARSHALL_STRING_ATTRIB(AttributeNamespace,ATTRIBUTENAMESPACE,NULL);
            }

            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                getAttributeValues().push_back(childXMLObject);
            }

            void processAttribute(const DOMAttr* attribute) {
                PROC_STRING_ATTRIB(AttributeName,ATTRIBUTENAME,NULL);
                PROC_STRING_ATTRIB(AttributeNamespace,ATTRIBUTENAMESPACE,NULL);
            }
        };

        class SAML_DLLLOCAL AttributeValueImpl
            : public virtual AttributeValue, public AnyElementImpl, public AbstractValidatingXMLObject
        {
        public:
            virtual ~AttributeValueImpl() {}
    
            AttributeValueImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AnyElementImpl(nsURI, localName, prefix, schemaType) {
            }
                
            AttributeValueImpl(const AttributeValueImpl& src) : AnyElementImpl(src), AbstractValidatingXMLObject(src) {}
            
            IMPL_XMLOBJECT_CLONE(AttributeValue);
        };

        class SAML_DLLLOCAL AttributeStatementImpl : public virtual AttributeStatement, public SubjectStatementImpl
        {
        public:
            virtual ~AttributeStatementImpl() {}
    
            AttributeStatementImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : SubjectStatementImpl(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            AttributeStatementImpl(const AttributeStatementImpl& src) : SubjectStatementImpl(src) {
                VectorOf(Attribute) v=getAttributes();
                for (vector<Attribute*>::const_iterator i=src.m_Attributes.begin(); i!=src.m_Attributes.end(); i++) {
                    if (*i) {
                        v.push_back((*i)->cloneAttribute());
                    }
                }
            }
            
            IMPL_XMLOBJECT_CLONE(AttributeStatement);
            SubjectStatement* cloneSubjectStatement() const {
                return cloneAttributeStatement();
            }
            Statement* cloneStatement() const {
                return cloneAttributeStatement();
            }
            IMPL_TYPED_CHILDREN(Attribute, m_children.end());
    
        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILDREN(Attribute,SAMLConstants::SAML1_NS,true);
                SubjectStatementImpl::processChildElement(childXMLObject,root);
            }
        };

        class SAML_DLLLOCAL AdviceImpl : public virtual Advice,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~AdviceImpl() {}
    
            AdviceImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
            }
                
            AdviceImpl(const AdviceImpl& src)
                    : AbstractXMLObject(src),
                        AbstractDOMCachingXMLObject(src),
                        AbstractValidatingXMLObject(src) {
    
                for (list<XMLObject*>::const_iterator i=src.m_children.begin(); i!=src.m_children.end(); i++) {
                    if (*i) {
                        AssertionIDReference* ref=dynamic_cast<AssertionIDReference*>(*i);
                        if (ref) {
                            getAssertionIDReferences().push_back(ref->cloneAssertionIDReference());
                            continue;
                        }
    
                        Assertion* assertion=dynamic_cast<Assertion*>(*i);
                        if (assertion) {
                            getAssertions().push_back(assertion->cloneAssertion());
                            continue;
                        }
    
                        getOthers().push_back((*i)->clone());
                    }
                }
            }
            
            IMPL_XMLOBJECT_CLONE(Advice);
            IMPL_TYPED_CHILDREN(AssertionIDReference,m_children.end());
            IMPL_TYPED_CHILDREN(Assertion,m_children.end());
            IMPL_XMLOBJECT_CHILDREN(Other,m_children.end());
    
        protected:
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILDREN(AssertionIDReference,SAMLConstants::SAML1_NS,false);
                PROC_TYPED_CHILDREN(Assertion,SAMLConstants::SAML1_NS,true);
                
                // Unknown child.
                const XMLCh* nsURI=root->getNamespaceURI();
                if (!XMLString::equals(nsURI,SAMLConstants::SAML1_NS) && nsURI && *nsURI)
                    getOthers().push_back(childXMLObject);
                
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }
        };

        class SAML_DLLLOCAL AssertionImpl : public virtual Assertion,
            public AbstractComplexElement,
            public AbstractDOMCachingXMLObject,
            public AbstractValidatingXMLObject,
            public AbstractXMLObjectMarshaller,
            public AbstractXMLObjectUnmarshaller
        {
        public:
            virtual ~AssertionImpl() {
                XMLString::release(&m_AssertionID);
                XMLString::release(&m_Issuer);
                delete m_IssueInstant;
            }
    
            AssertionImpl(const XMLCh* nsURI, const XMLCh* localName, const XMLCh* prefix, const QName* schemaType)
                : AbstractXMLObject(nsURI, localName, prefix, schemaType) {
                init();
            }
                
            AssertionImpl(const AssertionImpl& src)                     
                : AbstractXMLObject(src),
                    AbstractDOMCachingXMLObject(src),
                    AbstractValidatingXMLObject(src) {
                init();
                setMinorVersion(src.getMinorVersion());
                setAssertionID(src.getAssertionID());
                setIssuer(src.getIssuer());
                setIssueInstant(src.getIssueInstant());
                if (src.getConditions())
                    setConditions(src.getConditions()->cloneConditions());
                if (src.getAdvice())
                    setAdvice(src.getAdvice()->cloneAdvice());
                if (src.getSignature())
                    setSignature(src.getSignature()->cloneSignature());
                for (list<XMLObject*>::const_iterator i=src.m_children.begin(); i!=src.m_children.end(); i++) {
                    if (*i) {
                        AuthenticationStatement* authst=dynamic_cast<AuthenticationStatement*>(*i);
                        if (authst) {
                            getAuthenticationStatements().push_back(authst->cloneAuthenticationStatement());
                            continue;
                        }
    
                        SubjectStatement* subst=dynamic_cast<SubjectStatement*>(*i);
                        if (subst) {
                            getSubjectStatements().push_back(subst->cloneSubjectStatement());
                            continue;
                        }
    
                        Statement* st=dynamic_cast<Statement*>(*i);
                        if (st) {
                            getStatements().push_back(st->cloneStatement());
                            continue;
                        }
                    }
                }
            }
            
            void init() {
                m_MinorVersion=1;
                m_AssertionID=NULL;
                m_Issuer=NULL;
                m_IssueInstant=NULL;
                m_children.push_back(NULL);
                m_children.push_back(NULL);
                m_children.push_back(NULL);
                m_Conditions=NULL;
                m_Advice=NULL;
                m_Signature=NULL;
                m_pos_Conditions=m_children.begin();
                m_pos_Advice=m_pos_Conditions;
                m_pos_Advice++;
                m_pos_Signature=m_pos_Advice;
                m_pos_Signature++;
            }
            
            IMPL_XMLOBJECT_CLONE(Assertion);
            IMPL_INTEGER_ATTRIB(MinorVersion);
            IMPL_STRING_ATTRIB(AssertionID);
            IMPL_STRING_ATTRIB(Issuer);
            IMPL_DATETIME_ATTRIB(IssueInstant);
            IMPL_TYPED_CHILD(Conditions);
            IMPL_TYPED_CHILD(Advice);
            IMPL_TYPED_CHILD(Signature);
            IMPL_TYPED_CHILDREN(Statement, m_pos_Signature);
            IMPL_TYPED_CHILDREN(SubjectStatement, m_pos_Signature);
            IMPL_TYPED_CHILDREN(AuthenticationStatement, m_pos_Signature);
    
        protected:
            void marshallAttributes(DOMElement* domElement) const {
                static const XMLCh MAJORVERSION[] = UNICODE_LITERAL_12(M,a,j,o,r,V,e,r,s,i,o,n);
                static const XMLCh ONE[] = { chDigit_1, chNull };
                domElement->setAttributeNS(NULL,MAJORVERSION,ONE);
                MARSHALL_INTEGER_ATTRIB(MinorVersion,MINORVERSION,NULL);
                if (!m_AssertionID)
                    const_cast<AssertionImpl*>(this)->m_AssertionID=SAMLConfig::getConfig().generateIdentifier();
                MARSHALL_ID_ATTRIB(AssertionID,ASSERTIONID,NULL);
                MARSHALL_STRING_ATTRIB(Issuer,ISSUER,NULL);
                if (!m_IssueInstant)
                    const_cast<AssertionImpl*>(this)->m_IssueInstant=new DateTime(time(NULL));
                MARSHALL_DATETIME_ATTRIB(IssueInstant,ISSUEINSTANT,NULL);
            }
    
            void processChildElement(XMLObject* childXMLObject, const DOMElement* root) {
                PROC_TYPED_CHILD(Conditions,SAMLConstants::SAML1_NS,false);
                PROC_TYPED_CHILD(Advice,SAMLConstants::SAML1_NS,false);
                PROC_TYPED_CHILD(Signature,XMLConstants::XMLSIG_NS,false);
                PROC_TYPED_CHILDREN(AuthenticationStatement,SAMLConstants::SAML1_NS,false);
                PROC_TYPED_CHILDREN(SubjectStatement,SAMLConstants::SAML1_NS,true);
                PROC_TYPED_CHILDREN(Statement,SAMLConstants::SAML1_NS,true);
                AbstractXMLObjectUnmarshaller::processChildElement(childXMLObject,root);
            }
    
            void processAttribute(const DOMAttr* attribute) {
                static const XMLCh MAJORVERSION[] = UNICODE_LITERAL_12(M,a,j,o,r,V,e,r,s,i,o,n);
                if (XMLHelper::isNodeNamed(attribute,NULL,MAJORVERSION)) {
                    if (XMLString::parseInt(attribute->getValue()) != 1)
                        throw UnmarshallingException("Assertion has invalid major version.");
                }
                PROC_INTEGER_ATTRIB(MinorVersion,MINORVERSION,NULL);
                PROC_ID_ATTRIB(AssertionID,ASSERTIONID,NULL);
                PROC_STRING_ATTRIB(Issuer,ISSUER,NULL);
                PROC_DATETIME_ATTRIB(IssueInstant,ISSUEINSTANT,NULL);
            }
        };
    
    };
};

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

// Builder Implementations

IMPL_XMLOBJECTBUILDER(Action);
IMPL_XMLOBJECTBUILDER(Advice);
IMPL_XMLOBJECTBUILDER(Assertion);
IMPL_XMLOBJECTBUILDER(AssertionIDReference);
IMPL_XMLOBJECTBUILDER(Attribute);
IMPL_XMLOBJECTBUILDER(AttributeDesignator);
IMPL_XMLOBJECTBUILDER(AttributeStatement);
IMPL_XMLOBJECTBUILDER(AttributeValue);
IMPL_XMLOBJECTBUILDER(Audience);
IMPL_XMLOBJECTBUILDER(AudienceRestrictionCondition);
IMPL_XMLOBJECTBUILDER(AuthenticationStatement);
IMPL_XMLOBJECTBUILDER(AuthorizationDecisionStatement);
IMPL_XMLOBJECTBUILDER(AuthorityBinding);
IMPL_XMLOBJECTBUILDER(Conditions);
IMPL_XMLOBJECTBUILDER(ConfirmationMethod);
IMPL_XMLOBJECTBUILDER(DoNotCacheCondition);
IMPL_XMLOBJECTBUILDER(Evidence);
IMPL_XMLOBJECTBUILDER(NameIdentifier);
IMPL_XMLOBJECTBUILDER(Subject);
IMPL_XMLOBJECTBUILDER(SubjectConfirmation);
IMPL_XMLOBJECTBUILDER(SubjectConfirmationData);
IMPL_XMLOBJECTBUILDER(SubjectLocality);

// Unicode literals
const XMLCh Action::LOCAL_NAME[] =                  UNICODE_LITERAL_6(A,c,t,i,o,n);
const XMLCh Action::TYPE_NAME[] =                   UNICODE_LITERAL_10(A,c,t,i,o,n,T,y,p,e);
const XMLCh Action::NAMESPACE_ATTRIB_NAME[] =       UNICODE_LITERAL_9(N,a,m,e,s,p,a,c,e);
const XMLCh Advice::LOCAL_NAME[] =                  UNICODE_LITERAL_6(A,d,v,i,c,e);
const XMLCh Advice::TYPE_NAME[] =                   UNICODE_LITERAL_10(A,d,v,i,c,e,T,y,p,e);
const XMLCh Assertion::LOCAL_NAME[] =               UNICODE_LITERAL_9(A,s,s,e,r,t,i,o,n);
const XMLCh Assertion::TYPE_NAME[] =                UNICODE_LITERAL_13(A,s,s,e,r,t,i,o,n,T,y,p,e);
const XMLCh Assertion::MINORVERSION_ATTRIB_NAME[] = UNICODE_LITERAL_12(M,i,n,o,r,V,e,r,s,i,o,n);
const XMLCh Assertion::ASSERTIONID_ATTRIB_NAME[] =  UNICODE_LITERAL_11(A,s,s,e,r,t,i,o,n,I,D);
const XMLCh Assertion::ISSUER_ATTRIB_NAME[] =       UNICODE_LITERAL_6(I,s,s,u,e,r);
const XMLCh Assertion::ISSUEINSTANT_ATTRIB_NAME[] = UNICODE_LITERAL_12(I,s,s,u,e,I,n,s,t,a,n,t);
const XMLCh AssertionIDReference::LOCAL_NAME[] =    UNICODE_LITERAL_20(A,s,s,e,r,t,i,o,n,I,D,R,e,f,e,r,e,n,c,e);
const XMLCh Attribute::LOCAL_NAME[] =               UNICODE_LITERAL_9(A,t,t,r,i,b,u,t,e);
const XMLCh Attribute::TYPE_NAME[] =                UNICODE_LITERAL_13(A,t,t,r,i,b,u,t,e,T,y,p,e);
const XMLCh AttributeDesignator::LOCAL_NAME[] =     UNICODE_LITERAL_19(A,t,t,r,i,b,u,t,e,D,e,s,i,g,n,a,t,o,r);
const XMLCh AttributeDesignator::TYPE_NAME[] =      UNICODE_LITERAL_23(A,t,t,r,i,b,u,t,e,D,e,s,i,g,n,a,t,o,r,T,y,p,e);
const XMLCh AttributeDesignator::ATTRIBUTENAME_ATTRIB_NAME[] =              UNICODE_LITERAL_13(A,t,t,r,i,b,u,t,e,N,a,m,e);
const XMLCh AttributeDesignator::ATTRIBUTENAMESPACE_ATTRIB_NAME[] =         UNICODE_LITERAL_18(A,t,t,r,i,b,u,t,e,N,a,m,e,s,p,a,c,e);
const XMLCh AttributeStatement::LOCAL_NAME[] =      UNICODE_LITERAL_18(A,t,t,r,i,b,u,t,e,S,t,a,t,e,m,e,n,t);
const XMLCh AttributeStatement::TYPE_NAME[] =       UNICODE_LITERAL_22(A,t,t,r,i,b,u,t,e,S,t,a,t,e,m,e,n,t,T,y,p,e);
const XMLCh AttributeValue::LOCAL_NAME[] =          UNICODE_LITERAL_14(A,t,t,r,i,b,u,t,e,V,a,l,u,e);
const XMLCh Audience::LOCAL_NAME[] =                UNICODE_LITERAL_8(A,u,d,i,e,n,c,e);
const XMLCh AudienceRestrictionCondition::LOCAL_NAME[] =    UNICODE_LITERAL_28(A,u,d,i,e,n,c,e,R,e,s,t,r,i,c,t,i,o,n,C,o,n,d,i,t,i,o,n);
const XMLCh AudienceRestrictionCondition::TYPE_NAME[] =     UNICODE_LITERAL_32(A,u,d,i,e,n,c,e,R,e,s,t,r,i,c,t,i,o,n,C,o,n,d,i,t,i,o,n,T,y,p,e);
const XMLCh AuthenticationStatement::LOCAL_NAME[] = UNICODE_LITERAL_23(A,u,t,h,e,n,t,i,c,a,t,i,o,n,S,t,a,t,e,m,e,n,t);
const XMLCh AuthenticationStatement::TYPE_NAME[] =  UNICODE_LITERAL_27(A,u,t,h,e,n,t,i,c,a,t,i,o,n,S,t,a,t,e,m,e,n,t,T,y,p,e);
const XMLCh AuthenticationStatement::AUTHENTICATIONMETHOD_ATTRIB_NAME[] =   UNICODE_LITERAL_20(A,u,t,h,e,n,t,i,c,a,t,i,o,n,M,e,t,h,o,d);
const XMLCh AuthenticationStatement::AUTHENTICATIONINSTANT_ATTRIB_NAME[] =  UNICODE_LITERAL_21(A,u,t,h,e,n,t,i,c,a,t,i,o,n,I,n,s,t,a,n,t);
const XMLCh AuthorityBinding::LOCAL_NAME[] =        UNICODE_LITERAL_16(A,u,t,h,o,r,i,t,y,B,i,n,d,i,n,g);
const XMLCh AuthorityBinding::TYPE_NAME[] =         UNICODE_LITERAL_20(A,u,t,h,o,r,i,t,y,B,i,n,d,i,n,g,T,y,p,e);
const XMLCh AuthorityBinding::AUTHORITYKIND_ATTRIB_NAME[] = UNICODE_LITERAL_13(A,u,t,h,o,r,i,t,y,K,i,n,d);
const XMLCh AuthorityBinding::LOCATION_ATTRIB_NAME[] =      UNICODE_LITERAL_8(L,o,c,a,t,i,o,n);
const XMLCh AuthorityBinding::BINDING_ATTRIB_NAME[] =       UNICODE_LITERAL_7(B,i,n,d,i,n,g);
const XMLCh AuthorizationDecisionStatement::LOCAL_NAME[] =  UNICODE_LITERAL_30(A,u,t,h,o,r,i,z,a,t,i,o,n,D,e,c,i,s,i,o,n,S,t,a,t,e,m,e,n,t);
const XMLCh AuthorizationDecisionStatement::TYPE_NAME[] =   UNICODE_LITERAL_34(A,u,t,h,o,r,i,z,a,t,i,o,n,D,e,c,i,s,i,o,n,S,t,a,t,e,m,e,n,t,T,y,p,e);
const XMLCh AuthorizationDecisionStatement::RESOURCE_ATTRIB_NAME[] =        UNICODE_LITERAL_8(R,e,s,o,u,r,c,e);
const XMLCh AuthorizationDecisionStatement::DECISION_ATTRIB_NAME[] =        UNICODE_LITERAL_8(D,e,c,i,s,i,o,n);
const XMLCh AuthorizationDecisionStatement::DECISION_PERMIT[] =             UNICODE_LITERAL_6(P,e,r,m,i,t);
const XMLCh AuthorizationDecisionStatement::DECISION_DENY[] =               UNICODE_LITERAL_4(D,e,n,y);
const XMLCh AuthorizationDecisionStatement::DECISION_INDETERMINATE[] =      UNICODE_LITERAL_13(I,n,d,e,t,e,r,m,i,n,a,t,e);
const XMLCh Condition::LOCAL_NAME[] =               UNICODE_LITERAL_9(C,o,n,d,i,t,i,o,n);
const XMLCh Conditions::LOCAL_NAME[] =              UNICODE_LITERAL_10(C,o,n,d,i,t,i,o,n,s);
const XMLCh Conditions::TYPE_NAME[] =               UNICODE_LITERAL_14(C,o,n,d,i,t,i,o,n,s,T,y,p,e);
const XMLCh Conditions::NOTBEFORE_ATTRIB_NAME[] =   UNICODE_LITERAL_9(N,o,t,B,e,f,o,r,e);
const XMLCh Conditions::NOTONORAFTER_ATTRIB_NAME[] =UNICODE_LITERAL_12(N,o,t,O,n,O,r,A,f,t,e,r);
const XMLCh ConfirmationMethod::LOCAL_NAME[] =      UNICODE_LITERAL_18(C,o,n,f,i,r,m,a,t,i,o,n,M,e,t,h,o,d);
const XMLCh DoNotCacheCondition::LOCAL_NAME[] =     UNICODE_LITERAL_19(D,o,N,o,t,C,a,c,h,e,C,o,n,d,i,t,i,o,n);
const XMLCh DoNotCacheCondition::TYPE_NAME[] =      UNICODE_LITERAL_23(D,o,N,o,t,C,a,c,h,e,C,o,n,d,i,t,i,o,n,T,y,p,e);
const XMLCh Evidence::LOCAL_NAME[] =                UNICODE_LITERAL_8(E,v,i,d,e,n,c,e);
const XMLCh Evidence::TYPE_NAME[] =                 UNICODE_LITERAL_12(E,v,i,d,e,n,c,e,T,y,p,e);
const XMLCh NameIdentifier::LOCAL_NAME[] =          UNICODE_LITERAL_14(N,a,m,e,I,d,e,n,t,i,f,i,e,r);
const XMLCh NameIdentifier::TYPE_NAME[] =           UNICODE_LITERAL_18(N,a,m,e,I,d,e,n,t,i,f,i,e,r,T,y,p,e);
const XMLCh NameIdentifier::NAMEQUALIFIER_ATTRIB_NAME[] =   UNICODE_LITERAL_13(N,a,m,e,Q,u,a,l,i,f,i,e,r);
const XMLCh NameIdentifier::FORMAT_ATTRIB_NAME[] =  UNICODE_LITERAL_6(F,o,r,m,a,t);
const XMLCh Statement::LOCAL_NAME[] =               UNICODE_LITERAL_9(S,t,a,t,e,m,e,n,t);
const XMLCh Subject::LOCAL_NAME[] =                 UNICODE_LITERAL_7(S,u,b,j,e,c,t);
const XMLCh Subject::TYPE_NAME[] =                  UNICODE_LITERAL_11(S,u,b,j,e,c,t,T,y,p,e);
const XMLCh SubjectConfirmation::LOCAL_NAME[] =     UNICODE_LITERAL_19(S,u,b,j,e,c,t,C,o,n,f,i,r,m,a,t,i,o,n);
const XMLCh SubjectConfirmation::TYPE_NAME[] =      UNICODE_LITERAL_23(S,u,b,j,e,c,t,C,o,n,f,i,r,m,a,t,i,o,n,T,y,p,e);
const XMLCh SubjectConfirmationData::LOCAL_NAME[] = UNICODE_LITERAL_23(S,u,b,j,e,c,t,C,o,n,f,i,r,m,a,t,i,o,n,D,a,t,a);
const XMLCh SubjectLocality::LOCAL_NAME[] =         UNICODE_LITERAL_15(S,u,b,j,e,c,t,L,o,c,a,l,i,t,y);
const XMLCh SubjectLocality::TYPE_NAME[] =          UNICODE_LITERAL_19(S,u,b,j,e,c,t,L,o,c,a,l,i,t,y,T,y,p,e);
const XMLCh SubjectLocality::IPADDRESS_ATTRIB_NAME[] =      UNICODE_LITERAL_9(I,P,A,d,d,r,e,s,s);
const XMLCh SubjectLocality::DNSADDRESS_ATTRIB_NAME[] =     UNICODE_LITERAL_10(D,N,S,A,d,d,r,e,s,s);

#define XCH(ch) chLatin_##ch
#define XNUM(d) chDigit_##d