/*
* The program implements the operation of a simple parking meter. 
* It reads from input lines with data indicating a payment order or
* checking if a selected car has paid for the specified hours.
 */

#include <assert.h>
#include <iostream>
#include <queue>
#include <regex>
#include <sstream>
#include <utility>
#include <vector>

using std::cerr;
using std::cin;
using std::cout;
using std::getline;
using std::istringstream;
using std::pair;
using std::queue;
using std::regex;
using std::string;
using std::vector;

using hour_t = int16_t;
using line_count_t = size_t;
// Vector <Pair <Registration number, Pair <Start time, End time>>>
using parking_payments_vector_t = vector<pair<string, pair<hour_t, hour_t>>>;

const hour_t MINUTES_IN_HOUR = 60;
const hour_t MIN_PAYMENT_IN_MIN = 10;
const hour_t MAX_PAYMENT_IN_MIN = 719;
const hour_t OPENING_TIME_MIN = 480; 
const hour_t CLOSING_TIME_MIN = 1200; 

/*
* Using regular expressions, it checks if the passed string is a valid
* line ordering payment for parking.
* It checks if the format of the license plate and hours meets the
* task requirements and if the given hours are within [8.00, 20.00].
* Returns true if the string meets the task requirements, false otherwise.
*/
bool validate_payment_line(string const *line) {
    assert(line != NULL);

    static string const regex_pattern =
        "[\\s\\t]*[A-Z][A-Z0-9]{2,10}"
        "[\\s\\t]+((0?[8-9]|1[0-9])\\.[0-5][0-9]|20\\.00)"
        "[\\s\\t]+((0?[8-9]|1[0-9])\\.[0-5][0-9]|20\\.00)[\\s\\t]*";

    static regex const regexRule(regex_pattern);

    return regex_match(*line, regexRule);
}

/*
* Using regular expressions, it checks if the passed string is a valid
* line checking payment for parking.
* It checks if the format of the license plate and hours meets the
* task requirements and if the given hours are within [8.00, 20.00].
* Returns true if the string meets the task requirements, false otherwise.
*/
bool validate_checking_line(string const *line) {
    assert(line != NULL);

    static string const regex_pattern =
        "[\\s\\t]*[A-Z][A-Z0-9]{2,10}"
        "[\\s\\t]+((0?[8-9]|1[0-9])\\.[0-5][0-9]|20\\.00)[\\s\\t]*";

    static regex const regexRule(regex_pattern);

    return regex_match(*line, regexRule);
}

/*
 * Konwertuje łańcuch reprezentujacy datę zgodną ze specyfikacją zadania na
 * liczbę całkowitą oznaczajaca liczbę upłyniętych minut od północy.
 */

/*
* Converts a string representing a date in accordance with the task specification
* into an integer indicating the number of minutes elapsed since midnight.
*/
hour_t date_to_minutes_past_midnight(string const *date) {
    assert(date != NULL);   

    size_t date_length = (*date).length();

    hour_t hours = (hour_t) stoi((*date).substr(0, date_length - 3));
    hour_t minutes =
        (hour_t) stoi((*date).substr(date_length - 2, date_length - 1));

    return (hour_t) (MINUTES_IN_HOUR * hours + minutes);
}

/*
 * Parsuje line w formacie \s<nr rej.>\s<czas rozp.>\s<czas zak.>\s, gdzie \s
 * oznacza znaki białe.
 * Czas rozpoczęcia i zakończenia zapisywane są jako liczba upłyniętych minut
 * od północy.
 * Dane wycięte z łańcucha wpisywane są pod adresy podane jako argumenty.
 */
/*
* Parses the line in the format \s<Registration number>\s<Start time>\s<End time>,
* where \s represents whitespace characters.
* The start and end times are recorded as the number of minutes elapsed since midnight.
* The data extracted from the string is written to the addresses given as arguments. 
*/
void parse_payment_line(string const *line, string *license_plate,
                        hour_t *parking_start, hour_t *parking_end) {
    assert(line != NULL && license_plate != NULL &&
           parking_start != NULL && parking_end != NULL);

    istringstream iss(*line);
    string first_date, second_date;

    iss >> *license_plate >> first_date >> second_date;

    *parking_start = date_to_minutes_past_midnight(&first_date);
    *parking_end = date_to_minutes_past_midnight(&second_date);
}

/*
 * Parsuje line w formacie \s<nr rej.>\s<czas bieżący>\s, gdzie \s oznacza znaki
 * białe.
 * Czas rozpoczęcia i zakończenia zapisywane są jako liczba upłyniętych minut od
 * północy.
 * Dane wycięte z łańcucha wpisywane są pod adresy podane jako argumenty.
 */
/*
* Parses the line in the format \s<Registration number>\s<Start time>\s<End time>,
* where \s represents whitespace characters.
* The start and end times are recorded as the number of minutes elapsed since midnight.
* The data extracted from the string is written to the addresses given as arguments.
*/
void parse_checking_line(string const *line, string *license_plate,
                         hour_t *time_to_check) {
    assert(line != NULL && license_plate != NULL && time_to_check != NULL);

    istringstream iss(*line);
    string date;

    iss >> *license_plate >> date;

    *time_to_check = date_to_minutes_past_midnight(&date);
}

/*
* Checks if there is a new day - if the hour currently being checked is
* lower than the last hour checked.
* Returns true if the day has changed false otherwise.
*/
bool check_if_new_day(hour_t last_checked_hour, hour_t current_time) {
    if (last_checked_hour > current_time)
        return true;
    return false;
}

