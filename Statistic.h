#ifndef STATISTIC_H_
#define STATISTIC_H_

class FPSStatistic
{
public:
	double meanFPS;
	double stddevFPS;

	int numberOfFPS;

	FPSStatistic() : meanFPS(0.0), stddevFPS(0.0), numberOfFPS(0) {

	}

	void update(int newFPS) {
		meanFPS += newFPS;
		stddevFPS += newFPS*newFPS;
		++numberOfFPS;
	}

	void commit() {
		meanFPS /= numberOfFPS;
		stddevFPS /= numberOfFPS;
		stddevFPS -= meanFPS;
		stddevFPS = sqrt(stddevFPS);
	}
};

#endif // STATISTIC_H_