#include "Clonewise.h"
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <cstdio>
#include <cstdlib>

static std::map<std::string, std::set<std::string> > embeddedList;

static void
LoadEmbeddedCodeCopiesList(const char *filename)
{
	std::ifstream stream;
	char s[1024];

	stream.open(filename);
	if (!stream) {
		fprintf(stderr, "Can't open embedded code copies list\n");
		exit(1);
	}
	while (!stream.eof()) {
		std::string str, s1, s2;
		size_t n;

		stream.getline(s, sizeof(s));
		if (s[0] == 0)
			break;
		str = std::string(s);
		n = str.find_first_of('/');
		s1 = str.substr(0, n);
		s2 = str.substr(n + 1);
		if (packagesSignatures.find(s1) == packagesSignatures.end())
			continue;
		if (packagesSignatures.find(s2) == packagesSignatures.end())
			continue;
		embeddedList[s1].insert(s2);
	}
	stream.close();
}

static void
printFeatureVector(int cl, float *featureVector)
{
	int i;

	printf("FEATUREVECTOR:%i ", cl);
	for (i = 1; i < NFEATURES; i++) {
		printf("%i:%f ", i, featureVector[i - 1]);
	}
	printf("%i:%f\n", i, featureVector[i - 1]);
}

void
DoStatistics1()
{
	std::map<std::string, std::set<std::string> >::const_iterator iter1;
	int match, total;

	match = 0;
	total = 0;
	for (	iter1  = embeddedList.begin();
		iter1 != embeddedList.end();
		iter1++)
	{
		std::set<std::string>::const_iterator iter2;
		std::map<std::string, std::list<std::string> > *sig1, *sig2;

		sig1 = &packagesSignatures[iter1->first];
		for (	iter2  = iter1->second.begin();
			iter2 != iter1->second.end();
			iter2++)
		{
			std::map<std::string, std::list<std::string> >::const_iterator iter3;

			sig2 = &packagesSignatures[*iter2];
			for (	iter3  = sig1->begin();
				iter3 != sig1->end();
				iter3++)
			{
				if (sig2->find(iter3->first) != sig2->end()) {
					match++;
				} else {
				}
				total++;
			}
		}
	}
	printf("Probability (filename shared in embedded codee copy):%f:%i:%i\n", (float)match / (float)total, match, total);
}

void
GetScoreForNotEmbedded(float &score)
{
	std::map<std::string, std::list<std::string> >::const_iterator pIter;
	std::string p1, p2;
	int n1, n2, i;
	bool breakit;
	float featureVector[NFEATURES];

	do {
		n1 = rand() % packages.size();
		n2 = rand() % packages.size();
		breakit = true;
		for (	pIter  = packages.begin(), i = 0;
			pIter != packages.end() && i < n1;
			pIter++, i++);
		p1 = pIter->first;
		for (	pIter  = packages.begin(), i = 0;
			pIter != packages.end() && i < n2;
			pIter++, i++);
		p2 = pIter->first;

		if (embeddedList.find(p1) != embeddedList.end() && embeddedList[p1].find(p2) != embeddedList[p1].end())
			breakit = false;
		else if (embeddedList.find(p2) != embeddedList.end() && embeddedList[p2].find(p1) != embeddedList[p2].end())
			breakit = false;
	} while (!breakit);
	CheckForClone(packagesSignatures[p1], packagesSignatures[p2], score, featureVector);
	printFeatureVector(2, featureVector);
//	printMatch(packagesSignatures[p1], packagesSignatures[p2]);
	
}

void
DoStatistics2()
{
	std::map<std::string, std::set<std::string> >::const_iterator iter1;
	std::multiset<float> scoresEmbedded, scoresNotEmbedded;
	float maxScore;
	std::multiset<float>::const_iterator lIter1, lIter2;
	int total;
	float featureVector[NFEATURES];

	total = 0;
	maxScore = 0.0;
	for (	iter1  = embeddedList.begin();
		iter1 != embeddedList.end();
		iter1++)
	{
		std::set<std::string>::const_iterator iter2;
		std::map<std::string, std::list<std::string> > *sig1, *sig2;

		sig1 = &packagesSignatures[iter1->first];
		for (	iter2  = iter1->second.begin();
			iter2 != iter1->second.end();
			iter2++)
		{
			float score;

			sig2 = &packagesSignatures[*iter2];
			CheckForClone(*sig1, *sig2, score, featureVector);
			printFeatureVector(1, featureVector);
			total++;
printf("%s --> %s %f\n", iter1->first.c_str(), iter2->c_str(), score);
			scoresEmbedded.insert(score);
			if (score > maxScore)
				maxScore = score;
		}
	}
	for (int i = 0; i < 3000; i++) {
		float score;

		GetScoreForNotEmbedded(score);
		scoresNotEmbedded.insert(score);
		if (score > maxScore)
			maxScore = score;
	}
	lIter1 = scoresEmbedded.begin();
	lIter2 = scoresNotEmbedded.begin();
	printf("total=%i\n", total);
	printf("\"weight\",\"embedded\",\"notEmbedded\",\n");
	for (int i = 0; i < ceil(maxScore); i++) {
		int count1, count2;

		count1 = 0;
		while (lIter1 != scoresEmbedded.end() && *lIter1 < (i + 1)) {
			lIter1++;
			count1++;
		}
		count2 = 0;
		while (lIter2 != scoresNotEmbedded.end() && *lIter2 < (i + 1)) {
			lIter2++;
			count2++;
		}
		printf("%i,%i,%i,\n", i, count1, (int)((float)count2 * (float)total / 3000.0));
	}
}
			
int
main(int, char *argv[])
{
	useSSDeep = true;
//	verbose = 3;
	srand(atoi(argv[1]));
	LoadEverything();
	LoadEmbeddedCodeCopiesList(argv[1]);
//	DoStatistics1();
	DoStatistics2();
}
