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
        vector <vector <int> > & c
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

        c[0].push_back(c1);
        c[1].push_back(c2);
        c[2].push_back(c3);
        c[3].push_back(c4);
        c[4].push_back(c5);
        c[5].push_back(c6);
        c[6].push_back(c7);
        c[7].push_back(c8);
        c[8].push_back(c9);
        c[9].push_back(c10);
        c[10].push_back(c11);
        c[11].push_back(c12);
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

// ищет первый всплеск на заданном диапазоне
void burst_search(
        bool & all_bursts_found,
        double & last_burst_end_time,
        const double & Ti,        // начало интервала поиска всплеска
        const double & Tf,        // конец интервала поиска всплеска // избыточно
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
        if (arr_time[i] <= Ti) {continue;}
        if (arr_time[i] >= Tf) {break;}

        // нет смысла начинать суммировать с бина ниже фона
        if (arr_counts[i] < bg_level) {continue;}

        int length = 0;
        double C_tot = 0; // полное число отсчётов на выбранном интервале

        for (int j = i; j < i_max; ++j) {

            length++;
            C_tot += arr_counts[j];
            double C_bg = bg_level * length; // число отсчётов от фона на выбранном интервале
            double frac = (C_tot - C_bg) / sqrt(C_bg);

            if (frac > threshold && arr_time[i] != arr_time[j]) {
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
        all_bursts_found = true;
        cout << "No (more) bursts found.\n";
        return;
    }

    cout.precision(6);
    //minimalistic output
    cout << "Burst: from " << burst_begin_time << " to " << burst_end_time << ". Bg. level: " << bg_level << ".\n";

    // extended output
    /* cout << "Burst found!" << '\n';
    cout << "Bg. level: " << bg_level << '\n';
    cout << "Start time: " << burst_begin_time << '\n';
    cout << "End time: " << burst_end_time << '\n';
    cout << "Duration: " << dt_burst << '\n';
    cout << "Counts tot: " << counts_tot << '\n';
    cout << "Counts bg: " << counts_bg << '\n';
    cout << "Significance: " << frac_detected << '\n'; */

    last_burst_end_time = burst_end_time;

}

void where_are_the_bursts(
        const double & data_begin_time,
        const double & data_end_time,
        const double & threshold, // порог детектирования
        const vector <double> & time,
        const vector <int> & counts
)
{
    bool all_bursts_found = false;
    double last_burst_end_time = data_begin_time;

    while (!all_bursts_found) {

        // будем рассчитывать bg_level на отрезке фиксированной длины, который начинается с конца последнего всплеска
        double k = 20.0; // длина отрезка расчета фона

        // задаем границы поиска уровня фона
        double bg_begin_time = last_burst_end_time;
        double bg_end_time = data_end_time;

        if (last_burst_end_time < data_end_time - k) {
            bg_end_time = last_burst_end_time + k;
        } else {
            bg_begin_time = data_end_time - k;
        }

        // определяем уровень фона в интересующем нас всплеске
        double bg_level = get_bg_level(
                bg_begin_time,
                bg_end_time,
                time,
                counts
        );

        // анализируем на всплески
        burst_search(
                all_bursts_found,
                last_burst_end_time,
                last_burst_end_time,
                data_end_time,
                threshold,
                bg_level,
                time,
                counts
                );
    }
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
            return "you have selected a missing option.\n";
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
            return "you have selected a missing option.\n";
    }
}

vector<int> select_energy_interval(vector <vector <int> > & c){
    cout << "Select energy interval according to the table in ReadMe.\nEnter the chosen number (1-12): ";
    int counts_number;
    cin >> counts_number;

    switch (counts_number) {
        case 1:
            return copy(c[0]);
        case 2:
            return copy(c[1]);
        case 3:
            return copy(c[2]);
        case 4:
            return copy(c[3]);
        case 5:
            return copy(c[4]);
        case 6:
            return copy(c[5]);
        case 7:
            return copy(c[6]);
        case 8:
            return copy(c[7]);
        case 9:
            return copy(c[8]);
        case 10:
            return copy(c[9]);
        case 11:
            return copy(c[10]);
        case 12:
            return copy(c[11]);
        default:
            cout << "You chose a missing option.\n";
            vector <int> nothing;
            return nothing;
    }
}

int main() {
    // выбираем, из какого файла читать данные
    string file_name = select_input_file();

    // задаем массивы для записи данных
    vector <double> time1;
    vector <double> time2;
    vector <vector <int> > c (12); // counts[i]

    const double threshold = 5; // порог (значимость) детектирования

    // читаем файл
    read_data(file_name,time1, time2,c);

    // анализируем набор данных на наличие всплесков
    where_are_the_bursts(
            time1[0],
            time1[time1.size() - 1],
            threshold,
            time1,
            select_energy_interval(c) // выбираем энергетический диапазон
    );

    return 0;
}

// Не стоит это читать =) Это наброски моих следующих действий и размышления.

// как проверить корректность работы burst_search? всплески незаметны?
// улучшить алгоритм определения bg_level
