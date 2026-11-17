#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <stdexcept>
using namespace std;

// Hospital scheduling system: manages users, doctors, patients, and appointments.
// Data is stored in simple text files so the project remains beginner-friendly.
const int MAX_RECORDS = 500;
const string USERS_FILE = "data/users.txt";
const string PATIENTS_FILE = "data/patients.txt";
const string DOCTORS_FILE = "data/doctors.txt";
const string APPOINTMENTS_FILE = "data/appointments.txt";
const string AVAILABILITY_FILE = "data/availability.txt";

// Input helpers used throughout the program.
string getLineInput(string prompt)
{
    string value;
    while (true)
    {
        cout << prompt;
        getline(cin, value);
        if (value == "")
        {
            cout << "This field cannot be empty. Please try again.\n";
        }
        else if (value.find(',') != string::npos)
        {
            cout << "Please do not use commas. They separate records in our text files.\n";
        }
        else
        {
            return value;
        }
    }
}

int getNumber(string prompt, int minimum, int maximum)
{
    string input;
    int number;
    while (true)
    {
        cout << prompt;
        getline(cin, input);
        try
        {
            number = stoi(input);
            if (number >= minimum && number <= maximum)
                return number;
        }
        catch (...)
        {
            // stoi rejects empty input and values that are not valid integers.
        }
        cout << "Please enter a number from " << minimum << " to " << maximum << ".\n";
    }
}

bool isNumber(string value)
{
    if (value == "")
        return false;
    for (int i = 0; i < (int)value.length(); i++)
    {
        if (value[i] < '0' || value[i] > '9')
            return false;
    }
    return true;
}

// Read a comma-separated line into individual fields.
int splitLine(string line, string fields[], int maximumFields)
{
    int count = 0;
    int start = 0;
    int position;

    while ((position = (int)line.find(',', start)) != -1 && count < maximumFields - 1)
    {
        fields[count] = line.substr(start, position - start);
        count++;
        start = position + 1;
    }

    if (count < maximumFields)
    {
        fields[count] = line.substr(start);
        count++;
    }
    return count;
}

string currentDate()
{
    time_t now = time(0);
    tm *localTime = localtime(&now);
    char date[11];
    strftime(date, sizeof(date), "%Y-%m-%d", localTime);
    return date;
}

string currentTime()
{
    time_t now = time(0);
    tm *localTime = localtime(&now);
    char clockTime[6];
    strftime(clockTime, sizeof(clockTime), "%H:%M", localTime);
    return clockTime;
}

bool validDate(string date)
{
    if (date.length() != 10)
        return false;
    if (date[4] != '-' || date[7] != '-')
        return false;

    string year = date.substr(0, 4);
    string month = date.substr(5, 2);
    string day = date.substr(8, 2);

    if (!isNumber(year) || !isNumber(month) || !isNumber(day))
        return false;

    int y = stoi(year);
    int m = stoi(month);
    int d = stoi(day);

    if (y < 2020 || m < 1 || m > 12 || d < 1 || d > 31)
        return false;
    if (m == 2)
    {
        bool leapYear = (y % 400 == 0) || (y % 4 == 0 && y % 100 != 0);
        if (leapYear && d > 29)
            return false;
        if (!leapYear && d > 28)
            return false;
    }
    if ((m == 4 || m == 6 || m == 9 || m == 11) && d > 30)
        return false;
    return true;
}

bool validTime(string value)
{
    if (value.length() != 5 || value[2] != ':')
        return false;
    string hour = value.substr(0, 2);
    string minute = value.substr(3, 2);
    if (!isNumber(hour) || !isNumber(minute))
        return false;
    int h = stoi(hour);
    int m = stoi(minute);
    return h >= 0 && h <= 23 && m == 0;
}

int timeToMinutes(string value)
{
    int hour = stoi(value.substr(0, 2));
    int minute = stoi(value.substr(3, 2));
    return hour * 60 + minute;
}

string minutesToTime(int minutes)
{
    int hour = minutes / 60;
    string result = "";
    if (hour < 10)
        result += "0";
    result += to_string(hour) + ":00";
    return result;
}

string getDateInput(bool futureOnly)
{
    while (true)
    {
        string date = getLineInput("Enter date (YYYY-MM-DD): ");
        if (!validDate(date))
        {
            cout << "Invalid date format. Please try again.\n";
        }
        // YYYY-MM-DD dates sort lexicographically in chronological order.
        else if (futureOnly && date < currentDate())
        {
            cout << "You cannot select a date in the past.\n";
        }
        else
        {
            return date;
        }
    }
}

string getTimeInput(string prompt)
{
    while (true)
    {
        string value = getLineInput(prompt + " (HH:00): ");
        if (validTime(value))
            return value;
        cout << "Please use a whole-hour time, for example 09:00.\n";
    }
}

