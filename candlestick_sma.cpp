#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

struct PriceData {
    string timestamp;
    double price;
};

struct Candle {
    string time;
    double open;
    double high;
    double low;
    double close;
    double volume;
};

// Function to calculate SMA
vector<double> calculateSMA(const vector<Candle>& candles, int smaPeriod) {
    vector<double> smaValues;
    for (int i = smaPeriod - 1; i < candles.size(); ++i) {
        double sum = 0;
        for (int j = i - smaPeriod + 1; j <= i; ++j) {
            sum += candles[j].close;
        }
        smaValues.push_back(sum / smaPeriod);
    }
    return smaValues;
}

// Function to generate candlesticks from price data
vector<Candle> generateCandlesticks(const vector<PriceData>& priceData, int candleLengthMinutes) {
    vector<Candle> candles;
    int dataPointsPerCandle = candleLengthMinutes;

    for (size_t i = 0; i < priceData.size(); i += dataPointsPerCandle) {
        Candle candle;
        candle.time = priceData[i].timestamp; // Start time of the candle
        candle.open = priceData[i].price;
        candle.high = priceData[i].price;
        candle.low = priceData[i].price;
        candle.close = priceData[i + dataPointsPerCandle - 1].price; // Assuming complete data for each candle
        candle.volume = 0; // You'll need to calculate volume based on your data

        for (size_t j = i; j < min(i + dataPointsPerCandle, priceData.size()); ++j) {
            candle.high = max(candle.high, priceData[j].price);
            candle.low = min(candle.low, priceData[j].price);
            // Calculate volume here if your data provides it
        }
        candles.push_back(candle);
    }
    return candles;
}

int main() {
    string inputFilename = "ETHUSDT_1.csv";
    int candleLengthMinutes = 5; // Example value, get this from user input
    int smaPeriod = 20;         // Example value, get this from user input

    ifstream inputFile(inputFilename);
    if (!inputFile.is_open()) {
        cerr << "Error opening input file: " << inputFilename << endl;
        return 1;
    }

    vector<PriceData> priceData;
    string line;
    getline(inputFile, line); // Skip header line if present

    while (getline(inputFile, line)) {
        stringstream ss(line);
        string timestamp, priceStr;
        getline(ss, timestamp, ',');
        getline(ss, priceStr, ','); 

        PriceData data;
        data.timestamp = timestamp;
        data.price = stod(priceStr); 
        priceData.push_back(data);
    }
    inputFile.close();

    // Generate candlesticks
    vector<Candle> candles = generateCandlesticks(priceData, candleLengthMinutes);

    // Calculate SMA
    vector<double> smaValues = calculateSMA(candles, smaPeriod);

    // Write candlesticks to CSV file
    ofstream candleOutput("candlesticks.csv");
    candleOutput << "Time,Open,High,Low,Close,Volume" << endl;
    for (const auto& candle : candles) {
        candleOutput << candle.time << ","
                     << candle.open << ","
                     << candle.high << ","
                     << candle.low << ","
                     << candle.close << ","
                     << candle.volume << endl;
    }
    candleOutput.close();

    // Write SMA to CSV file
    ofstream smaOutput("sma.csv");
    smaOutput << "Time,SMA" << endl;
    for (size_t i = 0; i < smaValues.size(); ++i) {
        smaOutput << candles[i + smaPeriod - 1].time << ","  // Align SMA with the corresponding candle
                  << smaValues[i] << endl;
    }
    smaOutput.close();

    cout << "Candlesticks and SMA calculated and written to CSV files." << endl;

    return 0;
}