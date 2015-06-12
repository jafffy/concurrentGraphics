#ifndef STATISTIC_H_
#define STATISTIC_H_

#include <deque>
#include <set>

class FPSStatistic
{
public:
	double meanFPS;
	double stddevFPS;

	int numberOfFPS;

	std::set<double> FPSs;

	FPSStatistic() : meanFPS(0.0), stddevFPS(0.0), numberOfFPS(0) {

	}

	void update(int newFPS) {
		FPSs.insert(newFPS);
	}

	void commit() {
		std::deque<double> removal;
		for (double fps : FPSs) {
			removal.push_back(fps);
		}

		for (unsigned int i = 0; i < FPSs.size() * 0.15; ++i) {
			removal.pop_front();
			removal.pop_back();
		}

		for (double fps : removal) {
			printf("%lf\n", fps);
			meanFPS += fps;
			stddevFPS += fps*fps;
		}

		meanFPS /= removal.size();
		stddevFPS /= removal.size();
		stddevFPS -= meanFPS*meanFPS;
		stddevFPS = sqrt(stddevFPS);
	}
};

#endif // STATISTIC_H_