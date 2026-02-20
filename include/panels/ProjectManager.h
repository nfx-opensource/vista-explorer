#pragma once

#include "Project.h"
#include "ProjectSerializer.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace nfx::vista
{
    class ProjectManager
    {
    public:
        ProjectManager();

        void render();

        bool hasActiveProject() const
        {
            return m_activeProject.has_value();
        }
        const Project* activeProject() const;
        Project* activeProject();

        void setChangeNotifier( std::function<void()> notifier )
        {
            m_onChanged = std::move( notifier );
        }

    private:
        void renderToolbar();
        void renderNewProjectDialog();
        void renderOpenDialog();
        void renderSaveAsDialog();
        void renderProjectInfo();
        void renderParticulars();
        void renderStatusBar();

        void doSave( const std::string& path );
        void doLoad( const std::string& path );
        void notifyChanged();

        // File browser helpers
        struct BrowserEntry
        {
            std::string name;
            std::filesystem::path fullPath;
            bool isDirectory = false;
        };

        void refreshBrowserEntries();

        std::optional<Project> m_activeProject;
        std::function<void()> m_onChanged;

        // New project dialog state
        bool m_showNewDialog = false;
        std::string m_newName;
        std::string m_newShipId;
        bool m_newShipIdValid = false;
        std::string m_newShipIdError;

        // Open dialog state
        bool m_showOpenDialog = false;
        std::string m_openPath;

        // Save As dialog state
        bool m_showSaveAsDialog = false;
        std::string m_saveAsPath;

        // Shared file browser state (used by Open and Save As)
        std::filesystem::path m_browserCurrentDir;
        std::vector<BrowserEntry> m_browserEntries;
        std::string m_browserFileName; // editable filename (Save As) or selected path (Open)
        bool m_browserDirty = true;    // true = needs refreshBrowserEntries()

        // Status bar
        std::string m_statusMessage;
        bool m_statusIsError = false;
    };
} // namespace nfx::vista
