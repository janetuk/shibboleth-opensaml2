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
 * @file saml/saml2/metadata/AbstractMetadataProvider.h
 * 
 * Base class for caching metadata providers.
 */

#ifndef __saml2_absmetadataprov_h__
#define __saml2_absmetadataprov_h__

#include <saml/saml2/metadata/ObservableMetadataProvider.h>

#include <ctime>
#include <map>
#include <vector>
#include <string>

namespace xmltooling {
    class XMLTOOL_API Credential;
    class XMLTOOL_API CredentialCriteria;
    class XMLTOOL_API KeyInfoResolver;
    class XMLTOOL_API Mutex;
};

namespace opensaml {
    namespace saml2md {
        
        class SAML_API MetadataFilter;

#if defined (_MSC_VER)
        #pragma warning( push )
        #pragma warning( disable : 4251 )
#endif

        /**
         * Base class for caching metadata providers.
         */
        class SAML_API AbstractMetadataProvider : public ObservableMetadataProvider
        {
        protected:
            /**
             * Constructor.
             * 
             * If a DOM is supplied, a set of default logic will be used to identify
             * and build a KeyInfoResolver plugin and install it into the provider.
             * 
             * The following XML content is supported:
             * 
             * <ul>
             *  <li>&lt;KeyInfoResolver&gt; elements with a type attribute
             * </ul>
             * 
             * XML namespaces are ignored in the processing of these elements.
             * 
             * @param e DOM to supply configuration for provider
             */
            AbstractMetadataProvider(const xercesc::DOMElement* e=nullptr);
            
        public:
            virtual ~AbstractMetadataProvider();
            
            using MetadataProvider::getEntityDescriptor;
            using MetadataProvider::getEntitiesDescriptor;

            void outputStatus(std::ostream& os) const;
            void emitChangeEvent() const;
            void emitChangeEvent(const EntityDescriptor&) const;
            std::pair<const EntityDescriptor*,const RoleDescriptor*> getEntityDescriptor(const Criteria& criteria) const;
            const EntitiesDescriptor* getEntitiesDescriptor(const char* name, bool requireValidMetadata=true) const;
            const xmltooling::Credential* resolve(const xmltooling::CredentialCriteria* criteria=nullptr) const;
            std::vector<const xmltooling::Credential*>::size_type resolve(
                std::vector<const xmltooling::Credential*>& results, const xmltooling::CredentialCriteria* criteria=nullptr
                ) const;

        protected:
            /** Time of last update for reporting. */
            mutable time_t m_lastUpdate;

            /** Embedded KeyInfoResolver instance. */
            xmltooling::KeyInfoResolver* m_resolver;

            /**
             * Loads an entity into the cache for faster lookup.
             * <p>This includes processing known reverse lookup strategies for artifacts.
             * The validUntil parameter will contain the smallest value found on output.
             * 
             * @param site          entity definition
             * @param validUntil    maximum expiration time of the entity definition
             * @param replace       true iff existing entries for the same entity should be cleared/replaced
             */
            virtual void indexEntity(EntityDescriptor* site, time_t& validUntil, bool replace=false) const;

            /**
             * Loads a group of entities into the cache for faster lookup.
             * <p>The validUntil parameter will contain the smallest value found on output.
             * 
             * @param group         group definition
             * @param validUntil    maximum expiration time of the group definition
             */
            virtual void indexGroup(EntitiesDescriptor* group, time_t& validUntil) const;

            /**
             * @deprecated
             * Loads an entity into the cache for faster lookup.
             * <p>This includes processing known reverse lookup strategies for artifacts.
             * 
             * @param site          entity definition
             * @param validUntil    maximum expiration time of the entity definition
             * @param replace       true iff existing entries for the same entity should be cleared/replaced
             */
            virtual void index(EntityDescriptor* site, time_t validUntil, bool replace=false) const;

            /**
             * @deprecated
             * Loads a group of entities into the cache for faster lookup.
             * 
             * @param group         group definition
             * @param validUntil    maximum expiration time of the group definition
             */
            virtual void index(EntitiesDescriptor* group, time_t validUntil) const;

            /**
             * Clear the cache of known entities and groups.
             *
             * @param freeSites true iff the objects cached in the site map should be freed.
             */
            virtual void clearDescriptorIndex(bool freeSites=false);

        private:
            typedef std::multimap<std::string,const EntityDescriptor*> sitemap_t;
            typedef std::multimap<std::string,const EntitiesDescriptor*> groupmap_t;
            mutable sitemap_t m_sites;
            mutable sitemap_t m_sources;
            mutable groupmap_t m_groups;

            std::auto_ptr<xmltooling::KeyInfoResolver> m_resolverWrapper;
            mutable std::auto_ptr<xmltooling::Mutex> m_credentialLock;
            typedef std::map< const RoleDescriptor*, std::vector<xmltooling::Credential*> > credmap_t;
            mutable credmap_t m_credentialMap;
            const credmap_t::mapped_type& resolveCredentials(const RoleDescriptor& role) const;
        };

#if defined (_MSC_VER)
        #pragma warning( pop )
        #pragma warning( disable : 4251 )
#endif
        
    };
};

#endif /* __saml2_absmetadataprov_h__ */
