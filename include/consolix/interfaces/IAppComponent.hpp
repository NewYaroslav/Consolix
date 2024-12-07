#pragma once
#ifndef _CONSOLIX_IAPPLICATION_COMPONENT_HPP_INCLUDED
#define _CONSOLIX_IAPPLICATION_COMPONENT_HPP_INCLUDED

/// \file IAppComponent.hpp
/// \brief Defines the interface for application components.

namespace consolix {

    /// \class IAppComponent
    /// \brief Interface for defining application components.
    ///
    /// Provides the foundational structure for application components with a defined lifecycle:
    /// - `initialize()`: One-time setup before execution.
    /// - `process()`: Repeated execution during the main loop.
    /// - `is_initialized()`: Verifies if the component is ready for execution.
    ///
    /// Access to lifecycle methods is restricted to the `AppComponentManager` class to ensure controlled execution.
    class IAppComponent {
        friend class AppComponentManager; ///< Grants access to `AppComponentManager`.
    public:
        /// \brief Virtual destructor for polymorphic usage.
        virtual ~IAppComponent() = default;

    protected:

        /// \brief Initializes the component.
        /// This method is called once before the component is processed.
        /// \return `true` if the initialization is successful, `false` otherwise.
        virtual bool initialize() = 0;

        /// \brief Checks if the component is initialized.
        /// Used to verify if the component is ready to be executed.
        /// \return `true` if the component is initialized, `false` otherwise.
        virtual bool is_initialized() const = 0;

        /// \brief Executes the component's main logic.
        /// Called repeatedly during the application's main execution loop.
        virtual void process() = 0;

    }; // IAppComponent

}; // namespace consolix

#endif // _CONSOLIX_IAPPLICATION_COMPONENT_HPP_INCLUDED