void createDataFiles()
{
    ofstream file;

    file.open(USERS_FILE, ios::app);
    if (!file.is_open())
        throw runtime_error("Cannot create data/users.txt. Create the data folder first.");
    file.close();

    file.open(PATIENTS_FILE, ios::app);
    if (!file.is_open())
        throw runtime_error("Cannot create data/patients.txt.");
    file.close();

    file.open(DOCTORS_FILE, ios::app);
    if (!file.is_open())
        throw runtime_error("Cannot create data/doctors.txt.");
    file.close();

    file.open(APPOINTMENTS_FILE, ios::app);
    if (!file.is_open())
        throw runtime_error("Cannot create data/appointments.txt.");
    file.close();

    file.open(AVAILABILITY_FILE, ios::app);
    if (!file.is_open())
        throw runtime_error("Cannot create data/availability.txt.");
    file.close();
}

// Used by booking and rescheduling when a slot conflicts with an existing appointment.
class SlotUnavailableException : public exception
{
public:
    const char *what() const noexcept override
    {
        return "This appointment slot is not available.";
    }
};

// A user account holds login details and the role used to select the right menu.
class User
{
private:
    string id;
    string fullName;
    string username;
    string password;
    string role;

public:
    User() {}

    User(string userId, string name, string user, string pass, string userRole)
    {
        id = userId;
        fullName = name;
        username = user;
        password = pass;
        role = userRole;
    }

    string getId() { return id; }
    string getFullName() { return fullName; }
    string getUsername() { return username; }
    string getPassword() { return password; }
    string getRole() { return role; }

    void setFullName(string name) { fullName = name; }
    void setPassword(string pass) { password = pass; }

    string toLine()
    {
        return id + "," + fullName + "," + username + "," + password + "," + role;
    }
};

// Patient-specific data is kept in the patient file; login details stay in User.
class Patient : public User
{
public:
    Patient() {}
    Patient(string id, string name, string username, string password)
        : User(id, name, username, password, "Patient") {}
};

// Doctor profile data, including the hours used to build available appointment slots.
class Doctor
{
private:
    string id;
    string fullName;
    string username;
    string specialization;
    string startTime;
    string endTime;

public:
    Doctor() {}

    Doctor(string doctorId, string name, string user, string special,
           string start, string end)
    {
        id = doctorId;
        fullName = name;
        username = user;
        specialization = special;
        startTime = start;
        endTime = end;
    }

    string getId() { return id; }
    string getFullName() { return fullName; }
    string getUsername() { return username; }
    string getSpecialization() { return specialization; }
    string getStartTime() { return startTime; }
    string getEndTime() { return endTime; }

    void setFullName(string name) { fullName = name; }
    void setSpecialization(string value) { specialization = value; }
    void setWorkingHours(string start, string end)
    {
        startTime = start;
        endTime = end;
    }

    string toLine()
    {
        return id + "," + fullName + "," + username + "," + specialization + "," + startTime + "," + endTime;
    }
};

// Represents a confirmed, cancelled, or completed appointment.
class Appointment
{
private:
    string id;
    string patientId;
    string doctorId;
    string date;
    string startTime;
    string endTime;
    string status;

public:
    Appointment() {}

    Appointment(string appointmentId, string patient, string doctor,
                string appointmentDate, string start, string end, string appointmentStatus)
    {
        id = appointmentId;
        patientId = patient;
        doctorId = doctor;
        date = appointmentDate;
        startTime = start;
        endTime = end;
        status = appointmentStatus;
    }

    string getId() { return id; }
    string getPatientId() { return patientId; }
    string getDoctorId() { return doctorId; }
    string getDate() { return date; }
    string getStartTime() { return startTime; }
    string getEndTime() { return endTime; }
    string getStatus() { return status; }

    void setStatus(string value) { status = value; }
    void setDateAndTime(string newDate, string start, string end)
    {
        date = newDate;
        startTime = start;
        endTime = end;
    }

    string toLine()
    {
        return id + "," + patientId + "," + doctorId + "," + date + "," + startTime + "," + endTime + "," + status;
    }
};

