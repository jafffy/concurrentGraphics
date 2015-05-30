#ifndef REFERENCECOUNTED_H_
#define REFERENCECOUNTED_H_

class ReferenceCounted
{
public:
	ReferenceCounted()
	: referenceCounter(1)
	{
	}

	int grab()
	{
		return ++referenceCounter;
	}

	void drop()
	{
		if (referenceCounter - 1 == 0) {
			delete this;
		} else {
			--referenceCounter;
		}
	}

	int const getReferenceCounter() const { return referenceCounter; }


protected:
	int referenceCounter;
};

#endif // REFERENCECOUNTED_H_
