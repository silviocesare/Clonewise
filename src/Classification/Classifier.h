#ifndef Classifier_h
#define Classifier_h

class FeatureVector {
public:
	float *m_FeatureVector;
	int m_Dimensionality;

	FeatureVector(int n) : m_Dimensionality(n) { m_FeatureVector = new float[m_Dimensionality]; }
	~FeatureVector() { delete [] m_FeatureVector; }
};

class Classifier {
public:
	Classifier(Feature *features) {
		m_Features = features;

		for (m_Dimensionality = 0; features[m_Dimensionality].m_Name; m_Dimensionality++);
	}
	int m_Dimensionality;
	Features *m_Features;

	static FeatureVector *NewFeatureVector() { return new FeatureVector[m_Dimensionality]; }
	virtual int Classify(std::vector<FeeatureVector *> &instances) = 0;
};

class SupervisedLearner {
public:
	virtual Classifier *Train(const std::vector<FeatureVector *> &instances) = 0;
};

#endif
