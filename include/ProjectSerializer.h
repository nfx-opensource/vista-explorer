#pragma once

#include "Project.h"

#include <nfx/Json.h>
#include <dnv/vista/sdk/transport/ShipId.h>

#include <filesystem>

namespace nfx::vista
{
    class ProjectSerializer
    {
    public:
        static std::filesystem::path defaultDir();
        static bool load( const std::string& path, Project& out );
        static bool save( const Project& p, const std::string& path );
    };
} // namespace nfx::vista
