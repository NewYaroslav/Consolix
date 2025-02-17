/// \example example_application_main_loop.cpp

#include <consolix/core.hpp>

/// \brief Application configuration structure.
///
/// Defines the configurable parameters for the application, loaded from
/// a JSON file or specified via command-line arguments.
struct AppConfig {
    std::string text;               ///< Text to display in each loop iteration.
    std::vector<std::string> items; ///< List of items to display.
    int period;                     ///< Delay between loop iterations in milliseconds.
    bool debug_mode;                ///< Enable or disable debugging mode.

    /// \brief Macro for JSON serialization/deserialization.
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AppConfig, text, items, period, debug_mode)
};

/// \class CustomLoop
/// \brief Custom loop component for the main application logic.
///
/// This component handles application-specific logic in three stages:
/// - Initialization (`on_once`).
/// - Repeated execution (`on_loop`).
/// - Graceful shutdown (`on_shutdown`).
class CustomLoop final : public consolix::BaseLoopComponent {
public:

    virtual ~CustomLoop() = default;

    /// \brief Called once at the start of the loop.
    bool on_once() override {
        CONSOLIX_STREAM() <<
            consolix::color(consolix::TextColor::Green) <<  "Hello, " <<
            consolix::color(consolix::TextColor::Yellow) << "world!";

        auto args = consolix::get_service<consolix::CliArguments>();
        if (args.count("help")) {
            CONSOLIX_STREAM() << consolix::get_service<consolix::CliOptions>().help();
            std::exit(0);
        }

        auto config = consolix::get_service<AppConfig>();
        CONSOLIX_SET_DEBUG_MODE(config.debug_mode);

        LOGIT_TRACE0();

        // Делаем тестовую запись в логгер с уникальными файлами
        CONSOLIX_UNIQUE_FILE_STREAM() << "Test 123";
        LOGIT_PRINT_INFO("Unique log file: ", CONSOLIX_UNIQUE_FILE_NAME());
        return true;
    }

    /// \brief Called repeatedly during the execution loop
    void on_loop() override {
        // Retrieve configuration
        const auto& config = consolix::get_service<AppConfig>();

        // Print text and items
        CONSOLIX_STREAM() << config.text;
        CONSOLIX_STREAM() << consolix::color(consolix::TextColor::Green) << "items:";
        for (auto &item : config.items) {
            CONSOLIX_STREAM() << consolix::color(consolix::TextColor::Cyan) << item;
        }

        // Sleep for the configured period
        std::this_thread::sleep_for(std::chrono::milliseconds(config.period));

        LOGIT_TRACE0();
    }

    /// \brief Called during application shutdown.
    /// \param signal The shutdown signal.
    void on_shutdown(int signal) override {
        CONSOLIX_STREAM() << "Application is shutting down. Received signal: " << signal;
    }
}; // CustomLoop

int main(int argc, char* argv[]) {
    // Set the console title to the application name
    consolix::add<consolix::TitleComponent>(u8"Consolix - консольное приложение");

    // Initialize the logger. This must be the first component.
    consolix::add<consolix::LoggerComponent>();

    // Initialize command-line argument handler. Depends on LoggerComponent.
    consolix::add<consolix::CliComponent>(
            "Consolix",
            "A demonstration program showcasing the features of the Consolix library, "
            "including logging, configuration management, and command-line argument parsing.",
            [](consolix::CliOptions& options){
        options.add_options()
        ("c,config", "Path to the configuration file", cxxopts::value<std::string>())
        ("d,debug", "Enable debugging mode", cxxopts::value<bool>()->default_value("false"))
        ("p,period", "Period in milliseconds", cxxopts::value<int>()->default_value("10"))
        ("h,help", "Show help message");
        options.allow_unrecognised_options();
    });

    // Add logo component to display the application logo at startup.
    consolix::add<consolix::LogoComponent>(consolix::TextColor::Yellow);

    // Load configuration from a JSON file.
    consolix::add<consolix::ConfigComponent<AppConfig>>(
        "config.json", "config");

    // Add the custom loop component.
    consolix::add<CustomLoop>();

    // Start the application and run all components.
    consolix::run([](){
        // UTF-8 string support for older versions of Windows
        CONSOLIX_STREAM() <<
            consolix::color(consolix::TextColor::Green) << u8"Привет, мир!";
    });
    return 0;
}
