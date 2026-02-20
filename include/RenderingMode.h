#pragma once

#include <GLFW/glfw3.h>

namespace nfx
{
    /**
     * @brief Rendering mode abstraction for GLFW event handling
     * @details Supports three modes:
     *          - EventDriven: glfwWaitEvents() - waits indefinitely for events, ~0% GPU at rest
     *          - Adaptive:    glfwWaitEventsTimeout() - wakes on events or after a timeout (~10fps
     *                         cap at rest), balances responsiveness and GPU usage
     *          - Polling:     glfwPollEvents() - continuous polling, highest CPU/GPU usage
     */
    class RenderingMode
    {
    public:
        enum class Mode
        {
            Adaptive,    ///< Adaptive: wake on events or timeout, low GPU at rest (glfwWaitEventsTimeout)
            EventDriven, ///< Event-driven: wait indefinitely for events (glfwWaitEvents)
            Polling      ///< Polling: continuous polling (glfwPollEvents)
        };

        explicit RenderingMode( Mode mode = Mode::Adaptive )
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
            else if( m_mode == Mode::Adaptive )
            {
                glfwWaitEventsTimeout( k_adaptiveTimeoutSeconds );
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
            switch( m_mode )
            {
                case Mode::Adaptive:
                    return "Adaptive";
                case Mode::EventDriven:
                    return "Event-driven";
                case Mode::Polling:
                    return "Polling";
            }
            return "";
        }

    private:
        static constexpr double k_adaptiveTimeoutSeconds = 0.1; ///< Max wait in Adaptive mode (~10fps at rest)

        Mode m_mode;
    };
} // namespace nfx
