#pragma once
#ifndef _CONSOLIX_ISHUTDOWNABLE_HPP_INCLUDED
#define _CONSOLIX_ISHUTDOWNABLE_HPP_INCLUDED

/// \file IShutdownable.hpp
/// \brief Defines the interface for components that support graceful shutdown.

namespace consolix {

    /// \class IShutdownable
    /// \brief Interface for components supporting shutdown logic.
    ///
    /// This interface defines a method for handling shutdown events,
    /// which is particularly useful for releasing resources, saving state,
    /// or performing cleanup operations when a termination signal is received.
    /// Access to the `shutdown` method is restricted to the `AppComponentManager` class.
    class IShutdownable {
        friend class AppComponentManager; ///< Grants access to the `AppComponentManager` class.

    public:
        /// \brief Virtual destructor for polymorphic use.
        virtual ~IShutdownable() = default;

    protected:
        /// \brief Handles shutdown logic for the component.
        ///
        /// This method is called when a shutdown signal is received. Derived components
        /// should implement this method to perform necessary cleanup or save state before termination.
        /// \param signal The shutdown signal (e.g., SIGINT or SIGTERM).
        virtual void shutdown(int signal) = 0;
    };

} // namespace consolix

#endif // _CONSOLIX_ISHUTDOWNABLE_HPP_INCLUDED
