#ifndef TIMER_H
#define TIMER_H


typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;


struct TimeResults {
    double mean;
    double stdev;
    double median; 
    int size; 

    friend std::ostream& operator<<(std::ostream& os, const TimeResults& t){
        if (t.size == 1){
            return os << t.mean;
        }else {
            return os << "mean: " << t.mean << std::endl
                << "stdev: " << t.stdev << std::endl
                << "median: " << t.median;
        }
    }
};

class Timer {

public:
    void start() {
        begin = std::chrono::steady_clock::now();
    } 

    void stop(){
        auto end = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration<double, std::milli>(end-begin).count();
        times.push_back(diff);

    }

    TimeResults getResult(){
        int repeat = static_cast<int>(times.size());
        TimeResults result; 

        double mean = 0.0;
        sort(times.begin(), times.end());

        // remove 10% worst and best cases 
        const int truncate = static_cast<int>(repeat * 0.1);
        mean = accumulate(times.begin() + truncate, times.end() - truncate, 0.0);
        int size = repeat - 2*truncate;
        result.size = size; 
        mean = mean/size; 
        result.mean = mean;


        std::vector<double> diff(size);
        transform(times.begin() + truncate, times.end() - truncate, diff.begin(), [mean](double x) { return x-mean; });
        double sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
        result.stdev = std::sqrt(sq_sum / size);
        result.median = (size % 2) ? times[size/2] : (times[size/2-1] + times[size/2]) / 2;

        return result;
    }
private:
    std::vector<double> times;
    TimePoint begin;
};

#define BENCHMARK(CODE, TIMES, RES) { \
    Timer timer;                      \
    for (int i = 0; i < TIMES; ++i){  \
        timer.start();                \
        CODE;                         \
        timer.stop();                 \
    }                                 \
    RES = timer.getResult();          \
}                                     \


#endif 
