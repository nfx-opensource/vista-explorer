#include "ProjectSerializer.h"

#include <fstream>
#include <sstream>
#include <string>

namespace nfx::vista
{
    std::filesystem::path ProjectSerializer::defaultDir()
    {
        std::filesystem::path home;

#if defined( _WIN32 )
        const char* userProfile = std::getenv( "USERPROFILE" );
        if( userProfile )
        {
            home = userProfile;
        }
        else
        {
            const char* homeDrive = std::getenv( "HOMEDRIVE" );
            const char* homePath = std::getenv( "HOMEPATH" );
            if( homeDrive && homePath )
            {
                home = std::string( homeDrive ) + homePath;
            }
        }
#else
        const char* homeEnv = std::getenv( "HOME" );
        if( homeEnv )
        {
            home = homeEnv;
        }
#endif

        if( home.empty() )
        {
            home = std::filesystem::temp_directory_path();
        }

        std::filesystem::path dir;

#if defined( _WIN32 )
        const char* appData = std::getenv( "APPDATA" );
        if( appData )
        {
            dir = std::filesystem::path( appData ) / APP_NAME;
        }
        else
        {
            dir = home / "AppData" / "Roaming" / APP_NAME;
        }
#else
        dir = home / ".config" / APP_NAME;
#endif

        std::filesystem::create_directories( dir );
        return dir;
    }

    std::optional<Project> ProjectSerializer::load( const std::string& path )
    {
        std::ifstream f( path );
        if( !f )
        {
            return std::nullopt;
        }

        std::ostringstream ss;
        ss << f.rdbuf();

        nfx::json::Document doc;
        if( !nfx::json::Document::fromString( ss.str(), doc ) )
        {
            return std::nullopt;
        }

        // --- shipId (mandatory) ---
        const auto* shipIdNode = doc.find( "shipId" );
        if( !shipIdNode )
        {
            return std::nullopt;
        }

        auto shipIdStr = shipIdNode->root<std::string>();
        if( !shipIdStr )
        {
            return std::nullopt;
        }

        auto shipId = dnv::vista::sdk::ShipId::fromString( *shipIdStr );
        if( !shipId )
        {
            return std::nullopt;
        }

        // --- name ---
        std::string name;
        if( const auto* n = doc.find( "name" ) )
        {
            if( auto v = n->root<std::string>() )
            {
                name = std::move( *v );
            }
        }

        // Construct with mandatory fields
        Project out{ std::move( name ), std::move( *shipId ) };
        out.filePath = path;
        out.isDirty = false;

        // --- particulars (optional block) ---
        const auto* part = doc.find( "particulars" );
        if( part )
        {
            auto& s = out.particulars;

            auto readStr = [&]( const char* key, std::string& dest ) {
                if( const auto* node = part->find( key ) )
                {
                    if( auto v = node->root<std::string>() )
                    {
                        dest = std::move( *v );
                    }
                }
            };

            auto readDouble = [&]( const char* key, std::optional<double>& dest ) {
                if( const auto* node = part->find( key ) )
                {
                    if( auto v = node->root<double>() )
                    {
                        dest = *v;
                    }
                }
            };

            auto readInt = [&]( const char* key, std::optional<int>& dest ) {
                if( const auto* node = part->find( key ) )
                {
                    if( auto v = node->root<int64_t>() )
                    {
                        dest = static_cast<int>( *v );
                    }
                }
            };

            readStr( "vesselName", s.vesselName );
            readStr( "callSign", s.callSign );
            readStr( "mmsi", s.mmsi );
            readStr( "flagState", s.flagState );
            readStr( "portOfRegistry", s.portOfRegistry );
            readStr( "owner", s.owner );
            readStr( "vesselOperator", s.vesselOperator );
            readStr( "classificationSociety", s.classificationSociety );
            readStr( "shipType", s.shipType );
            readStr( "propulsionType", s.propulsionType );
            readStr( "propellerType", s.propellerType );
            readInt( "propellerCount", s.propellerCount );
            readDouble( "mcr", s.mcr );
            readDouble( "serviceSpeed", s.serviceSpeed );
            readDouble( "depth", s.depth );
            readDouble( "draft", s.draft );
            readDouble( "grossTonnage", s.grossTonnage );
            readDouble( "deadweight", s.deadweight );
            readDouble( "netTonnage", s.netTonnage );
        }

        return out;
    }

    bool ProjectSerializer::save( const Project& p, const std::string& path )
    {
        nfx::json::Builder b( { .indent = 2 } );

        b.writeStartObject();

        b.write( "name", p.name );
        b.write( "shipId", p.shipId.toString() );

        // --- particulars ---
        b.writeKey( "particulars" );
        b.writeStartObject();
        {
            const auto& s = p.particulars;

            b.write( "vesselName", s.vesselName );
            b.write( "callSign", s.callSign );
            b.write( "mmsi", s.mmsi );
            b.write( "flagState", s.flagState );
            b.write( "portOfRegistry", s.portOfRegistry );
            b.write( "owner", s.owner );
            b.write( "vesselOperator", s.vesselOperator );
            b.write( "classificationSociety", s.classificationSociety );
            b.write( "shipType", s.shipType );
            b.write( "propulsionType", s.propulsionType );
            b.write( "propellerType", s.propellerType );

            if( s.propellerCount )
            {
                b.write( "propellerCount", *s.propellerCount );
            }
            else
            {
                b.write( "propellerCount", nullptr );
            }

            if( s.mcr )
            {
                b.write( "mcr", *s.mcr );
            }
            else
            {
                b.write( "mcr", nullptr );
            }

            if( s.serviceSpeed )
            {
                b.write( "serviceSpeed", *s.serviceSpeed );
            }
            else
            {
                b.write( "serviceSpeed", nullptr );
            }

            if( s.depth )
            {
                b.write( "depth", *s.depth );
            }
            else
            {
                b.write( "depth", nullptr );
            }

            if( s.draft )
            {
                b.write( "draft", *s.draft );
            }
            else
            {
                b.write( "draft", nullptr );
            }

            if( s.grossTonnage )
            {
                b.write( "grossTonnage", *s.grossTonnage );
            }
            else
            {
                b.write( "grossTonnage", nullptr );
            }

            if( s.deadweight )
            {
                b.write( "deadweight", *s.deadweight );
            }
            else
            {
                b.write( "deadweight", nullptr );
            }

            if( s.netTonnage )
            {
                b.write( "netTonnage", *s.netTonnage );
            }
            else
            {
                b.write( "netTonnage", nullptr );
            }
        }
        b.writeEndObject(); // particulars

        b.writeEndObject(); // root

        std::ofstream f( path );
        if( !f )
        {
            return false;
        }

        f << b.toString();
        return f.good();
    }
} // namespace nfx::vista