// Handles registration and login checks against the user file.
class AuthSystem
{
public:
    bool usernameExists(string username)
    {
        ifstream file(USERS_FILE);
        if (!file.is_open())
            throw runtime_error("Cannot open users.txt");

        string line;
        string fields[5];
        while (getline(file, line))
        {
            splitLine(line, fields, 5);
            if (fields[2] == username)
            {
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }

    void registerUser(User user)
    {
        if (usernameExists(user.getUsername()))
        {
            throw invalid_argument("That username is already taken.");
        }

        ofstream file(USERS_FILE, ios::app);
        if (!file.is_open())
            throw runtime_error("Cannot open users.txt for writing.");
        file << user.toLine() << "\n";
        file.close();
    }

    bool login(string username, string password, User &loggedInUser)
    {
        ifstream file(USERS_FILE);
        if (!file.is_open())
            throw runtime_error("Cannot open users.txt");

        string line;
        string fields[5];
        while (getline(file, line))
        {
            int count = splitLine(line, fields, 5);
            if (count == 5 && fields[2] == username && fields[3] == password)
            {
                loggedInUser = User(fields[0], fields[1], fields[2], fields[3], fields[4]);
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }
};

// Core hospital workflow: loading files, checking availability, and managing menus.
class HospitalSystem
{
private:
    AuthSystem auth;

    int loadDoctors(Doctor doctors[])
    {
        ifstream file(DOCTORS_FILE);
        if (!file.is_open())
            throw runtime_error("Cannot open doctors.txt");
        int count = 0;
        string line;
        string fields[6];
        while (getline(file, line) && count < MAX_RECORDS)
        {
            if (line.empty())
                continue;

            int fieldCount = splitLine(line, fields, 6);
            if (fieldCount < 3)
                continue;

            string doctorId = fields[0];
            string fullName = fields[1];
            string username = "";
            string specialization = "";
            string start = "00:00";
            string end = "00:00";

            // Older doctor records may omit the username or working hours.
            // Recover those values from the account and availability files.
            if (fieldCount >= 6)
            {
                username = fields[2];
                specialization = fields[3];
                start = fields[4];
                end = fields[5];
            }
            else if (fieldCount == 4)
            {
                username = fields[2];
                specialization = fields[3];
            }
            else if (fieldCount == 3)
            {
                specialization = fields[2];
            }

            if (username.empty())
            {
                User users[MAX_RECORDS];
                int userCount = loadUsers(users);
                for (int i = 0; i < userCount; i++)
                {
                    if (users[i].getId() == doctorId)
                    {
                        username = users[i].getUsername();
                        break;
                    }
                }
            }

            if (fieldCount < 6 || start == "" || end == "")
            {
                ifstream availabilityFile(AVAILABILITY_FILE);
                string availabilityLine;
                string availabilityFields[3];
                while (getline(availabilityFile, availabilityLine))
                {
                    int availabilityCount = splitLine(availabilityLine, availabilityFields, 3);
                    if (availabilityCount >= 3 && availabilityFields[0] == doctorId)
                    {
                        start = availabilityFields[1];
                        end = availabilityFields[2];
                        break;
                    }
                }
                availabilityFile.close();
            }

            if (specialization.empty())
            {
                specialization = "General Medicine";
            }

            doctors[count] = Doctor(doctorId, fullName, username, specialization, start, end);
            count++;
        }
        file.close();
        return count;
    }

    int loadAppointments(Appointment appointments[])
    {
        ifstream file(APPOINTMENTS_FILE);
        if (!file.is_open())
            throw runtime_error("Cannot open appointments.txt");
        int count = 0;
        string line;
        string fields[7];
        while (getline(file, line) && count < MAX_RECORDS)
        {
            if (splitLine(line, fields, 7) == 7)
            {
                appointments[count] = Appointment(fields[0], fields[1], fields[2], fields[3], fields[4], fields[5], fields[6]);
                count++;
            }
        }
        file.close();
        return count;
    }

    int loadUsers(User users[])
    {
        ifstream file(USERS_FILE);
        if (!file.is_open())
            throw runtime_error("Cannot open users.txt");
        int count = 0;
        string line;
        string fields[5];
        while (getline(file, line) && count < MAX_RECORDS)
        {
            if (splitLine(line, fields, 5) == 5)
            {
                users[count] = User(fields[0], fields[1], fields[2], fields[3], fields[4]);
                count++;
            }
        }
        file.close();
        return count;
    }

    int loadPatients(Patient patients[])
    {
        ifstream file(PATIENTS_FILE);
        if (!file.is_open())
            throw runtime_error("Cannot open patients.txt");
        int count = 0;
        string line;
        string fields[3];
        while (getline(file, line) && count < MAX_RECORDS)
        {
            if (line.empty())
                continue;

            int fieldCount = splitLine(line, fields, 3);
            if (fieldCount < 2)
                continue;

            string patientId = fields[0];
            string fullName = fields[1];
            string username = (fieldCount >= 3) ? fields[2] : "";

            // Older patient records contain no username, so look it up by ID.
            if (username.empty())
            {
                User users[MAX_RECORDS];
                int userCount = loadUsers(users);
                for (int i = 0; i < userCount; i++)
                {
                    if (users[i].getId() == patientId)
                    {
                        username = users[i].getUsername();
                        break;
                    }
                }
            }

            patients[count] = Patient(patientId, fullName, username, "");
            count++;
        }
        file.close();
        return count;
    }

    void saveDoctors(Doctor doctors[], int count)
    {
        ofstream file(DOCTORS_FILE);
        if (!file.is_open())
            throw runtime_error("Cannot save doctors.txt");
        for (int i = 0; i < count; i++)
            file << doctors[i].toLine() << "\n";
        file.close();
    }

    void saveAppointments(Appointment appointments[], int count)
    {
        ofstream file(APPOINTMENTS_FILE);
        if (!file.is_open())
            throw runtime_error("Cannot save appointments.txt");
        for (int i = 0; i < count; i++)
            file << appointments[i].toLine() << "\n";
        file.close();
    }

    void saveUsers(User users[], int count)
    {
        ofstream file(USERS_FILE);
        if (!file.is_open())
            throw runtime_error("Cannot save users.txt");
        for (int i = 0; i < count; i++)
            file << users[i].toLine() << "\n";
        file.close();
    }

    string makeId(string filename, string prefix)
    {
        ifstream file(filename);
        if (!file.is_open())
            throw runtime_error("Cannot open " + filename);
        int count = 1000;
        string line;
        while (getline(file, line))
            count++;
        file.close();
        return prefix + to_string(count + 1);
    }

    string patientIdForUsername(string username)
    {
        User users[MAX_RECORDS];
        int count = loadUsers(users);
        for (int i = 0; i < count; i++)
        {
            if (users[i].getUsername() == username && users[i].getRole() == "Patient")
                return users[i].getId();
        }
        return "";
    }

    string doctorIdForUsername(string username)
    {
        Doctor doctors[MAX_RECORDS];
        int count = loadDoctors(doctors);
        for (int i = 0; i < count; i++)
        {
            if (doctors[i].getUsername() == username)
                return doctors[i].getId();
        }
        return "";
    }

    string patientName(string patientId)
    {
        User users[MAX_RECORDS];
        int count = loadUsers(users);
        for (int i = 0; i < count; i++)
            if (users[i].getId() == patientId)
                return users[i].getFullName();
        return "Unknown Patient";
    }

    string doctorName(string doctorId)
    {
        Doctor doctors[MAX_RECORDS];
        int count = loadDoctors(doctors);
        for (int i = 0; i < count; i++)
            if (doctors[i].getId() == doctorId)
                return doctors[i].getFullName();
        return "Unknown Doctor";
    }

    bool slotIsAvailable(string doctorId, string patientId, string date, string startTime, string ignoredId = "")
    {
        Appointment appointments[MAX_RECORDS];
        int count = loadAppointments(appointments);
        int requestedStart = timeToMinutes(startTime);
        int requestedEnd = requestedStart + 60;

        for (int i = 0; i < count; i++)
        {
            if (appointments[i].getId() == ignoredId)
                continue;
            if (appointments[i].getDate() != date)
                continue;
            if (appointments[i].getStatus() == "Cancelled" || appointments[i].getStatus() == "Rescheduled")
                continue;

            int existingStart = timeToMinutes(appointments[i].getStartTime());
            int existingEnd = timeToMinutes(appointments[i].getEndTime());
            // Adjacent appointments are allowed; only overlapping intervals conflict.
            bool overlap = requestedStart < existingEnd && requestedEnd > existingStart;

            // A reschedule can ignore the appointment being replaced, but must
            // still check for conflicts with every other active appointment.
            if (overlap && (appointments[i].getDoctorId() == doctorId || appointments[i].getPatientId() == patientId))
                return false;
        }
        return true;
    }

    bool doctorExists(string doctorId)
    {
        Doctor doctors[MAX_RECORDS];
        int count = loadDoctors(doctors);
        for (int i = 0; i < count; i++)
            if (doctors[i].getId() == doctorId)
                return true;
        return false;
    }

    void createInitialAdmin()
    {
        User users[MAX_RECORDS];
        int count = loadUsers(users);
        if (count > 0)
            return;

        cout << "\n========== INITIAL ADMIN SETUP ==========\n\n";
        string name = getLineInput("Enter full name: ");
        string username = getLineInput("Enter username: ");
        string password = getLineInput("Enter password: ");
        User admin("A1001", name, username, password, "Admin");
        auth.registerUser(admin);
        cout << "\nAdmin account created successfully!\n";
        cout << "Admin ID: A1001\n";
    }

    void patientRegistration()
    {
        cout << "\n========== PATIENT REGISTRATION ==========\n\n";
        string name = getLineInput("Enter full name: ");
        string username = getLineInput("Enter username: ");
        string password = getLineInput("Enter password: ");

        string id = makeId(PATIENTS_FILE, "P");
        Patient patient(id, name, username, password);
        auth.registerUser(User(id, name, username, password, "Patient"));

        ofstream patientFile(PATIENTS_FILE, ios::app);
        if (!patientFile.is_open())
            throw runtime_error("Cannot open patients.txt");
        patientFile << id << "," << name << "," << username << "\n";
        patientFile.close();

        cout << "\nPatient registered successfully!\nPatient ID: " << patient.getId() << "\n";
    }

    void registerDoctor()
    {
        cout << "\n========== REGISTER NEW DOCTOR ==========\n\n";
        string name = getLineInput("Enter full name: ");
        string username = getLineInput("Enter username: ");
        string password = getLineInput("Enter password: ");
        string specialization = getLineInput("Enter specialization: ");
        string start = getTimeInput("Enter starting time");
        string end = getTimeInput("Enter ending time");

        if (timeToMinutes(end) <= timeToMinutes(start))
        {
            cout << "Ending time must be later than starting time.\n";
            return;
        }

        string id = makeId(DOCTORS_FILE, "D");
        auth.registerUser(User(id, name, username, password, "Doctor"));

        Doctor doctor(id, name, username, specialization, start, end);
        ofstream doctorFile(DOCTORS_FILE, ios::app);
        if (!doctorFile.is_open())
            throw runtime_error("Cannot open doctors.txt");
        doctorFile << doctor.toLine() << "\n";
        doctorFile.close();

        ofstream availabilityFile(AVAILABILITY_FILE, ios::app);
        if (!availabilityFile.is_open())
            throw runtime_error("Cannot open availability.txt");
        availabilityFile << id << "," << start << "," << end << "\n";
        availabilityFile.close();

        cout << "\nDoctor registered successfully!\nDoctor ID: " << id << "\n";
    }

    void viewAllUsers()
    {
        User users[MAX_RECORDS];
        int count = loadUsers(users);
        cout << "\n========== ALL USERS ==========\n";
        if (count == 0)
            cout << "No users found.\n";
        for (int i = 0; i < count; i++)
        {
            cout << "\nID: " << users[i].getId()
                 << "\nName: " << users[i].getFullName()
                 << "\nUsername: " << users[i].getUsername()
                 << "\nRole: " << users[i].getRole() << "\n";
        }
    }

    void searchUser()
    {
        string id = getLineInput("Enter user ID: ");
        User users[MAX_RECORDS];
        int count = loadUsers(users);
        for (int i = 0; i < count; i++)
        {
            if (users[i].getId() == id)
            {
                cout << "\nName: " << users[i].getFullName()
                     << "\nUsername: " << users[i].getUsername()
                     << "\nRole: " << users[i].getRole() << "\n";
                return;
            }
        }
        cout << "User not found.\n";
    }

    void updateUser()
    {
        string id = getLineInput("Enter user ID to update: ");
        User users[MAX_RECORDS];
        int count = loadUsers(users);
        int found = -1;
        for (int i = 0; i < count; i++)
            if (users[i].getId() == id)
                found = i;
        if (found == -1)
        {
            cout << "User not found.\n";
            return;
        }

        cout << "1. Update full name\n2. Update password\n";
        int choice = getNumber("Choose: ", 1, 2);
        if (choice == 1)
            users[found].setFullName(getLineInput("Enter new full name: "));
        else
            users[found].setPassword(getLineInput("Enter new password: "));
        saveUsers(users, count);

        // Names are duplicated in role-specific files, so keep those copies in sync.
        if (users[found].getRole() == "Doctor" && choice == 1)
        {
            Doctor doctors[MAX_RECORDS];
            int doctorCount = loadDoctors(doctors);
            for (int i = 0; i < doctorCount; i++)
            {
                if (doctors[i].getId() == id)
                    doctors[i].setFullName(users[found].getFullName());
            }
            saveDoctors(doctors, doctorCount);
        }
        else if (users[found].getRole() == "Patient" && choice == 1)
        {
            Patient patients[MAX_RECORDS];
            int patientCount = loadPatients(patients);
            ofstream patientFile(PATIENTS_FILE);
            if (!patientFile.is_open())
                throw runtime_error("Cannot update patients.txt");
            for (int i = 0; i < patientCount; i++)
            {
                if (patients[i].getId() == id)
                    patients[i].setFullName(users[found].getFullName());
                patientFile << patients[i].getId() << "," << patients[i].getFullName()
                            << "," << patients[i].getUsername() << "\n";
            }
            patientFile.close();
        }
        cout << "User information updated successfully.\n";
    }

    void manageUsersMenu()
    {
        int choice;
        do
        {
            cout << "\n========== MANAGE USERS ==========\n\n";
            cout << "1. View All Users\n2. Search User\n3. Register New Doctor\n4. Update User Information\n0. Back\n";
            choice = getNumber("\nChoose an option: ", 0, 4);
            if (choice == 1)
                viewAllUsers();
            else if (choice == 2)
                searchUser();
            else if (choice == 3)
                registerDoctor();
            else if (choice == 4)
                updateUser();
        } while (choice != 0);
    }

    void printAppointment(Appointment appointment)
    {
        cout << "\nAppointment ID: " << appointment.getId()
             << "\nPatient: " << patientName(appointment.getPatientId())
             << " (" << appointment.getPatientId() << ")"
             << "\nDoctor: " << doctorName(appointment.getDoctorId())
             << " (" << appointment.getDoctorId() << ")"
             << "\nDate: " << appointment.getDate()
             << "\nTime: " << appointment.getStartTime() << " - " << appointment.getEndTime()
             << "\nStatus: " << appointment.getStatus() << "\n";
    }

    void viewAllAppointments()
    {
        Appointment appointments[MAX_RECORDS];
        int count = loadAppointments(appointments);
        cout << "\n========== ALL APPOINTMENTS ==========\n";
        if (count == 0)
            cout << "No appointments found.\n";
        for (int i = 0; i < count; i++)
            printAppointment(appointments[i]);
    }

    void searchAppointment()
    {
        string id = getLineInput("Enter appointment ID: ");
        Appointment appointments[MAX_RECORDS];
        int count = loadAppointments(appointments);
        for (int i = 0; i < count; i++)
        {
            if (appointments[i].getId() == id)
            {
                printAppointment(appointments[i]);
                return;
            }
        }
        cout << "Appointment not found.\n";
    }

    void cancelAppointment(string appointmentId, string patientId = "")
    {
        Appointment appointments[MAX_RECORDS];
        int count = loadAppointments(appointments);
        for (int i = 0; i < count; i++)
        {
            if (appointments[i].getId() == appointmentId &&
                (patientId == "" || appointments[i].getPatientId() == patientId))
            {
                if (appointments[i].getStatus() != "Confirmed")
                {
                    cout << "Only confirmed appointments can be cancelled.\n";
                    return;
                }
                appointments[i].setStatus("Cancelled");
                saveAppointments(appointments, count);
                cout << "Appointment cancelled successfully.\n";
                return;
            }
        }
        cout << "Appointment not found.\n";
    }

    void adminUpdateAppointment()
    {
        string id = getLineInput("Enter appointment ID: ");
        Appointment appointments[MAX_RECORDS];
        int count = loadAppointments(appointments);
        for (int i = 0; i < count; i++)
        {
            if (appointments[i].getId() == id)
            {
                if (appointments[i].getStatus() != "Confirmed")
                {
                    cout << "Only confirmed appointments can be updated.\n";
                    return;
                }
                string newDate = getDateInput(true);
                string newStart = getTimeInput("Enter new starting time");
                string newEnd = minutesToTime(timeToMinutes(newStart) + 60);

                Doctor doctors[MAX_RECORDS];
                int doctorCount = loadDoctors(doctors);
                bool withinWorkingHours = false;
                for (int j = 0; j < doctorCount; j++)
                {
                    if (doctors[j].getId() == appointments[i].getDoctorId() &&
                        timeToMinutes(newStart) >= timeToMinutes(doctors[j].getStartTime()) &&
                        timeToMinutes(newEnd) <= timeToMinutes(doctors[j].getEndTime()))
                    {
                        withinWorkingHours = true;
                    }
                }
                if (!withinWorkingHours)
                {
                    cout << "The selected time is outside the doctor's working hours.\n";
                    return;
                }
                if (newDate == currentDate() && newStart <= currentTime())
                {
                    cout << "You cannot select a time that has already passed.\n";
                    return;
                }
                if (!slotIsAvailable(appointments[i].getDoctorId(), appointments[i].getPatientId(), newDate, newStart, id))
                {
                    cout << "That time slot is not available.\n";
                    return;
                }
                appointments[i].setDateAndTime(newDate, newStart, newEnd);
                saveAppointments(appointments, count);
                cout << "Appointment updated successfully.\n";
                return;
            }
        }
        cout << "Appointment not found.\n";
    }

    void manageAppointmentsMenu()
    {
        int choice;
        do
        {
            cout << "\n======= MANAGE APPOINTMENTS =======\n\n";
            cout << "1. View All Appointments\n2. Search Appointment\n3. Cancel Appointment\n4. Update Appointment\n0. Back\n";
            choice = getNumber("\nChoose an option: ", 0, 4);
            if (choice == 1)
                viewAllAppointments();
            else if (choice == 2)
                searchAppointment();
            else if (choice == 3)
                cancelAppointment(getLineInput("Enter appointment ID: "));
            else if (choice == 4)
                adminUpdateAppointment();
        } while (choice != 0);
    }

    void adminMenu()
    {
        int choice;
        do
        {
            cout << "\n============= ADMIN MENU =============\n\n";
            cout << "1. Manage Users\n2. Manage Appointments\n0. Logout\n";
            choice = getNumber("\nChoose an option: ", 0, 2);
            if (choice == 1)
                manageUsersMenu();
            else if (choice == 2)
                manageAppointmentsMenu();
        } while (choice != 0);
    }

    void showAvailableSlots(string doctorId, string patientId, string date)
    {
        Doctor doctors[MAX_RECORDS];
        int count = loadDoctors(doctors);
        int doctorIndex = -1;
        for (int i = 0; i < count; i++)
            if (doctors[i].getId() == doctorId)
                doctorIndex = i;
        if (doctorIndex == -1)
            throw runtime_error("Doctor not found.");

        int start = timeToMinutes(doctors[doctorIndex].getStartTime());
        int end = timeToMinutes(doctors[doctorIndex].getEndTime());
        bool found = false;

        cout << "\nAvailable one-hour slots:\n";
        for (int time = start; time + 60 <= end; time += 60)
        {
            string slot = minutesToTime(time);
            // A slot must fit within working hours and still be in the future today.
            if (date == currentDate() && slot <= currentTime())
                continue;
            if (slotIsAvailable(doctorId, patientId, date, slot))
            {
                cout << slot << " - " << minutesToTime(time + 60) << "\n";
                found = true;
            }
        }
        if (!found)
            cout << "No available slots for this date.\n";
    }

    void bookAppointment(string patientId)
    {
        Doctor doctors[MAX_RECORDS];
        int count = loadDoctors(doctors);
        if (count == 0)
        {
            cout << "No doctors are registered yet.\n";
            return;
        }

        cout << "\nAvailable specializations:\n";
        for (int i = 0; i < count; i++)
            cout << "- " << doctors[i].getSpecialization() << "\n";
        string specialization = getLineInput("\nEnter specialization: ");

        cout << "\nDoctors in " << specialization << ":\n";
        bool found = false;
        for (int i = 0; i < count; i++)
        {
            if (doctors[i].getSpecialization() == specialization)
            {
                cout << doctors[i].getId() << " | " << doctors[i].getFullName()
                     << " | Working hours: " << doctors[i].getStartTime() << " - " << doctors[i].getEndTime()
                     << " | Duration: 1 hour\n";
                found = true;
            }
        }
        if (!found)
        {
            cout << "No doctors found for this specialization.\n";
            return;
        }

        string doctorId = getLineInput("Enter doctor ID: ");
        int doctorIndex = -1;
        for (int i = 0; i < count; i++)
            if (doctors[i].getId() == doctorId && doctors[i].getSpecialization() == specialization)
                doctorIndex = i;
        if (doctorIndex == -1)
        {
            cout << "Invalid doctor selection.\n";
            return;
        }

        string date = getDateInput(true);
        showAvailableSlots(doctorId, patientId, date);
        string start = getTimeInput("Select starting time");

        try
        {
            // Validate again after input; the user may enter a time that was not listed.
            if (timeToMinutes(start) < timeToMinutes(doctors[doctorIndex].getStartTime()) ||
                timeToMinutes(start) + 60 > timeToMinutes(doctors[doctorIndex].getEndTime()))
                throw SlotUnavailableException();
            if (date == currentDate() && start <= currentTime())
                throw invalid_argument("You cannot book a time that has already passed.");
            if (!slotIsAvailable(doctorId, patientId, date, start))
                throw SlotUnavailableException();

            string end = minutesToTime(timeToMinutes(start) + 60);
            cout << "\nPlease confirm your appointment:\n";
            cout << "Doctor: " << doctors[doctorIndex].getFullName()
                 << "\nDate: " << date << "\nTime: " << start << " - " << end << "\n";
            string confirm = getLineInput("Confirm? (y/n): ");
            if (confirm != "y" && confirm != "Y")
            {
                cout << "Booking cancelled.\n";
                return;
            }

            string id = makeId(APPOINTMENTS_FILE, "AP");
            Appointment appointment(id, patientId, doctorId, date, start, end, "Confirmed");
            ofstream file(APPOINTMENTS_FILE, ios::app);
            if (!file.is_open())
                throw runtime_error("Cannot save appointment.");
            file << appointment.toLine() << "\n";
            file.close();
            cout << "\nAppointment booked successfully!\nAppointment ID: " << id << "\n";
        }
        catch (const SlotUnavailableException &e)
        {
            cout << "Error: " << e.what() << "\n";
        }
        catch (const exception &e)
        {
            cout << "Error: " << e.what() << "\n";
        }
    }

    void viewPatientAppointments(string patientId)
    {
        Appointment appointments[MAX_RECORDS];
        int count = loadAppointments(appointments);
        bool found = false;
        for (int i = 0; i < count; i++)
        {
            if (appointments[i].getPatientId() == patientId)
            {
                printAppointment(appointments[i]);
                found = true;
            }
        }
        if (!found)
            cout << "You do not have any appointments yet.\n";
    }

    void patientReschedule(string patientId)
    {
        string id = getLineInput("Enter appointment ID to reschedule: ");
        Appointment appointments[MAX_RECORDS];
        int count = loadAppointments(appointments);
        int index = -1;
        for (int i = 0; i < count; i++)
        {
            if (appointments[i].getId() == id && appointments[i].getPatientId() == patientId && appointments[i].getStatus() == "Confirmed")
                index = i;
        }
        if (index == -1)
        {
            cout << "Confirmed appointment not found.\n";
            return;
        }

        string newDate = getDateInput(true);
        showAvailableSlots(appointments[index].getDoctorId(), patientId, newDate);
        string newStart = getTimeInput("Select new starting time");

        try
        {
            if (newDate == currentDate() && newStart <= currentTime())
                throw invalid_argument("You cannot select a time that has already passed.");
            if (!slotIsAvailable(appointments[index].getDoctorId(), patientId, newDate, newStart, id))
                throw SlotUnavailableException();

            Doctor doctors[MAX_RECORDS];
            int doctorCount = loadDoctors(doctors);
            string doctorStart = "00:00", doctorEnd = "00:00";
            for (int i = 0; i < doctorCount; i++)
            {
                if (doctors[i].getId() == appointments[index].getDoctorId())
                {
                    doctorStart = doctors[i].getStartTime();
                    doctorEnd = doctors[i].getEndTime();
                }
            }
            if (timeToMinutes(newStart) < timeToMinutes(doctorStart) || timeToMinutes(newStart) + 60 > timeToMinutes(doctorEnd))
                throw SlotUnavailableException();

            // Keep the old appointment for history and add the replacement as a new record.
            appointments[index].setStatus("Rescheduled");
            saveAppointments(appointments, count);

            string newId = makeId(APPOINTMENTS_FILE, "AP");
            Appointment newAppointment(newId, patientId, appointments[index].getDoctorId(), newDate,
                                       newStart, minutesToTime(timeToMinutes(newStart) + 60), "Confirmed");
            ofstream file(APPOINTMENTS_FILE, ios::app);
            if (!file.is_open())
                throw runtime_error("Cannot save the new appointment.");
            file << newAppointment.toLine() << "\n";
            file.close();
            cout << "Appointment rescheduled successfully.\nNew Appointment ID: " << newId << "\n";
        }
        catch (const exception &e)
        {
            cout << "Error: " << e.what() << "\n";
        }
    }

    void patientMenu(User user)
    {
        string patientId = user.getId();
        int choice;
        do
        {
            cout << "\n============= PATIENT MENU =============\n\n";
            cout << "1. Book an Appointment\n2. View My Appointments\n3. Reschedule Appointment\n4. Cancel Appointment\n0. Logout\n";
            choice = getNumber("\nChoose an option: ", 0, 4);
            if (choice == 1)
                bookAppointment(patientId);
            else if (choice == 2)
                viewPatientAppointments(patientId);
            else if (choice == 3)
                patientReschedule(patientId);
            else if (choice == 4)
                cancelAppointment(getLineInput("Enter appointment ID: "), patientId);
        } while (choice != 0);
    }

    void viewDoctorAppointments(string doctorId)
    {
        Appointment appointments[MAX_RECORDS];
        int count = loadAppointments(appointments);
        bool found = false;
        for (int i = 0; i < count; i++)
        {
            if (appointments[i].getDoctorId() == doctorId)
            {
                printAppointment(appointments[i]);
                found = true;
            }
        }
        if (!found)
            cout << "You do not have any appointments yet.\n";
    }

    void completeAppointment(string doctorId)
    {
        string id = getLineInput("Enter appointment ID: ");
        Appointment appointments[MAX_RECORDS];
        int count = loadAppointments(appointments);
        for (int i = 0; i < count; i++)
        {
            if (appointments[i].getId() == id && appointments[i].getDoctorId() == doctorId)
            {
                if (appointments[i].getStatus() != "Confirmed")
                {
                    cout << "This appointment cannot be completed.\n";
                    return;
                }
                if (appointments[i].getDate() > currentDate() ||
                    (appointments[i].getDate() == currentDate() && appointments[i].getEndTime() > currentTime()))
                {
                    cout << "You can complete an appointment only after its scheduled end time.\n";
                    return;
                }
                appointments[i].setStatus("Completed");
                saveAppointments(appointments, count);
                cout << "Appointment marked as completed.\n";
                return;
            }
        }
        cout << "Appointment not found.\n";
    }

    void doctorMenu(User user)
    {
        string doctorId = user.getId();
        int choice;
        do
        {
            cout << "\n============= DOCTOR MENU =============\n\n";
            cout << "1. View My Appointments\n2. Mark an Appointment as Completed\n0. Logout\n";
            choice = getNumber("\nChoose an option: ", 0, 2);
            if (choice == 1)
                viewDoctorAppointments(doctorId);
            else if (choice == 2)
                completeAppointment(doctorId);
        } while (choice != 0);
    }

public:
    void run()
    {
        createDataFiles();
        createInitialAdmin();

        int choice;
        do
        {
            cout << "\n========================================\n";
            cout << "       HOSPITAL SCHEDULING SYSTEM\n";
            cout << "========================================\n\n";
            cout << "1. Login\n2. Register\n0. Exit\n";
            choice = getNumber("\nChoose an option: ", 0, 2);

            if (choice == 1)
            {
                string username = getLineInput("Enter username: ");
                string password = getLineInput("Enter password: ");
                User loggedInUser;
                if (auth.login(username, password, loggedInUser))
                {
                    cout << "\nWelcome, " << loggedInUser.getFullName() << "!\n";
                    if (loggedInUser.getRole() == "Admin")
                        adminMenu();
                    else if (loggedInUser.getRole() == "Doctor")
                        doctorMenu(loggedInUser);
                    else if (loggedInUser.getRole() == "Patient")
                        patientMenu(loggedInUser);
                }
                else
                {
                    cout << "Invalid username or password.\n";
                }
            }
            else if (choice == 2)
            {
                cout << "\n========== REGISTRATION ==========\n\n";
                cout << "1. Register as Patient\n0. Back to Main Menu\n";
                int registerChoice = getNumber("\nChoose an option: ", 0, 1);
                if (registerChoice == 1)
                    patientRegistration();
            }
        } while (choice != 0);

        cout << "\nThank you for using Hospital Scheduling System. Goodbye!\n";
    }
};

// Entry point for the application.
int main()
{
    try
    {
        HospitalSystem hospital;
        hospital.run();
    }
    catch (const exception &e)
    {
        cout << "\nSystem error: " << e.what() << "\n";
    }
    return 0;
}
