#include <Arduino.h>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <iostream>
#include <array>
#include <cmath>
#include <PubSubClient.h>
#include <WiFi.h>

#define VREF = 1.65
#define VGAS_0 = 2
#define V_OFFSET = (VGAS_0 - VREF)

struct CubicSpline {
    double x;
    double a;
    double b;
    double c;
    double d;

    CubicSpline() = default;

    CubicSpline(double x, double a, double b, double c, double d) {
        this->x = x;
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
    }
};

class CubicSplineSet {


    std::vector<CubicSpline> splineContainer;
    double interval;
    double start;
public:
    template<typename T, std::size_t N>
    CubicSplineSet (const double interval, const double start, const std::array<T, N>& yPoints) {


        // The algorithm used to compute the discrete spline functions can be found here => https://en.wikipedia.org/wiki/Spline_(mathematics)#Algorithm_for_computing_natural_cubic_splines

        // in math, refer to the last index as n, therefore n-1 {code} <=> n {math}
        this->interval = interval;
        this->start = start;
        constexpr std::size_t n = N;
        constexpr std::uint32_t aOffset = 0; // a size = n {math = n+1}
        constexpr std::uint32_t bOffset = n; // b sizes = n-1 {math = n}
        constexpr std::uint32_t dOffset = bOffset + n - 1; // d size = n-1 {math = n}
        constexpr std::uint32_t alphaOffset = dOffset + n - 1; // alpha size = n-1 {math = n}
        constexpr std::uint32_t cOffset = alphaOffset + n - 1; // c size = n {math = n+1}
        constexpr std::uint32_t lOffset = cOffset + n; // l size = n
        constexpr std::uint32_t muOffset = lOffset + n; // mu size = n
        constexpr std::uint32_t zOffset = muOffset + n; // z size = n

        std::array<double, zOffset+n> data {};

        for (std::uint_fast32_t i {}; i < n; i++) { // for i = 0,...,n
            data[i+aOffset] = yPoints[i]; // a[i] = y[i]
        }



        for (std::uint_fast32_t i {1}; i < n - 1; ++i) { // for i = 1,...,n-1
            data[i+alphaOffset] =
                    (3/interval)* // 3/h[i]
                    (data[i+aOffset+1] - data[i+aOffset]) - //a[i+1] - a[i]
                    (3/interval)* // 3/h[i-1]
                    (data[i+aOffset] - data[i+aOffset-1]); // a[i] - a[i-1]
        }

        data[lOffset]= 1;
        data[muOffset] = 0;
        data[zOffset] = 0;

        for (std::uint_fast32_t i {1}; i < n - 1; ++i) {
            data[i+lOffset] =
                    2 * ((start+interval*(i+1)) - (start+interval*(i-1))) - // 2 * (x[i+1] - x[i-1])
                    interval * data[i+muOffset-1]; // h[i-1] * μ[i-1]
            data[i+muOffset] =
                    interval / data[i+lOffset]; // h[i] / l[i]
            data[i+zOffset] =
                    (data[i+alphaOffset] - interval * data[i+zOffset-1]) / // α[i] - h[i-1] * z[i-1]
                    data[i+lOffset]; // l[i]
        }
        data[n - 1 + lOffset] = 1;
        data[n - 1 + zOffset] = 0;
        data[n - 1 + cOffset] = 0;

        for (std::int_fast32_t j {static_cast<int_fast32_t>(n - 2)}; j >= 0; --j) {
            data[j+cOffset] =
                    data[j+zOffset] - data[j+muOffset] * data[j+cOffset+1]; // z[j] - μ[j] * c[j+1]
            data[j+bOffset] =
                    ((data[j+aOffset+1] - data[j+aOffset]) / // a[j+1] - a[j]
                     interval) - // h[j]
                    ((interval * (data[j+cOffset+1] + 2 * data[j+cOffset])) / 3); // h[j] * (c[j+1] + 2c[j]) / 3
            data[j+dOffset] =
                    (data[j+cOffset+1] - data[j+cOffset]) / // c[j+1] - c[j]
                    (3 * interval); // 3h[j]

        }

        for (std::uint_fast32_t i {}; i < n - 1; ++i) {
            splineContainer.emplace_back(
                    start+interval*i,
                    data[i+aOffset],
                    data[i+bOffset],
                    data[i+cOffset],
                    data[i+dOffset]
            );
        }
    }
    // retrieving a number outside the spline plot space will result in undefined behaviour or undefined S(x) values
    double get_y_where_x_equals(double x) {
        auto pos = std::upper_bound(splineContainer.begin(), splineContainer.end(), x, [](double value, const CubicSpline& spline) {
            return spline.x > value;
        })-1;

        return pos->a + pos->b * (x - pos->x) + pos->c * std::pow((x - pos->x), 2) + pos->d * std::pow((x - pos->x), 3);
    }

    void print_equations() {
        for (int i {}; i < splineContainer.size(); ++i) {
            auto&[x, a, b, c, d] = splineContainer[i];
            std::cout << "Spline " << i+1 << " Equation: S(x) = " <<
                      (a >= 0 ? ' ' : '-') << std::abs(a) <<
                      (b >= 0 ? '+' : '-') << std::abs(b) << "(x-" << x << ')' <<
                      (c >= 0 ? '+' : '-') << std::abs(c) << "(x-" << x << ")^2" <<
                      (d >= 0 ? '+' : '-') << std::abs(d) << "(x-" << x << ")^3, " << '{' << start+interval*i << " <= x < " << start+interval*(i+1) << "}\n";
        }
    }
};


PubSubClient client;

void setup() {
    Serial.begin(115200);
    std::array plots {0.0, -2.1, 3.4, 6.3, -1.8};
    auto splineGraph = CubicSplineSet(10, 10, plots);
    analogReadResolution(12);

    constexpr char* SSID = "Samsnug";
    constexpr char* PASSWORD = "********";
    constexpr char* MQTT_SERVER = "test.mosquitto.org";
    constexpr int PORT = 1883;

    Serial.println("Connecting to network");
    WiFi.begin(SSID, PASSWORD);

    while (WiFiClass::status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());



    client.setServer(MQTT_SERVER, PORT);
    client.setCallback([](char* topic, byte* message, unsigned int length){
        Serial.println("Message sent");
    });

}

void loop() {
    std::uint16_t digitalIn = analogRead(34);
    client.publish("egg/fucking/smelly/asp", (std::to_string(digitalIn) + "mV").c_str());
}
