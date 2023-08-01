#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include <fstream>

using namespace std;

vector <int> copy(vector <int> & v) {
    vector <int> result;
    for (int i = 0; i < v.size(); ++i)
        result.push_back(v[i]);
    return result;
}

void read_data (
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
    cout << "Start time: " << time1[0] << '\n';
    cout << "End time: " << time1[time1.size()-1] << "\n\n";

    in.close();
}

double get_bg_level(
        const double & Ti, // начало интервала определения фона
        const double & Tf, // конец интервала определения фона
        const vector <double> & arr_time,
        const vector <int> & arr_counts
)
{
    // определяем уровень фона
    double sum = 0;
    int counter = 0;
    for (int i = 0; i <= arr_time.size(); ++i) {
        if (arr_time[i] < Ti) {continue;}
        if (arr_time[i] > Tf) {break;}

        sum += arr_counts[i];
        counter++;
    }

    return sum / counter;
}

// minimalistic edition
void burst_search(
        //const double & Ti,        // начало интервала поиска всплеска
        //const double & Tf,        // конец интервала поиска всплеска
        const double & threshold, // порог детектирования
        const double & bg_level,  // уровень фона
        const vector <double> & arr_time,
        const vector <int> & arr_counts
)
{
    double
            burst_begin_time, // время начала всплеска
    burst_end_time,   // время конца всплеска
    counts_tot,       // полное число отсчётов при поиске превышения
    counts_bg,        // число отсчётов фона при поиске превышения
    frac_detected,    // значимость обнаруженного всплеска
    dt_burst;         // длителность интервала всплеска

    bool excess_found = false;

    // ограничиваем конец интервала поиска
    // caution! учитываем, что данные подаются с интервалом в одну секунду
    //int Tf_i = Tf - arr_time[0];
    //int i_max = Tf_i;

    // общий случай (конец интервала поиска -- конец массива)
    int i_max = arr_counts.size();

    for (int i = 0; i < arr_counts.size(); ++i) {
        //if (arr_time[i] < Ti) {continue;}
        //if (arr_time[i] == Tf) {break;}

        // нет смысла начинать суммировать с бина ниже фона
        if (arr_counts[i] < bg_level) {continue;}

        int length = 0;
        double C_tot = 0; // полное число отсчётов на выбранном интервале

        for (int j = i; j < i_max; ++j) {

            length++;
            C_tot += arr_counts[j];
            double C_bg = bg_level * length; // число отсчётов от фона на выбранном интервале
            double frac = (C_tot - C_bg) / sqrt(C_bg);

            if (frac > threshold) {
                burst_begin_time = arr_time[i];
                burst_end_time = arr_time[j];
                counts_tot = C_tot;
                counts_bg = C_bg;
                frac_detected = frac;
                dt_burst = burst_end_time - burst_begin_time;

                excess_found = true;
                i_max--;
            }
        }
    }

    if(!excess_found) {
        cout << "Bursts not found.\n";
        return;
    }

    cout.precision(4);
    cout << "Burst search results:" << '\n';
    cout << "Start time: " << burst_begin_time << '\n';
    cout << "End time: " << burst_end_time << '\n';
    cout << "Duration: " << dt_burst << '\n';
    cout << "Counts tot: " << counts_tot << '\n';
    cout << "Counts bg: " << counts_bg << '\n';
    cout << "Significance: " << frac_detected << '\n';
}

void where_are_my_bursts(
        const double & data_begin_time,
        const double & bg_end_time,
        const double & threshold, // порог детектирования
        const vector <double> & time,
        const vector <int> & counts
)
{
    // определяем уровень фона в интересующем нас всплеске
    double bg_level = get_bg_level(
            data_begin_time,
            bg_end_time,
            time,
            counts
    );
    cout << "Bg. level: " << bg_level << "\n\n";

    // анализируем на всплески
    burst_search(threshold, bg_level, time, counts);
}

