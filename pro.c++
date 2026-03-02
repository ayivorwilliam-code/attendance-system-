#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <limits>
#include <cctype>

using namespace std;

struct Student
{
    string id;
    string name;
    int presentCount = 0;
    int totalClasses = 0;
    float attendance = 0.0f;
};

class AttendanceRegister
{
private:
    vector<Student> students;
    string className;
    string filename;
    string csvFilename;                  // additional output file in CSV (comma‑separated) format
    time_t lastSessionStart = 0;
    time_t lastSessionEnd = 0;

    void calculateAttendance(Student &s)
    {
        if (s.totalClasses > 0)
            s.attendance = (static_cast<float>(s.presentCount) / s.totalClasses) * 100.0f;
        else
            s.attendance = 0.0f;
    }

    string getCurrentDate()
    {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        char date[16];
        strftime(date, sizeof(date), "%Y-%m-%d", ltm);
        return string(date);
    }

    string getCurrentDateTime()
    {
        time_t now = time(0);
        tm *ltm = localtime(&now);
        char buf[32];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ltm);
        return string(buf);
    }

    void loadFromFile()
    {
        ifstream in(filename);
        if (!in.is_open())
            return;
        // simple loader: expects lines of form id\tname\tpresentCount\ttotalClasses
        string line;
        while (getline(in, line))
        {
            if (line.empty())
                continue;
            if (line.rfind("ID\t", 0) == 0)
                continue; // skip header if present
            Student s;
            // parse by tabs
            size_t p1 = line.find('\t');
            if (p1 == string::npos)
                continue;
            s.id = line.substr(0, p1);
            size_t p2 = line.find('\t', p1 + 1);
            if (p2 == string::npos)
                continue;
            s.name = line.substr(p1 + 1, p2 - (p1 + 1));
            size_t p3 = line.find('\t', p2 + 1);
            if (p3 == string::npos)
                continue;
            try
            {
                s.presentCount = stoi(line.substr(p2 + 1, p3 - (p2 + 1)));
                s.totalClasses = stoi(line.substr(p3 + 1));
            }
            catch (const exception &e)
            {
                continue; // skip records with invalid numbers
            }
            calculateAttendance(s);
            students.push_back(s);
        }
    }

    void saveToFile()
    {
        ofstream out(filename);
        if (!out.is_open())
            return;
        if (lastSessionStart != 0)
        {
            // write session info as a comment line
            char s1[32];
            char s2[32];
            tm *t1 = localtime(&lastSessionStart);
            tm *t2 = localtime(&lastSessionEnd);
            strftime(s1, sizeof(s1), "%Y-%m-%d %H:%M:%S", t1);
            strftime(s2, sizeof(s2), "%Y-%m-%d %H:%M:%S", t2);
            double secs = difftime(lastSessionEnd, lastSessionStart);
            int mins = static_cast<int>(secs) / 60;
            int remSecs = static_cast<int>(secs) % 60;
            out << "# Session: " << s1 << " - " << s2 << " (Duration: " << mins << "m " << remSecs << "s)\n";
        }
        out << "ID\tName\tPresent\tTotal\n";
        for (const auto &s : students)
        {
            out << s.id << '\t' << s.name << '\t' << s.presentCount << '\t' << s.totalClasses << "\n";
        }
        // additionally write CSV version
        saveCSV();
    }

public:
    AttendanceRegister(const string &class_name = "EEE L200")
    {
        className = class_name;
        // replace spaces for filename
        string safe = className;
        for (auto &c : safe)
            if (isspace((unsigned char)c))
                c = '_';
        filename = "attendance_" + safe + ".txt";
        csvFilename = "attendance_" + safe + ".csv";  // CSV extension (comma‑separated)
        loadFromFile();
    }

    void addStudent()
    {
        Student s;
        cout << "\nNew Student\n";
        cout << "  Student ID : ";
        getline(cin >> ws, s.id);
        cout << "  Full Name  : ";
        getline(cin, s.name);
        students.push_back(s);
        cout << "Student added successfully\n";
        saveToFile();
    }

    void saveCSV()
    {
        // produces a comma-separated output file with .csv extension
        ofstream out(csvFilename);
        if (!out.is_open())
            return;
        if (lastSessionStart != 0)
        {
            char s1[32];
            char s2[32];
            tm *t1 = localtime(&lastSessionStart);
            tm *t2 = localtime(&lastSessionEnd);
            strftime(s1, sizeof(s1), "%Y-%m-%d %H:%M:%S", t1);
            strftime(s2, sizeof(s2), "%Y-%m-%d %H:%M:%S", t2);
            double secs = difftime(lastSessionEnd, lastSessionStart);
            int mins = static_cast<int>(secs) / 60;
            int remSecs = static_cast<int>(secs) % 60;
            out << "# Session: " << s1 << "," << s2 << " (Duration: " << mins << "m " << remSecs << "s)\n";
        }
        out << "ID,Name,Present,Total\n";
        for (const auto &s : students)
        {
            out << s.id << ',' << s.name << ',' << s.presentCount << ',' << s.totalClasses << "\n";
        }
    }

    void exportCSV()
    {
        saveCSV();
        cout << "Records exported to CSV file: " << csvFilename << "\n";
    }

    void markAttendance()
    {
        if (students.empty())
        {
            cout << "No students in register yet.\n";
            return;
        }
        lastSessionStart = time(0);
        string startStr = getCurrentDateTime();
        string today = getCurrentDate();
        cout << "\nMarking attendance for: " << today << "\n";
        cout << "Class: " << className << "\n";
        cout << "Session start: " << startStr << "\n\n";
        for (size_t i = 0; i < students.size(); ++i)
        {
            cout << setw(3) << (i + 1) << ". " << left << setw(18) << students[i].id << setw(30) << students[i].name << " : ";
            char ch;
            cout << "(p=present, a=absent): ";
            cin >> ch;
            ch = tolower((unsigned char)ch);
            if (ch == 'p')
                students[i].presentCount++;
            students[i].totalClasses++;
            calculateAttendance(students[i]);
        }
        lastSessionEnd = time(0);
        string endStr = getCurrentDateTime();
        double secs = difftime(lastSessionEnd, lastSessionStart);
        int mins = static_cast<int>(secs) / 60;
        int remSecs = static_cast<int>(secs) % 60;
        cout << "\nAttendance marked for " << today << "\n";
        cout << "Session end: " << endStr << "\n";
        cout << "Duration: " << mins << "m " << remSecs << "s\n";
        saveToFile();
    }

    // showAttendance removed per request

    // showDefaulters removed per request
};

int main()
{
    AttendanceRegister reg("EEE L200");
    while (true)
    {
        cout << "\n--- Student Attendance System ---\n";
        cout << "1) Add student\n2) Mark attendance\n3) Export records (CSV format)\n4) Exit\n";
        cout << "Choose option: ";
        int opt;
        if (!(cin >> opt))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        switch (opt)
        {
        case 1:
            reg.addStudent();
            break;
        case 2:
            reg.markAttendance();
            break;
        case 3:
            reg.exportCSV();
            break;
        case 4:
            cout << "Exiting.\n";
            return 0;
        default:
            cout << "Invalid option\n";
            break;
        }
    }
    return 0;
}