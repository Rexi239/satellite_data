#include <iostream>
#include <vector>
#include <string>
#include <math.h>

#include <fstream>

using namespace std;

/*
Разбил код на отдельные функции
Данные в фунции передаются по ссылке, чтобы не было издержек на копирование данных

*/

void read_data(
        const string & file_name,
        vector <double> & arr_time,
        vector <int> & arr_counts
)
{

    // написано на основе примера в
    // https://cplusplus.com/reference/fstream/ifstream/is_open/

    ifstream in(file_name); // окрываем файл для чтения

    if (! in.is_open())
    {
        cout << "Error opening file: " << file_name;
        return;
    }

    // читаем заголовок файла
    string s1, s2;
    in >> s1 >> s2;

    double x;
    int y;

    while (in.good())
    {
        in >> x >> y;
        arr_time.push_back(x);
        arr_counts.push_back(y);
    }

    cout << "File: " << file_name << " has been read" << endl;
    cout << "Total lines: " << arr_time.size() << endl;
    cout << "Start time: " << arr_time[0] << endl;
    cout << "End time: " << arr_time[arr_time.size()-1] << endl << endl;

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
    for (int i = 0; i <= arr_time.size(); ++i)
    {
        if (arr_time[i] < Ti) {continue;}
        if (arr_time[i] > Tf) {break;}

        sum += arr_counts[i];
        counter++;
    }

    return sum / counter;
}

void burst_search_asya(
        const double & Ti,        // начало интервала поиска всплеска
        const double & Tf,        // конец интервала поиска всплеска
        const double & threshold, // порог детектирования
        const double & bg_counts, // уровень фона
        const vector <double> & arr_time,
        const vector <int> & arr_counts
)
{
    double
            burst_begin_time, // время начала всплеска
    burst_begin_interval_end, // конец интервала, где найдено первое превышение порога
    burst_end_time,   // время конца всплеска
    counts_tot,       // полное число отсчётов при поиске превышения
    counts_bg,        // число отсчётов фона при поиске превышения
    frac_detected,    // значимость обнаруженного всплеска
    dt_burst;         // длителность интервала всплеска

    bool borders_found = false;
    bool start_interval_found = false;

    for (int i = 0; i < arr_counts.size(); ++i)
    {

        if (arr_time[i] < Ti) continue;
        if (borders_found) break;

        //cout << i << endl;

        int length = 0;
        double C_tot = 0; // полное число отсчётов на выбранном интервале

        for (int j = i; j < arr_counts.size(); ++j)
        {
            if (arr_counts[j] > bg_counts)
            {
                length++;
                C_tot += arr_counts[j];
                double C_bg = bg_counts * length; // число отсчётов от фона на выбранном интервале
                double frac = (C_tot - C_bg) / sqrt(C_bg);

                if (frac > threshold)
                {
                    burst_begin_time = arr_time[i];
                    burst_end_time = arr_time[j+1];
                    counts_tot = C_tot;
                    counts_bg = C_bg;
                    frac_detected = frac;
                    dt_burst = burst_end_time - burst_begin_time;
                    borders_found = true;
                }
                if (borders_found && !start_interval_found)
                {
                    burst_begin_interval_end = arr_time[j+1];
                    start_interval_found = true;
                }
            }
            else
            {
                break;
            }
        }
    }

    cout.precision(4);
    cout << "Burst search results:" << endl;
    cout << "Start time: " << burst_begin_time << endl;
    cout << "Start interval end: " << burst_begin_interval_end << endl;
    cout << "End time: " << burst_end_time << endl;
    cout << "Duration: " << dt_burst << endl;
    cout << "Counts tot: " << counts_tot << endl;
    cout << "Counts bg: " << counts_bg << endl;
    cout << "Significance: " << frac_detected << endl;
}

void burst_search_ds(
        const double & Ti,        // начало интервала поиска всплеска
        const double & Tf,        // конец интервала поиска всплеска
        const double & threshold, // порог детектирования
        const double & bg_counts, // уровень фона
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

    int i = 0;
    int i_max = arr_counts.size();

    while (i < arr_counts.size())
    {
        i++;
        if (arr_time[i] < Ti) continue;

        // нет смысла начинать суммировать с бина ниже фона
        if (arr_counts[i] < bg_counts) {continue;}

        //cout << i << endl;

        int length = 0;
        double C_tot = 0; // полное число отсчётов на выбранном интервале

        for (int j = i; j < i_max; ++j)
        {

            length++;
            C_tot += arr_counts[j];
            double C_bg = bg_counts * length; // число отсчётов от фона на выбранном интервале
            double frac = (C_tot - C_bg) / sqrt(C_bg);

            if (frac > threshold)
            {
                burst_begin_time = arr_time[i];
                burst_end_time = arr_time[j+1];
                counts_tot = C_tot;
                counts_bg = C_bg;
                frac_detected = frac;
                dt_burst = burst_end_time - burst_begin_time;
                excess_found = true;
                i_max --;
            }
        }

    }

    cout.precision(4);
    cout << "Burst search results:" << endl;
    cout << "Start time: " << burst_begin_time << endl;
    cout << "End time: " << burst_end_time << endl;
    cout << "Duration: " << dt_burst << endl;
    cout << "Counts tot: " << counts_tot << endl;
    cout << "Counts bg: " << counts_bg << endl;
    cout << "Significance: " << frac_detected << endl;
}

int main()
{
    const string file_name = "burst.txt";
    //const string file_name = "burst_noise.txt";

    // определяем границы интервала поиска всплеска
    const double data_begin_time = -150.0; // начало всплеска
    const double data_end_time = 100.0; // конец всплеска
    const double bg_end_time = -70.0; // конец фонового интервала

    const double threshold = 5; // порог (значимость) детектирования
    // так как значение вычисляется на основе операций с вещественными числами, тип - double

    vector <double> arr_time;
    vector <int> arr_counts;

    read_data(file_name, arr_time, arr_counts);

    double bg_counts = get_bg_level(
            data_begin_time,
            bg_end_time,
            arr_time,
            arr_counts
    );
    cout << "Bg. level: " << bg_counts << endl << endl;

    cout << "Initial method by Asya:" << endl;
    burst_search_asya(
            bg_end_time,   // начало интервала поиска всплеска
            data_end_time, // конец интервала поиска всплеска
            threshold,     // порог детектирования
            bg_counts,     // уровень фона
            arr_time,
            arr_counts
    );
    cout << endl;

    cout << "Method by DS:" << endl;
    burst_search_ds(
            bg_end_time,
            data_end_time,
            threshold,
            bg_counts,
            arr_time,
            arr_counts
    );
    cout << endl;

    return 0;
}
