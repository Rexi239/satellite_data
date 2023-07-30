#include <iostream>
#include <vector>
#include <string>
#include <math.h>

#include <fstream>

using namespace std;

void read_data (
    //хочу читать файлы из папки не пихая туда .cpp -- как?
    const string & file_name,
    vector <double> & time1,
    vector <double> & time2,
    vector <int> & counts1,    // 8to10
    vector <int> & counts2,    // 10to16
    vector <int> & counts3,    // 16to25
    vector <int> & counts4,    // 25to40
    vector <int> & counts5,    // 40to64
    vector <int> & counts6,    // 64to100
    vector <int> & counts7,    // 100to160
    vector <int> & counts8,    // 160to250
    vector <int> & counts9,    // 250to400
    vector <int> & counts10,   // 400to640
    vector <int> & counts11,   // 640to1000
    vector <int> & counts12   // from1000
    )
{
    ifstream in(file_name);

    if (!in.is_open()) {
        cout << "Error opening file: " << file_name;
        return;
    }

    double t1, t2;
    int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12;

    // читаем данные массивов
    while (in.good()) {
        in >> t1 >> t2;
        in >> c1 >> c2 >> c3 >> c4 >> c5 >> c6 >> c7 >> c8 >> c9 >> c10 >> c11 >> c12;

        time1.push_back(t1);
        time2.push_back(t2);
        counts1.push_back(c1);
        counts2.push_back(c2);
        counts3.push_back(c3);
        counts4.push_back(c4);
        counts5.push_back(c5);
        counts6.push_back(c6);
        counts7.push_back(c7);
        counts8.push_back(c8);
        counts9.push_back(c9);
        counts10.push_back(c10);
        counts11.push_back(c11);
        counts12.push_back(c12);
    }

    cout << "File " << file_name << " has been read" << "\n\n";
    cout << "Total lines: " << time1.size() << '\n';
    cout << "Start time (1): " << time1[0] << '\n';
    cout << "End time (1): " << time1[time1.size()-1] << "\n\n";

    in.close();
}

int main() {
    // можно сделать штуку, которая выведет доступные название и path, чтобы копировать

    // выбираем из какого файла читать данные // FIX IT!
    /* string file_name;
    cout << "Enter the file name with quotation marks: ";
    cin >> file_name; */

    string file_name = "data\\krf20090227_49415_1_S1.thr";

    // задаем массивы для записи данных
    vector <double> time1;
    vector <double> time2;

    vector <int> counts1;    // 8to10
    vector <int> counts2;    // 10to16
    vector <int> counts3;    // 16to25
    vector <int> counts4;    // 25to40
    vector <int> counts5;    // 40to64
    vector <int> counts6;    // 64to100
    vector <int> counts7;    // 100to160
    vector <int> counts8;    // 160to250
    vector <int> counts9;    // 250to400
    vector <int> counts10;   // 400to640
    vector <int> counts11;   // 640to1000
    vector <int> counts12;   // from1000

    // читаем файл
    read_data(
              file_name,
              time1, time2,
              counts1, counts2, counts3, counts4, counts5, counts6,
              counts7, counts8, counts9, counts10, counts11, counts12
        );

    return 0;
}