string select_input_file_extended() {
    cout << "You can read any file from list below. \n\n";
    // мб как-то по другому отображать файлы, например, даты+
    cout << "1. krf20090227_49415_1_S1.thr\n";
    cout << "2. krf20090406_62535_1_S1.thr\n";
    cout << "3. krf20090409_53058_1_S2.thr\n";
    cout << "4. krf20090523_34077_1_S2.thr\n";
    cout << "5. krf20090605_74449_1_S1.thr\n";
    cout << "6. krf20090718_65864_1_S2.thr\n";
    cout << "7. krf20090719_5488_1_S2.thr\n";
    cout << "8. krf20090804_73601_1_S2.thr\n";
    cout << "9. krf20090929_16384_1_S2.thr\n";
    cout << "\nEnter the number of the selected file to read it: ";

    int file_number;
    cin >> file_number;

    switch(file_number){
        case 1:
            return "data\\krf20090227_49415_1_S1.thr";
        case 2:
            return "data\\krf20090406_62535_1_S1.thr";
        case 3:
            return "data\\krf20090409_53058_1_S2.thr";
        case 4:
            return "data\\krf20090523_34077_1_S2.thr";
        case 5:
            return "data\\krf20090605_74449_1_S1.thr";
        case 6:
            return "data\\krf20090718_65864_1_S2.thr";
        case 7:
            return "data\\krf20090719_5488_1_S2.thr";
        case 8:
            return "data\\krf20090804_73601_1_S2.thr";
        case 9:
            return "data\\krf20090929_16384_1_S2.thr";
        default:
            return "you chose a missing option.\n";
    }
}

string select_input_file() {
    cout << "Enter the file number 1-9 to read it (0 - if you need help): ";

    int file_number;
    cin >> file_number;

    switch(file_number){
        case 0:
            return select_input_file_extended();
        case 1:
            return "data\\krf20090227_49415_1_S1.thr";
        case 2:
            return "data\\krf20090406_62535_1_S1.thr";
        case 3:
            return "data\\krf20090409_53058_1_S2.thr";
        case 4:
            return "data\\krf20090523_34077_1_S2.thr";
        case 5:
            return "data\\krf20090605_74449_1_S1.thr";
        case 6:
            return "data\\krf20090718_65864_1_S2.thr";
        case 7:
            return "data\\krf20090719_5488_1_S2.thr";
        case 8:
            return "data\\krf20090804_73601_1_S2.thr";
        case 9:
            return "data\\krf20090929_16384_1_S2.thr";
        default:
            return "you chose a missing option.\n";
    }
}

int select_energy_interval(){
    cout << "Select the energy interval according to table in ReadMe.\nEnter the chosen number (1-12): ";
    int counts_number;
    cin >> counts_number;
    return counts_number;
}

int main() {
    // выбираем из какого файла читать данные
    string file_name = select_input_file();
    // string file_name = "data\\krf20090227_49415_1_S1.thr";

    // определяем границы интервала поиска всплеска
    const double data_begin_time = -125.0; // начало всплеска
    const double data_end_time = 125.0; // конец всплеска
    const double bg_end_time = -70.0; // конец фонового интервала

    const double threshold = 5; // порог (значимость) детектирования

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

    // выбираем диапазон всплеска
    int counts_number = select_energy_interval();
    // cerr << counts_number << endl;

    // как сделать менее громоздко?
    vector <int> counts;
    switch (counts_number) {
        case 1:
            counts = copy(counts1);
            break;
        case 2:
            counts = copy(counts2);
            break;
        case 3:
            counts = copy(counts3);
            break;
        case 4:
            counts = copy(counts4);
            break;
        case 5:
            counts = copy(counts5);
            break;
        case 6:
            counts = copy(counts6);
            break;
        case 7:
            counts = copy(counts7);
            break;
        case 8:
            counts = copy(counts8);
            break;
        case 9:
            counts = copy(counts9);
            break;
        case 10:
            counts = copy(counts10);
            break;
        case 11:
            counts = copy(counts11);
            break;
        case 12:
            counts = copy(counts12);
            break;
        default:
            cout << "You chose a missing option.\n";
            return 0;
    }

    // анализируем набор данных на наличие всплесков
    where_are_my_bursts(
            time1[0],
            time1[time1.size() - 1],
            threshold,
            time1,
            counts
    );

    return 0;
}

// Не стоит это читать =) Это наброски моих следующих действий и размышления.

// почему burst_search плохо работает?

// написать нормальную функцию, которая находит все всплески, а не только первый
/*  поиск по высоким точкам + алгоритм запускается для каждой -- так себе идея
    поиск по значимым превышениям типа n * bg_level и выбор интервала до таких точек?
    но тогда проблемы, если bg_level близок к нулю
*/
