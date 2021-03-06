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
 * @file saml/binding/ArtifactMap.h
 * 
 * Helper class for SAMLArtifact mapping and retrieval.
 */

#ifndef __saml_artmap_h__
#define __saml_artmap_h__

#include <saml/base.h>

#include <string>
#include <memory>
#include <xercesc/dom/DOM.hpp>

namespace xmltooling {
    class XMLTOOL_API StorageService;
    class XMLTOOL_API XMLObject;
};

namespace opensaml {

    class SAML_API SAMLArtifact;
    class SAML_DLLLOCAL ArtifactMappings;
    
#if defined (_MSC_VER)
    #pragma warning( push )
    #pragma warning( disable : 4251 )
#endif

    /**
     * Helper class for SAMLArtifact mapping and retrieval.
     */
    class SAML_API ArtifactMap
    {
        MAKE_NONCOPYABLE(ArtifactMap);
    public:
        
        /**
         * Creates a map on top of a particular storage service context, or in-memory.
         * 
         * @param storage       pointer to a StorageService, or nullptr to keep map in memory
         * @param context       optional label for storage context
         * @param artifactTTL   time to live in seconds, determines how long artifact remains valid
         */
        ArtifactMap(xmltooling::StorageService* storage=nullptr, const char* context=nullptr, unsigned int artifactTTL=180);

        /**
         * Creates a map on top of a particular storage service context, or in-memory.
         * 
         * @param e         root of a DOM with optional XML attributes for context and artifactTTL
         * @param storage   pointer to a StorageService, or nullptr to keep map in memory
         */
        ArtifactMap(const xercesc::DOMElement* e, xmltooling::StorageService* storage=nullptr);

        virtual ~ArtifactMap();
        
        /**
         * Associates XML content with an artifact and optionally a specific relying party.
         * Specifying no relying party means that the first attempt to resolve the artifact
         * will succeed. The XML content cannot have a parent object, and any existing references
         * to the content will be invalidated.
         * 
         * @param content       the XML content to map to an artifact
         * @param artifact      the artifact representing the XML content
         * @param relyingParty  entityID of the party authorized to resolve the artifact
         * @return the generated artifact
         */
        virtual void storeContent(xmltooling::XMLObject* content, const SAMLArtifact* artifact, const char* relyingParty=nullptr);
        
        /**
         * Retrieves the XML content represented by the artifact. The identity of the
         * relying party can be supplied, if known. If the wrong party tries to resolve
         * an artifact, an exception will be thrown and the mapping will be removed.
         * The caller is responsible for freeing the XML content returned.
         * 
         * @param artifact      the artifact representing the XML content
         * @param relyingParty  entityID of the party trying to resolve the artifact
         * @return the XML content
         */
        virtual xmltooling::XMLObject* retrieveContent(const SAMLArtifact* artifact, const char* relyingParty=nullptr);

        /**
         * Retrieves the relying party to whom the artifact was issued.
         *
         * @param artifact  the artifact to check
         * @return  entityID of the party to whom the artifact was issued, if any
         */
        virtual std::string getRelyingParty(const SAMLArtifact* artifact);

    private:
        xmltooling::StorageService* m_storage;
        std::string m_context;
        std::auto_ptr<ArtifactMappings> m_mappings;
        unsigned int m_artifactTTL;
    };

#if defined (_MSC_VER)
    #pragma warning( pop )
#endif

};

#endif /* __saml_artmap_h__ */
