/*
 *  Copyright 2010 Internet2
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
 * DiscoverableMetadataProvider.cpp
 *
 * A metadata provider that provides a JSON feed of IdP discovery information.
 */

#include "internal.h"
#include "binding/SAMLArtifact.h"
#include "saml2/metadata/Metadata.h"
#include "saml2/metadata/DiscoverableMetadataProvider.h"

#include <fstream>
#include <xmltooling/logging.h>
#include <xmltooling/XMLToolingConfig.h>

using namespace opensaml::saml2md;
using namespace xmltooling;
using namespace std;

namespace {
    void disco(string& s, const EntityDescriptor* entity) {
        if (entity) {
            const vector<IDPSSODescriptor*>& idps = entity->getIDPSSODescriptors();
            if (!idps.empty()) {
                auto_ptr_char entityid(entity->getEntityID());
                // Open a struct and output id: entityID.
                s += "{\n \"id\": \"";
                s += entityid.get();
                s += '\"';
                for (vector<IDPSSODescriptor*>::const_iterator idp = idps.begin(); idp != idps.end(); ++idp) {
                    if ((*idp)->getExtensions()) {
                        const vector<XMLObject*>& exts =  const_cast<const Extensions*>((*idp)->getExtensions())->getUnknownXMLObjects();
                        for (vector<XMLObject*>::const_iterator ext = exts.begin(); ext != exts.end(); ++ext) {
                            const UIInfo* info = dynamic_cast<UIInfo*>(*ext);
                            if (info) {
                                const vector<DisplayName*>& dispnames = info->getDisplayNames();
                                if (!dispnames.empty()) {
                                    s += ",\n \"names\": [";
                                    for (vector<DisplayName*>::const_iterator dispname = dispnames.begin(); dispname != dispnames.end(); ++dispname) {
                                        if (dispname != dispnames.begin())
                                            s += ',';
                                        auto_ptr_char dn((*dispname)->getName());
                                        auto_ptr_char lang((*dispname)->getLang());
                                        s += "\n  {\n  \"name\": \"";
                                        s += dn.get();
                                        s += "\",\n  \"lang\": \"";
                                        s += lang.get();
                                        s += "\"\n  }";
                                    }
                                    s += "\n ]";
                                }

                                const vector<Description*>& descs = info->getDescriptions();
                                if (!descs.empty()) {
                                    s += ",\n \"descs\": [";
                                    for (vector<Description*>::const_iterator desc = descs.begin(); desc != descs.end(); ++desc) {
                                        if (desc != descs.begin())
                                            s += ',';
                                        auto_ptr_char d((*desc)->getDescription());
                                        auto_ptr_char lang((*desc)->getLang());
                                        s += "\n  {\n  \"desc\": \"";
                                        s += d.get();
                                        s += "\",\n  \"lang\": \"";
                                        s += lang.get();
                                        s += "\"\n  }";
                                    }
                                    s += "\n ]";
                                }

                                const vector<InformationURL*>& infurls = info->getInformationURLs();
                                if (!infurls.empty()) {
                                    s += ",\n \"infolinks\": [";
                                    for (vector<InformationURL*>::const_iterator infurl = infurls.begin(); infurl != infurls.end(); ++infurl) {
                                        if (infurl != infurls.begin())
                                            s += ',';
                                        auto_ptr_char iu((*infurl)->getURL());
                                        auto_ptr_char lang((*infurl)->getLang());
                                        s += "\n  {\n  \"url\": \"";
                                        s += iu.get();
                                        s += "\",\n  \"lang\": \"";
                                        s += lang.get();
                                        s += "\"\n  }";
                                    }
                                    s += "\n ]";
                                }

                                const vector<PrivacyStatementURL*>& privs = info->getPrivacyStatementURLs();
                                if (!privs.empty()) {
                                    s += ",\n \"privlinks\": [";
                                    for (vector<PrivacyStatementURL*>::const_iterator priv = privs.begin(); priv != privs.end(); ++priv) {
                                        if (priv != privs.begin())
                                            s += ',';
                                        auto_ptr_char pu((*priv)->getURL());
                                        auto_ptr_char lang((*priv)->getLang());
                                        s += "\n  {\n  \"url\": \"";
                                        s += pu.get();
                                        s += "\",\n  \"lang\": \"";
                                        s += lang.get();
                                        s += "\"\n  }";
                                    }
                                    s += "\n ]";
                                }

                                const vector<Logo*>& logos = info->getLogos();
                                if (!logos.empty()) {
                                    s += ",\n \"logos\": [";
                                    for (vector<Logo*>::const_iterator logo = logos.begin(); logo != logos.end(); ++logo) {
                                        if (logo != logos.begin())
                                            s += ',';
                                        s += "\n  {\n";
                                        auto_ptr_char imgsrc((*logo)->getURL());
                                        s += "  \"imgsrc\": \"";
                                        s += imgsrc.get();
                                        s += "\",\n  \"height\": \"";
                                        s += (*logo)->getHeight().second;
                                        s += "\",\n  \"width\": \"";
                                        s += (*logo)->getWidth().second;
                                        s += '\"';
                                        if ((*logo)->getLang()) {
                                            auto_ptr_char lang((*logo)->getLang());
                                            s += ",\n  \"lang\": \"";
                                            s += lang.get();
                                            s += '\"';
                                        }
                                        s += "\n  }";
                                    }
                                    s += "\n ]";
                                }
                            }
                        }
                    }
                }
                // Close the struct;
                s += "\n},\n";
            }
        }
    }

    void disco(string& s, const EntitiesDescriptor* group) {
        if (group) {
            const vector<EntitiesDescriptor*>& groups = group->getEntitiesDescriptors();
            for (vector<EntitiesDescriptor*>::const_iterator i = groups.begin(); i != groups.end(); ++i)
                disco(s, *i);

            const vector<EntityDescriptor*>& sites = group->getEntityDescriptors();
            for (vector<EntityDescriptor*>::const_iterator j = sites.begin(); j != sites.end(); ++j)
                disco(s, *j);
        }
    }
}

DiscoverableMetadataProvider::DiscoverableMetadataProvider()
{
}

DiscoverableMetadataProvider::~DiscoverableMetadataProvider()
{
}

void DiscoverableMetadataProvider::generateFeed()
{
    m_feed = "[\n";
    const XMLObject* object = getMetadata();
    disco(m_feed, dynamic_cast<const EntitiesDescriptor*>(object));
    disco(m_feed, dynamic_cast<const EntityDescriptor*>(object));
    m_feed += " { }\n]\n";

    SAMLConfig::getConfig().generateRandomBytes(m_feedTag, 4);
    m_feedTag = SAMLArtifact::toHex(m_feedTag);
}

string DiscoverableMetadataProvider::getCacheTag() const
{
    return m_feedTag;
}

ostream& DiscoverableMetadataProvider::outputFeed(ostream& os) const
{
    return os << m_feed;
}
