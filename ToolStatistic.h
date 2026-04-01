#ifndef TOOLSTATISTIC_H
#define TOOLSTATISTIC_H


// Helper function to calculate the median of a sorted range.
double median(const std::vector<double>& sorted, size_t start, size_t end) {
    size_t size = end - start;
    size_t mid = start + size / 2;

    if (size % 2 == 0) {
        return (sorted[mid - 1] + sorted[mid]) / 2.0;
    } else {
        return sorted[mid];
    }
}

// Function to calculate the box plot bounds.
std::tuple<double, double> GetBoxPlotBounds(const std::vector<double>& samples) {
    if (samples.empty()) {
        throw std::invalid_argument("Input vector is empty");
    }

    // Create a copy of the samples and sort it.
    std::vector<double> sortedSamples = samples;
    std::sort(sortedSamples.begin(), sortedSamples.end());

    size_t n = sortedSamples.size();

    // Calculate Q1 (lower quartile) and Q3 (upper quartile).
    double Q1 = median(sortedSamples, 0, n / 2);
    double Q3 = median(sortedSamples, (n + 1) / 2, n);

    // Calculate the interquartile range (IQR).
    double IQR = Q3 - Q1;

    // Calculate the lower and upper bounds of the box.
    double lowerBound = Q1 - 1.5 * IQR;
    double upperBound = Q3 + 1.5 * IQR;

    return {lowerBound, upperBound};
}

template<typename T>
struct Statistic{
    std::set<T> unique_elements;
    std::vector<T> data_set;
    std::vector<T> errors;
    size_t n_of_values= 0;
    T sum = 0;
    T sum2 = 0;
    T minimum;
    T maximum;
    T q;

    void Add(T val, T error){
        errors.push_back(error);
        Add(val);
    }
    void Add(T val){
        data_set.push_back(val);
        sum += val;
        sum2 += val*val;

        if (!n_of_values){
            minimum = val;
            maximum = val;
        }else{
            minimum = std::min(minimum, val);
            maximum = std::max(maximum, val);
        }

        unique_elements.insert(val);

        n_of_values++;
    }

    T Mean(){
        return sum / n_of_values;
    }
    T Variance(){
        return (sum2 * n_of_values - sum*sum) / (n_of_values*n_of_values);
    }

    T RMS(){
        return std::sqrt(Variance());
    }

    void operator=(const Statistic& h){ }
};

#endif