#pragma once

#include "Project.h"
#include "ProjectSerializer.h"

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

        std::optional<Project> m_activeProject;
        std::function<void()> m_onChanged;

        // New project dialog state
        bool m_showNewDialog    = false;
        std::string m_newName;
        std::string m_newShipId;
        bool m_newShipIdValid   = false;
        std::string m_newShipIdError;

        // Open dialog state
        bool m_showOpenDialog = false;
        std::string m_openPath;

        // Save As dialog state
        bool m_showSaveAsDialog = false;
        std::string m_saveAsPath;

        // Status bar
        std::string m_statusMessage;
        bool m_statusIsError = false;
    };
} // namespace nfx::vista
