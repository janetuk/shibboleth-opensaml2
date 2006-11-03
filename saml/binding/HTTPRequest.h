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
 * @file saml/binding/HTTPRequest.h
 * 
 * Interface to HTTP requests 
 */

#ifndef __saml_httpreq_h__
#define __saml_httpreq_h__

#include <saml/binding/GenericRequest.h>

namespace opensaml {
    
    /**
     * Interface to caller-supplied shim for accessing HTTP request context.
     * 
     * <p>To supply information from the surrounding web server environment,
     * a shim must be supplied in the form of this interface to adapt the
     * library to different proprietary server APIs.
     * 
     * <p>This interface need not be threadsafe.
     */
    class SAML_API HTTPRequest : public GenericRequest {
        MAKE_NONCOPYABLE(HTTPRequest);
    protected:
        HTTPRequest() {}
    public:
        virtual ~HTTPRequest() {}
        
        /**
         * Returns the HTTP method of the request (GET, POST, etc.)
         * 
         * @return the HTTP method
         */
        virtual const char* getMethod() const=0;
        
        /**
         * Returns the complete request URL, including scheme, host, port.
         * 
         * @return the request URL
         */
        virtual const char* getRequestURL() const=0;
        
        /**
         * Returns the HTTP query string appened to the request. The query
         * string is returned without any decoding applied, everything found
         * after the ? delimiter. 
         * 
         * @return the query string
         */
        virtual const char* getQueryString() const=0;

        /**
         * Returns a request header value.
         * 
         * @param name  the name of the header to return
         * @return the header's value, or an empty string
         */
        virtual std::string getHeader(const char* name) const=0;
    };
};

#endif /* __saml_httpreq_h__ */