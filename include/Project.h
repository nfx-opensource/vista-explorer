#pragma once

#include <dnv/vista/sdk/ImoNumber.h>
#include <dnv/vista/sdk/transport/ShipId.h>

#include <optional>
#include <string>

namespace nfx::vista
{

    struct ShipParticulars
    {
        // --- Identification ---
        std::string vesselName;     // Name of the vessel
        std::string callSign;       // Radio call sign
        std::string mmsi;           // Maritime Mobile Service Identity (9 digits)
        std::string flagState;      // Country flag (ISO 3166-1 alpha-2, e.g. "NO")
        std::string portOfRegistry; // Port of registry / home port
        std::string owner;          // Registered owner
        std::string vesselOperator; // Commercial operator

        // --- Classification & type ---
        std::string classificationSociety; // e.g. "DNV", "Lloyd's Register", "Bureau Veritas"
        std::string shipType;              // e.g. "Bulk carrier", "Tanker", "Passenger", "OSV"

        // --- Propulsion & performance ---
        std::string propulsionType;         // e.g. "Diesel", "Diesel-Electric", "LNG", "Steam turbine"
        std::string propellerType;          // e.g. "FPP", "CPP", "Azipod", "Waterjet"
        std::optional<int> propellerCount;  // number of propellers
        std::optional<double> mcr;          // Maximum Continuous Rating, kW
        std::optional<double> serviceSpeed; // knots

        // --- Dimensions ---
        std::optional<double> depth; // metres (moulded depth)
        std::optional<double> draft; // metres (design draft / scantling)

        // --- Tonnage ---
        std::optional<double> grossTonnage; // GT, dimensionless (convention measurement)
        std::optional<double> netTonnage;   // NT, dimensionless (usable capacity)
        std::optional<double> deadweight;   // DWT, metric tons
    };

    struct Project
    {
        // Human-readable workspace name
        std::string name;

        // ShipId: either a validated IMO number or an alternative identifier.
        dnv::vista::sdk::ShipId shipId;

        // Optional vessel metadata
        ShipParticulars particulars;

        // Persistence
        std::string filePath; // Empty until first save
        bool isDirty = false;
    };

} // namespace nfx::vista
