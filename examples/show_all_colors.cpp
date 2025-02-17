/// \example show_all_colors.cpp
/// \brief Demonstrates displaying all available text colors in Consolix.

#include <consolix/core.hpp>

/// \class ColorDemo
/// \brief Demonstrates the output of all available colors in Consolix.
///
/// This component initializes the console and sequentially prints each color
/// defined in `consolix::TextColor`, allowing the user to see all available options.
class ColorDemo final : public consolix::BaseLoopComponent {
public:

    virtual ~ColorDemo() = default;

    /// \brief Called once at the start of the loop.
    bool on_once() override {
        // List of text colors
        std::array<consolix::TextColor, 16> colors = {
            consolix::TextColor::Black,
            consolix::TextColor::DarkRed,
            consolix::TextColor::DarkGreen,
            consolix::TextColor::DarkYellow,
            consolix::TextColor::DarkBlue,
            consolix::TextColor::DarkMagenta,
            consolix::TextColor::DarkCyan,
            consolix::TextColor::LightGray,
            consolix::TextColor::DarkGray,
            consolix::TextColor::Red,
            consolix::TextColor::Green,
            consolix::TextColor::Yellow,
            consolix::TextColor::Blue,
            consolix::TextColor::Magenta,
            consolix::TextColor::Cyan,
            consolix::TextColor::White
        };

        // Print color list
        for (size_t i = 0; i < colors.size(); ++i) {
            CONSOLIX_STREAM() << consolix::color(colors[i]) << "color " << i;
        }

        return true;
    }

    void on_loop() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    /// \brief Called during application shutdown.
    /// \param signal The shutdown signal.
    void on_shutdown(int signal) override {
        CONSOLIX_STREAM() << "Application is shutting down. Received signal: " << signal;
    }
};

/// \brief Main entry point of the program.
/// \param argc Number of command-line arguments.
/// \param argv Array of command-line argument strings.
/// \return Exit code.
int main(int argc, char* argv[]) {
    consolix::add<consolix::TitleComponent>(u8"Consolix - Color Demonstration");
    consolix::add<consolix::LoggerComponent>();
    consolix::add<ColorDemo>();
    consolix::run();
    return 0;
}
