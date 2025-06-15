//
// Main.cpp
// 
// Grant Abernathy
// 
// 06-14-2025
// 
// Entry point, EDGE_ENTRYPOINT_TEMP is a temp entry point for it to compile.
// Not really needed, but kept.
//

#define EDGE_ENTRYPOINT_TEMP 1
#if EDGE_ENTRYPOINT_TEMP 1
#include <iostream>

int main()
{
    printf("Hello.");
}
#endif

#if EDGE_TEST
#include <iostream>
#include <string>
#include <limits> // Required for numeric_limits

// This function will trigger a compile-time warning.
// Check your build log to see the output from EDGE_WARNING.
void TestWarning() {
    EDGE_WARNING("This is a test warning message.");
}

// This function will trigger a compile-time error.
// Uncomment the line below to see the output from EDGE_ERROR.
void TestError() {
    // EDGE_ERROR("This is a test error message.");
}

// This function demonstrates the assertion macro.
void TestAssert() {
    std::cout << "--- Testing Assertions ---\n";

    EDGE_ASSERT(1 == 1, "This assertion should pass and do nothing.");
    std::cout << "Successfully passed the first assertion.\n";

#if EDGE_DEBUG
    std::cout << "Running a failing assertion in Debug mode. The program should break here.\n";
    std::cout << "If you continue execution, it's because you are in a debugger.\n";
    EDGE_ASSERT(1 == 0, "This assertion will fail in Debug builds!");
#else
    std::cout << "This is a Release build. The failing assertion will be compiled out and have no effect.\n";
    EDGE_ASSERT(1 == 0, "This assertion will fail in Debug builds!");
    std::cout << "The program continued without issue after the (disabled) assertion.\n";
#endif
    std::cout << "--- End of Assertion Test ---\n";
}


void PrintMenu() {
    std::cout << "\n--- Edge Core Test Menu ---\n";
    std::cout << "1 - Compiler Test\n";
    std::cout << "2 - Compiler Version Test\n";
    std::cout << "3 - Platform / Supported Platforms Test\n";
    std::cout << "4 - Build Configuration Test\n";
    std::cout << "5 - Architecture Test\n";
    std::cout << "6 - Global Support Test\n";
    std::cout << "7 - C++ Version / Version Requirement Test\n";
    std::cout << "8 - Log Error Test (Compile-time)\n";
    std::cout << "9 - Log Warning Test (Compile-time)\n";
    std::cout << "0 - Assert Test (Runtime)\n";
    std::cout << "Enter your choice (or any other key to exit): ";
}

int main() {
    char choice;
    do {
        PrintMenu();
        std::cin >> choice;

        // Clear the input buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


        switch (choice) {
        case '1':
            std::cout << "\n--- Compiler Test ---\n";
#if EDGE_COMPILER_MSVC
            std::cout << "Compiler: MSVC\n";
#elif EDGE_COMPILER_CLANG
            std::cout << "Compiler: Clang\n";
#elif EDGE_COMPILER_GCC
            std::cout << "Compiler: GCC\n";
#else
            std::cout << "Compiler: Unknown\n";
#endif
            break;
        case '2':
            std::cout << "\n--- Compiler Version Test ---\n";
            std::cout << "Compiler Version: " << EDGE_COMPILER_VER_ << "\n";
            break;
        case '3':
            std::cout << "\n--- Platform Test ---\n";
#if EDGE_PLATFORM_WINDOWS
            std::cout << "Platform: Windows\n";
#elif EDGE_PLATFORM_MACOS
            std::cout << "Platform: macOS\n";
#elif EDGE_PLATFORM_IOS
            std::cout << "Platform: iOS\n";
#elif EDGE_PLATFORM_ANDROID
            std::cout << "Platform: Android\n";
#else
            std::cout << "Platform: Unknown\n";
#endif
            std::cout << "--- Supported Platforms (Global Config) ---\n";
            std::cout << "Windows Support: " << (EDGE_GLOBAL_WIN_SUPPORTED ? "Enabled" : "Disabled") << "\n";
            std::cout << "macOS Support:   " << (EDGE_GLOBAL_MAC_SUPPORTED ? "Enabled" : "Disabled") << "\n";
            std::cout << "iOS Support:     " << (EDGE_GLOBAL_IOS_SUPPORTED ? "Enabled" : "Disabled") << "\n";
            std::cout << "Android Support: " << (EDGE_GLOBAL_ANDROID_SUPPORTED ? "Enabled" : "Disabled") << "\n";
            break;
        case '4':
            std::cout << "\n--- Build Configuration Test ---\n";
#if EDGE_DEBUG
            std::cout << "Build: Debug\n";
#elif EDGE_RELEASE
            std::cout << "Build: Release\n";
#endif
            break;
        case '5':
            std::cout << "\n--- Architecture Test ---\n";
#if EDGE_ARCH_X64
            std::cout << "Architecture: x64\n";
#elif EDGE_ARCH_ARM64
            std::cout << "Architecture: ARM64\n";
#endif
            break;
        case '6':
            std::cout << "\n--- Global Support Test ---\n";
            std::cout << "This test passes if the program compiles successfully.\n";
            std::cout << "The '#error' directives in edge_core.h prevent compilation on unsupported platforms/compilers.\n";
            break;
        case '7':
            std::cout << "\n--- C++ Version Test ---\n";
            std::cout << "Detected __cplusplus value: " << EDGE_CPP_VERSION_LONG << "L\n";
            std::cout << "Major Version (EDGE_CPP): C++" << EDGE_CPP << "\n";
            std::cout << "--- Project Requirements ---\n";
            std::cout << "Minimum Required:  " << EDGE_GLOBAL_CPP_MIN_VERSION << "L\n";
            std::cout << "Maximum Supported: " << EDGE_GLOBAL_CPP_MAX_VERSION << "L\n";
            std::cout << "Preferred Version: " << EDGE_GLOBAL_CPP_PREFERRED_VERSION << "L\n";
            break;
        case '8':
            std::cout << "\n--- Log Error Test ---\n";
            std::cout << "This test happens at compile time.\n";
            std::cout << "To test it, uncomment the 'EDGE_ERROR' line inside the TestError() function in main.cpp and recompile.\n";
            std::cout << "The compiler should output an error message with the file and line number.\n";
            // TestError(); // Would prevent compilation
            break;
        case '9':
            std::cout << "\n--- Log Warning Test ---\n";
            std::cout << "This test happens at compile time.\n";
            std::cout << "Check your compiler's build log. You should see a warning message from the TestWarning() function.\n";
            TestWarning();
            break;
        case '0':
            TestAssert();
            break;
        default:
            std::cout << "Exiting...\n";
            return 0;
        }
    } while (true);

    return 0;
}
#endif // EDGE_TEST