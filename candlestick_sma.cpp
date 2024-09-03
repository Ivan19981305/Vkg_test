#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>


struct PriceData {
    int time;         // Время в формате Unix timestamp
    double price;
    double volume;
};

struct Candle {
    int time;         // Время в формате Unix timestamp
    double open;
    double high;
    double low;
    double close;
    double volume;
};

struct SMA {
    int time;           // Время в формате Unix timestamp
    double sma;         // Значение SMA
};

// Функция для преобразования Unix timestamp в строку ISO 8601
std::string timestamp_to_iso8601(int timestamp) {
    time_t tt = timestamp;
    tm tm = *gmtime(&tt);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &tm);
    return std::string(buffer);
}

// Функция для расчета SMA
std::vector<SMA> calculateSMA(const std::vector<Candle>& candles, int smaPeriod) {
    std::vector<SMA> smaValues;                  // Вектор для хранения значений SMA
    std::vector<double> smaCalculate(smaPeriod); // Вектор для расчета SMA
    int calculated = 0;                          // Количество рассчитанных значений SMA
    double total;                                // Сумма цен закрытия для расчета SMA

    SMA currentSMA;

    if (!smaPeriod)
        return smaValues;

    if (candles.size())
    {   // Инициализация первого значения SMA
        currentSMA.time = candles[0].time;
        currentSMA.sma = candles[0].close;

        smaCalculate[calculated] = currentSMA.sma;
        calculated++;
        
        total = currentSMA.sma;

        smaValues.push_back(currentSMA);
    }

    // Расчет SMA для остальных свечей
    for (size_t i = 1; i < candles.size(); ++i)
    {
        // Проверка, прошёл ли хотя бы день
        if (candles[i].time - currentSMA.time >= 60 * 60 * 24)
        {
            for(int j = currentSMA.time; j < candles[i].time; j += 60 * 60 * 24)
            {
                // Добавление значений SMA за прошедшие дни
                total += candles[i - 1].close;
                if (calculated < smaPeriod)
                {
                    smaCalculate[calculated] = candles[i - 1].close;
                    calculated++;
                }
                else
                {
                    total -= smaCalculate[calculated % smaPeriod];
                    smaCalculate[calculated % smaPeriod] = candles[i - 1].close;
                    calculated++;
                }

                currentSMA.time = currentSMA.time + 60 * 60 * 24;
                currentSMA.sma = total / std::min(calculated, smaPeriod);

                smaValues.push_back(currentSMA);
            }          
        }
    }

    return smaValues;
}

// Функция для генерации свечей из данных о ценахa
std::vector<Candle> generateCandlesticks(const std::vector<PriceData>& priceData, int candleLengthMinutes) {
    std::vector<Candle> candles;
    
    Candle currentCandle;
    currentCandle.time = 0; // Инициализация времени начала свечи

    for (size_t i = 0; i < priceData.size(); ++i) {
        // Проверка, нужно ли начать новую свечу
        if (priceData[i].time - currentCandle.time >= 60 * candleLengthMinutes)
        {
            if (currentCandle.time)
                candles.push_back(currentCandle); // Добавление предыдущей свечи в вектор

            currentCandle.time = priceData[i].time;
            currentCandle.open = priceData[i].price;
            currentCandle.high = priceData[i].price;
            currentCandle.low = priceData[i].price;
            currentCandle.close = priceData[i].price;
            currentCandle.volume = priceData[i].volume;
        }
        else
        {   // Обновление значений текущей свечи
            currentCandle.high = std::max(currentCandle.low, priceData[i].price);
            currentCandle.low = std::min(currentCandle.low, priceData[i].price);
            currentCandle.close = priceData[i].price;
            currentCandle.volume += priceData[i].volume;
        }        
    }
    
    // Добавление последней свечи
    if (priceData.size())
        candles.push_back(currentCandle);

    return candles;
}

int main() {
    std::string inputFilename = "ETHUSDT_1.csv";
    int candleLengthMinutes = 5; // Длина свечи в минутах, пример
    int smaPeriod = 20;          // Период SMA, пример

    std::ifstream inputFile(inputFilename);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file: " << inputFilename << std::endl;
        return 1;
    }

    std::vector<PriceData> priceData; // Вектор для хранения данных о ценах
    std::string line;
    std::getline(inputFile, line); // Пропуск заголовка

    //Парсим входной csv
    while (std::getline(inputFile, line)) {
        std::stringstream ss(line);
        std::string timeString, priceString, volumeString;
        std::getline(ss, timeString, ',');
        std::getline(ss, priceString, ','); 
        std::getline(ss, volumeString); 

        PriceData data;
        data.time = stoi(timeString);
        data.price = stod(priceString); 
        data.volume = stod(volumeString); 
        priceData.push_back(data);
    }
    inputFile.close();

    // Сортировка данных по времени
    std::sort(priceData.begin(), priceData.end(), [](const PriceData& a, const PriceData& b) {
    return a.time < b.time;
  });

    // Генерация свечей
    std::vector<Candle> candles = generateCandlesticks(priceData, candleLengthMinutes);

    // Расчет SMA
    std::vector<SMA> smaValues = calculateSMA(candles, smaPeriod);

    // Запись свечей в CSV файл
    std::ofstream candleOutput("candlesticks.csv");
    candleOutput << "Time,Open,High,Low,Close,Volume" << std::endl;
    for (const auto& candle : candles) {
        candleOutput << timestamp_to_iso8601(candle.time) << ","
                     << candle.open << ","
                     << candle.high << ","
                     << candle.low << ","
                     << candle.close << ","
                     << candle.volume << std::endl;
    }
    candleOutput.close();

    // Запись SMA в CSV файл
    std::ofstream smaOutput("sma.csv");
    smaOutput << "Time,SMA" << std::endl;
    for (size_t i = 0; i < smaValues.size(); ++i) {
        smaOutput << timestamp_to_iso8601(smaValues[i].time) << ","  
                  << smaValues[i].sma << std::endl;
    }
    smaOutput.close();

    std::cout << "Свечи и SMA рассчитаны и записаны в CSV файлы." << std::endl;

    return 0;
}