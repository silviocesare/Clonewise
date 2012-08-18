#include "WekaClassifier.h"

void
WekaClassifier::PrintInstance(std::ofstream &testStrean, FeatureVector *v)
{
        if (testStream) {
#pragma omp critical
                {
                        for (int i = 0; i < NFEATURES; i++) {
                                if (!UseFeatureSelection || Features[i].Use) {
                                        *testStream << featureVector[i] << ",";
                                }
                        }
                        *testStream << cl << "\n";
                }
        }
}

void
WekaClassifier::PrintArffHeader(std::ofstream &testStream, Feature *features)
{
        testStream << "@RELATION Clones\n";

        for (int i = 0; features[i].Name; i++) {
                if (!UseFeatureSelection || features[i].Use) {
                        testStream << "@Attribute " << features[i].Name << " NUMERIC\n";
                }
        }

        testStream << "@ATTRIBUTE CLASS {Y,N}\n";
        testStream << "@DATA\n";
}

int
WekaClassifier::Classify(std::vector<FeeatureVector *> &instances)
{
	PrintArffHeader();
	for (int i = 0; i < instances.size(); i++) {
		PrintInstance(instances[i]);
	}
	snprintf(cmd, sizeof(cmd), "java -Xmx1024m -cp /usr/share/java/weka.jar 
weka.classifiers.trees.RandomForest -l /var/lib/Clonewise/clones/weka/model2 -T 
%s -p 0", testFilename);
        p = popen(cmd, "r");
        if (p == NULL) {
                fprintf(stderr, "Can't popen (%s): %s\n", strerror(errno), cmd);
                return -1;
        }
        for (int i = 0; i < 5; i++) {
                fgets(str, sizeof(str), p);
        }

	for (int i = 0; i < instances.size(); i++) {
		str[27] = 0;
		fgets(str, sizeof(str), p);

		if (str[27] == 'Y') {
			if (1 || (str[35] == '1' || (str[37] - '0') >= 8))
				FeatureVector.m_Class = true;
			} else {
				FeatureVector.m_Class = false;
			}
		} else {
			FeatureVector.m_Class = false;
		}
	}
	pclose(p);
	return 0;
}

Classifier*
WekaSupervisedLearner::Train(const std::vector<FeatureVector *> &instances, const std::string &modelFilename)
{
	char cmd[1024];
	char testFilename[L_tmpname + 128];

        tmpnam(t);
        snprintf(testFilename, sizeof(testFilename), "%s.arff", t);
        testStream.open(testFilename);
        if (!testStream) {
                fprintf(stderr, "Can't write test.arff\n");
                return NULL;
        }

	classifier = new WekaClassifier(testFilename, modelFilename.c_str());
	classifier->PrintArffHeader();
	for (int i = 0; i < instances.size(); i++) {
		classifier->PrintInstance(instances[i]);
	}
	snprintf(cmd, sizeof(cmd), "java -cp /usr/share/java/weka.jar weka.classifiers.trees.RandomForest -I 10 -K 0 -S 1 -d %s -t %s", modelFilename.c_str(), testFilename);
	system(cmd);
	return classifier;
}
