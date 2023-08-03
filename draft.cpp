#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include <fstream>

using namespace std;

void burst_search(
        bool & all_bursts_found,
        double & last_burst_end_time,
        const double & Ti,        // начало интервала поиска всплеска
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

    //конец интервала поиска -- конец массива!
    int i_max = arr_counts.size();

    for (int i = 0; i < arr_counts.size(); ++i) {
        if (arr_time[i] <= Ti) {continue;}

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

    const double static_bg_level = get_bg_level(data_begin_time, data_end_time, time, counts);
    cout << "Static bg. level: " << static_bg_level << ".\n\n";

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
        double bg_level = get_bg_level(bg_begin_time,bg_end_time,time,counts);

        // анализируем на всплески
        burst_search(
                all_bursts_found,
                last_burst_end_time,
                last_burst_end_time,
                threshold,
                bg_level,
                time,
                counts
        );
    }
}

int main() {
    cout << "Hello World!\n";
    return 0;
}