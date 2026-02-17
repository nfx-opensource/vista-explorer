#pragma once

#include <GLFW/glfw3.h>

namespace nfx
{
    /**
     * @brief Rendering mode abstraction for GLFW event handling
     * @details Supports two modes:
     *          - EventDriven: glfwWaitEvents() - waits for events, saves CPU
     *          - Polling: glfwPollEvents() - continuous polling, higher CPU but responsive
     */
    class RenderingMode
    {
    public:
        enum class Mode
        {
            EventDriven, ///< Event-driven: wait for events (glfwWaitEvents)
            Polling      ///< Polling: continuous polling (glfwPollEvents)
        };

        explicit RenderingMode( Mode mode = Mode::EventDriven )
            : m_mode{ mode }
        {
        }

        /**
         * @brief Get the current rendering mode
         */
        Mode mode() const
        {
            return m_mode;
        }

        /**
         * @brief Set the rendering mode
         */
        void setMode( Mode mode )
        {
            m_mode = mode;
        }

        /**
         * @brief Wait for or poll events based on current mode
         * @details Call this in the main loop
         */
        void waitOrPollEvents() const
        {
            if( m_mode == Mode::EventDriven )
            {
                glfwWaitEvents();
            }
            else
            {
                glfwPollEvents();
            }
        }

        /**
         * @brief Notify that a change occurred and rendering is needed
         * @details In EventDriven mode, posts an empty event to wake up glfwWaitEvents()
         *          In Polling mode, does nothing (already polling continuously)
         */
        void notifyChange() const
        {
            if( m_mode == Mode::EventDriven )
            {
                glfwPostEmptyEvent();
            }
            // In Polling mode, no need to do anything
        }

        /**
         * @brief Get mode name as string
         */
        const char* modeName() const
        {
            return m_mode == Mode::EventDriven ? "Event-driven" : "Polling";
        }

    private:
        Mode m_mode;
    };

} // namespace nfx
