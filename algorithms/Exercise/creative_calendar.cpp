#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

// Simple ANSI color codes (supported on modern terminals, including recent Windows)
const std::string RESET   = "\033[0m";
const std::string RED     = "\033[31m";
const std::string GREEN   = "\033[32m";
const std::string YELLOW  = "\033[33m";
const std::string BLUE    = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN    = "\033[36m";
const std::string BOLD    = "\033[1m";
const std::string REVERSE = "\033[7m";

bool isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int month, int year) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeap(year)) return 29;
    return days[month - 1];
}

// Zeller’s congruence: returns 0 = Sunday, 1 = Monday, ..., 6 = Saturday
int dayOfWeek(int d, int m, int y) {
    if (m < 3) {
        m += 12;
        y -= 1;
    }
    int K = y % 100;
    int J = y / 100;
    int h = (d + 13 * (m + 1) / 5 + K + K / 4 + J / 4 + 5 * J) % 7;
    int day = (h + 6) % 7; // convert to 0 = Sunday
    return day;
}

int main() {
    int month, year;
    std::cout << "Enter month (1-12): ";
    std::cin >> month;
    std::cout << "Enter year: ";
    std::cin >> year;

    // Get today's date for highlighting
    auto today     = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(today);
    std::tm local  = *std::localtime(&tt);
    int t_day   = local.tm_mday;
    int t_month = local.tm_mon + 1;
    int t_year  = local.tm_year + 1900;

    const std::string monthNames[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    int firstWeekday = dayOfWeek(1, month, year);
    int totalDays    = daysInMonth(month, year);

    std::string title = monthNames[month - 1] + " " + std::to_string(year);
    int width = 28;
    std::string border(width, '~');

    std::cout << "\n" << CYAN << border << RESET << "\n";
    std::cout << CYAN << std::setw((width + static_cast<int>(title.size())) / 2)
              << BOLD << title << RESET << "\n";
    std::cout << CYAN << border << RESET << "\n";

    // Weekday names row
    std::cout << BOLD
              << YELLOW << "Su " << GREEN  << "Mo "
              << GREEN  << "Tu " << GREEN  << "We "
              << GREEN  << "Th " << RED    << "Fr "
              << RED    << "Sa"  << RESET  << "\n";

    // Initial spaces for first week
    for (int i = 0; i < firstWeekday; ++i) {
        std::cout << "   ";
    }

    // Print days
    for (int day = 1; day <= totalDays; ++day) {
        int weekday = (firstWeekday + day - 1) % 7;

        bool isWeekend = (weekday == 0 || weekday == 6);
        bool isToday   = (day == t_day && month == t_month && year == t_year);

        std::string color;
        if (isToday) {
            color = REVERSE + BOLD + MAGENTA; // highlight today
        } else if (isWeekend) {
            color = RED;
        } else {
            color = GREEN;
        }

        std::cout << color << std::setw(2) << day << " " << RESET;

        if (weekday == 6) {
            std::cout << "\n";
        }
    }

    std::cout << "\n" << CYAN << border << RESET << "\n";

    return 0;
}