/*
* Checks if the parking time is greater than or equal to 10 and
* lower than or equal to 719 minutes.
*/
bool check_parking_time(hour_t parking_start, hour_t parking_end) {
    if (parking_start < parking_end && // Checking the one-day parking time.
         abs(parking_start - parking_end) >= MIN_PAYMENT_IN_MIN &&
         abs(parking_start - parking_end) <= MAX_PAYMENT_IN_MIN) {
            return true;
    }
    if ((parking_end < parking_start && // Checking the two-day parking time.
         (CLOSING_TIME_MIN - parking_start) + (parking_end - OPENING_TIME_MIN)
         >= MIN_PAYMENT_IN_MIN &&
         (CLOSING_TIME_MIN - parking_start) + (parking_end - OPENING_TIME_MIN)
         <= MAX_PAYMENT_IN_MIN)){
            return true;
    }
    return false;
}

/// MAIN FUNCTIONALITY ///
/*
* Removes unnecessary cars from the structure.
* If the day has changed:
* Current Time > End Time of parking -> two-day parking, don't remove.
* Otherwise -> parking is from the previous day, remove.
* If the day hasn't changed:
* Start Time < End Time && End Time < Current Time) -> remove.
* Otherwise -> current parking, don't remove.
* Start Time and End Time in the above comment refer to the currently
* considered pair, the removal of which we are considering.
*/
void delete_cars_from_data(parking_payments_vector_t *parking,
                           hour_t current_time, hour_t last_checked_hour) {
    assert(parking != NULL);

    if (check_if_new_day(last_checked_hour, current_time)) { // New day.
        for (auto entry = (*parking).begin(); entry != (*parking).end();) {
            // Parking from the previous day
            if ((*entry).second.first <= (*entry).second.second) {
                entry = ((*parking).erase(entry));
            }
            // Two-day parking, remove if (end time < current time)
            else if ((*entry).second.second < current_time) {
                entry = ((*parking).erase(entry));
            }
            // Current parking
            else {
                /* If the two-day parking is current, we change its start time to 8.00
                the next day to transform the two-day parking into a one-day parking 
                so that it gets removed after two days. */
                (*entry).second.first = OPENING_TIME_MIN;
                entry++;
            }
        }
    }
    else { // Day hasn't changed
        for (auto entry = (*parking).begin(); entry != (*parking).end();) {
            // Start time < End time && End time < Current time -> remove.
            if ((*entry).second.first < (*entry).second.second &&
                (*entry).second.second < current_time) {
                entry = ((*parking).erase(entry));
            }
            else { // Current parking.
                entry++;
            }
        }
    }
}

/*
* Adds payment for parking to the structure.
* We assume that the format of the line is correct.
* The payment may not meet the minimum and maximum parking time.
* Returns true if the payment was successfully added to the structure.
* Returns false if there was an error, i.e. someone is trying to pay for less
* than 10 minute or more than 11:59.
*/
bool add_payment(parking_payments_vector_t *parking, string const *line,
                 hour_t *last_checked_hour) {
    assert(parking != NULL && line != NULL && last_checked_hour != NULL);

    string license_plate;
    hour_t parking_start, parking_end;

    parse_payment_line(line, &license_plate, &parking_start, &parking_end);

    // Checks if parking time in [10, ..., 719]
    if (check_parking_time(parking_start, parking_end)) {
        // Remove cars that no one will ask about anymore.
        delete_cars_from_data(parking, parking_start, *last_checked_hour);
        *last_checked_hour = parking_start; // Aktualizujemy ostatnią godzinę.

        // If time in [10, ... , 719], we add to the structure.
        (*parking).push_back({license_plate, {parking_start, parking_end}});

        return true;
    }
    return false;
}

/*
 * Sprawdza płatność za parkowanie.
 * Zakłada, że format rozpatrywanej linii jest poprawny.
 * Zwraca true, jeżeli samochód o zadanej rejestracji opłacił parkowanie o danej
 * godzinie, false wpp.
 */
/*
* Checks payment for parking.
* Assumes that the format of the line is correct.
* Returns true if the car with specified registration has paid for parking
* at the given time, false otherwise.
*/
bool check_payment(parking_payments_vector_t *parking, string const *line,
                   hour_t *last_checked_hour) {
    assert(parking != NULL && line != NULL && last_checked_hour != NULL);

    string license_plate;
    hour_t time_to_check;

    parse_checking_line(line, &license_plate, &time_to_check);
    delete_cars_from_data(parking, time_to_check, *last_checked_hour);

    *last_checked_hour = time_to_check;

    for (auto entry : *parking) {
        if (license_plate == entry.first) {
            if (entry.second.first <= entry.second.second) {
                if (time_to_check <= entry.second.second)
                    return true;
            }
            else {
                if (time_to_check >= entry.second.first ||
                    time_to_check <= entry.second.second)
                    return true;
            }
        }
    }
    return false;
}

/*
 * Activate parking.
 */
void activate_parking(void) {
    line_count_t line_count = 0;
    hour_t last_checked_hour = 0; 
    string line;
    parking_payments_vector_t parking;

    while (getline(cin, line)) {
        line_count++;

        if (validate_payment_line(&line)) {
            if (add_payment(&parking, &line, &last_checked_hour))
                cout << "OK " << line_count << '\n';
            else
                cerr << "ERROR " << line_count << '\n';
        }
        else if (validate_checking_line(&line)) {
            if (check_payment(&parking, &line, &last_checked_hour))
                cout << "YES " << line_count << '\n';
            else
                cout << "NO " << line_count << '\n';
        }
        else {
            cerr << "ERROR " << line_count << '\n';
        }
    }
}

int main(void) {
    activate_parking();
    return 0;
}
