#ifndef WekaClassifier_h
#define WekaClassifier_h

#include "Classifier.h"

class WekaClassifier : public Classifier {
public:
	WekaClassifier(Feature *features) : Classifier(features) {}

	int Classify(std::vector<FeeatureVector *> &instances);
};

class WekaSupervisedLearner : public SupervisedLearner {
public:
	Classifier *Train(const std::vector<FeatureVector *> &instances);
};

#endif
