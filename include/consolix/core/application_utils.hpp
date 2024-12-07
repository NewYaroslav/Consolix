#pragma once
#ifndef _CONSOLIX_APPLICATION_UTILS_HPP_INCLUDED
#define _CONSOLIX_APPLICATION_UTILS_HPP_INCLUDED

/// \file application_utils.hpp
/// \brief Utility functions for managing application lifecycle and components.
/// \ingroup Core

namespace consolix {

    /// \brief Initializes the application.
    /// This function prepares the application and service locator for use.
    void init() {
        ServiceLocator::get_instance();
        ConsoleApplication::get_instance().init();
    }

    /// \brief Initializes the application with a custom action.
    /// This function prepares the application and service locator, and executes a custom action during initialization.
    /// \tparam InitAction A callable type for the custom initialization action.
    /// \param init_action The action to execute during initialization.
    template <typename InitAction>
    void init(InitAction init_action) {
        ServiceLocator::get_instance();
        ConsoleApplication::get_instance().init(init_action);
    }

    /// \brief Adds a new component to the application.
    /// Creates a new instance of the specified component type and adds it to the application.
    /// \tparam Component The type of the component to add.
    /// \tparam Args The types of the arguments to pass to the component's constructor.
    /// \param args Arguments to construct the component.
    /// \return A `std::shared_ptr` to the added component.
    template <typename Component, typename... Args>
    inline std::shared_ptr<Component> add(Args&&... args) {
        return ConsoleApplication::get_instance().add<Component>(std::forward<Args>(args)...);
    }

    /// \brief Adds an existing component to the application.
    /// Registers a pre-existing component with the application.
    /// \param component A `std::shared_ptr` to the component to add.
    inline void add(std::shared_ptr<IAppComponent> component) {
        ConsoleApplication::get_instance().add(std::move(component));
    }

    /// \brief Runs the application.
    /// Processes all components in the application's main loop.
    inline void run() {
        ConsoleApplication::get_instance().run();
    }

    /// \brief Runs the application with a custom loop action.
    ///
    /// Executes a custom action during each iteration of the application's main loop.
    /// \tparam IterationAction A callable type for the custom loop action.
    /// \param iteration_action The action to execute in the main loop.
    template <typename IterationAction>
    inline void run(IterationAction iteration_action) {
        ConsoleApplication::get_instance().run(iteration_action);
    }

    /// \brief Stops the application.
    /// Stops the application's main loop and begins the shutdown process.
    inline void stop() {
        ConsoleApplication::get_instance().stop();
    }

    /// \example Example usage of application utilities.
    ///
    /// ```cpp
    /// // Add logo component to display the application logo at startup.
    /// consolix::add<consolix::LogoComponent>(consolix::TextColor::DarkYellow);
    ///
    /// // Load configuration from a JSON file.
    /// consolix::add<consolix::ConfigComponent<AppConfig>>("config.json", "config");
    ///
    /// // Add the custom loop component.
    /// consolix::add<CustomLoop>();
    ///
    /// // Start the application and run all components.
    /// consolix::run();
    /// ```

}; // namespace consolix

#endif // _CONSOLIX_APPLICATION_UTILS_HPP_INCLUDED
