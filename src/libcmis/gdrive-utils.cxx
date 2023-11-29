/* libcmis
 * Version: MPL 1.1 / GPLv2+ / LGPLv2+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2013 Cao Cuong Ngo <cao.cuong.ngo@gmail.com>
 *
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPLv2+"), or
 * the GNU Lesser General Public License Version 2 or later (the "LGPLv2+"),
 * in which case the provisions of the GPLv2+ or the LGPLv2+ are applicable
 * instead of those above.
 */

#include "gdrive-utils.hxx"

#include <libcmis/xml-utils.hxx>

#include "json-utils.hxx"

using namespace std;
using libcmis::PropertyPtrMap;

string GdriveUtils::toCmisKey( const string& key )
{
    string convertedKey;
    if ( key == "id")
        convertedKey = "cmis:objectId";
    else if ( key == "ownerNames" )
        convertedKey = "cmis:createdBy";
    else if ( key == "description" )
        convertedKey = "cmis:description";
    else if ( key == "createdTime" )
        convertedKey = "cmis:creationDate";
    else if ( key == "lastModifyingUserName" )
        convertedKey = "cmis:lastModifiedBy";
    else if ( key == "modifiedTime" )
        convertedKey = "cmis:lastModificationDate";
    else if ( key == "name" )
        convertedKey = "cmis:contentStreamFileName";
    else if ( key == "mimeType" )
        convertedKey = "cmis:contentStreamMimeType";
    else if ( key == "size" )
        convertedKey = "cmis:contentStreamLength";
    else if ( key == "editable" )
        convertedKey = "cmis:isImmutable";
    else if ( key == "parents" )
        convertedKey = "cmis:parentId";
    else convertedKey = key;
    return convertedKey;
}

string GdriveUtils::toGdriveKey( const string& key )
{
    string convertedKey;
    if ( key == "cmis:objectId")
        convertedKey = "id";
    else if ( key == "cmis:createdBy" )
        convertedKey = "ownerNames";
    else if ( key == "cmis:creationDate" )
        convertedKey = "createdTime";
    else if ( key == "cmis:description" )
        convertedKey = "description";
    else if ( key == "cmis:lastModifiedBy" )
        convertedKey = "lastModifyingUserName";
    else if ( key == "cmis:lastModificationDate" )
        convertedKey = "modifiedTime";
    else if ( key == "cmis:contentStreamFileName" )
        convertedKey = "name";
    else if ( key == "cmis:name" )
        convertedKey = "name";
    else if ( key == "cmis:contentStreamMimeType" )
        convertedKey = "mimeType";
    else if ( key == "cmis:contentStreamLength" )
        convertedKey = "size";
    else if ( key == "cmis:isImmutable" )
        convertedKey = "editable";
    else if ( key == "cmis:parentId" )
        convertedKey = "parents";
    else convertedKey = key;
    return convertedKey;
}

Json GdriveUtils::toGdriveJson( const PropertyPtrMap& properties )
{
    Json propsJson;
    
    // check if cmis:name and cmis:contentStreamFileName has been duplicated
    bool duplicated = false;
    for ( PropertyPtrMap::const_iterator it = properties.begin() ; 
            it != properties.end() ; ++it )
    {
        string key = it->first;
        Json value( it->second );
        
        // Convert the key back to the gdrive key
        // take one of the two: cmis:name and cmis:contentStreamFileName
        const bool isName = key == "cmis:name" || key == "cmis:contentStreamFileName";

        if ( !isName || !duplicated )
            propsJson.add( toGdriveKey( key ), value );

        if ( isName )
            duplicated = true;
    }

    return propsJson;
} 

bool GdriveUtils::checkUpdatable( const string& key )
{
    // taken from https://developers.google.com/drive/v2/reference/files
    bool updatable = ( key == "name" ||
                  key == "description" ||
                  key == "modifiedTime" ||
                  key == "lastViewedByMeDate" );
    return updatable;    
}

bool GdriveUtils::checkMultiValued( const string& key )
{
    bool bMultiValued = ( key == "parents" ||
                  key == "exportLinks" ||
                  key == "labels" ||
                  key == "ownersName" ||
                  key == "owners");
    return bMultiValued;    
}

Json GdriveUtils::createJsonFromParentId( const string& parentId )
{
    // parents is a Json array
    Json firstParent;
    firstParent.add( Json( parentId.c_str() ) );
    
    return firstParent;
}

vector< string > GdriveUtils::parseGdriveProperty( string key, Json json )
{
    vector< string > values;
    if ( key == "owners" )
    {
        Json::JsonVector owners = json.getList( );
        for ( Json::JsonVector::iterator it = owners.begin( ); 
                it != owners.end( ); ++it )
        {
            string ownerName = ( *it )["displayName"].toString( );
            values.push_back( ownerName);
        }
    }
    else if ( key == "lastModifyingUser" )
    {
        string ownerName = json["displayName"].toString( );
        values.push_back( ownerName);
    }
    else if ( key == "userPermission" )
    {
        string ownerName = json["role"].toString( );
        values.push_back( ownerName);
    }
    else if ( key == "ownerNames" )
    {
        Json::JsonVector owners = json.getList( );
        for ( Json::JsonVector::iterator it = owners.begin( ); 
                it != owners.end( ); ++it )
        {
            string ownerName = ( *it )[""].toString( );
            values.push_back( ownerName);
        }
    }
    else if ( key == "parents" )
    {
        Json::JsonVector owners = json.getList( );
        for ( Json::JsonVector::iterator it = owners.begin( ); 
                it != owners.end( ); ++it )
        {
            string ownerName = ( *it )["id"].toString( );
            values.push_back( ownerName);
        }
    }
    else if ( key == "exportLinks" )
    {
        Json::JsonObject exportLinks = json.getObjects( );
        for ( Json::JsonObject::iterator it = exportLinks.begin( ); 
                it != exportLinks.end( ); ++it )
        {
            string mimeType = it->first;
            string link = it->second.toString( );
            values.push_back( mimeType + ":\"" + link +"\"");
        }
    }
    else if ( key == "labels" )
    {
        Json::JsonObject labels = json.getObjects( );
        for ( Json::JsonObject::iterator it = labels.begin( ); 
                it != labels.end( ); ++it )
        {
            string label = it->first;
            string isSet = it->second.toString( );
            values.push_back( label + ": " + isSet );
        }
    }
    else values.push_back( json.toString( ) );
    return values;
}

