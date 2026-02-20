#pragma once

#include "Project.h"

#include <nfx/Json.h>
#include <dnv/vista/sdk/transport/ShipId.h>

#include <filesystem>
#include <optional>

namespace nfx::vista
{
    class ProjectSerializer
    {
    public:
        static std::filesystem::path defaultDir();
        static std::optional<Project> load( const std::string& path );
        static bool save( const Project& p, const std::string& path );
    };
} // namespace nfx::vista
