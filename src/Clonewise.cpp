#include "munkres.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <map>
#include <getopt.h>
#include <set>
#include <list>
#include <cmath>
#include <cctype>
#include <unistd.h>
#include <cstring>
#include <fuzzy.h>
#include <cstdarg>
#include "Clonewise.h"
#include <omp.h>

Feature Features[] = {
	{ "N_Filenames_A", true },				// 1
	{ "N_Filenames_Source_A", true },			// 2
	{ "N_Filenames_B", true },				// 3
	{ "N_Filenames_Source_B", true },			// 4
	{ "N_Common_Filenames", true },			// 5
	{ "N_Common_Similar_Filenames", true },			// 6
	{ "N_Common_FilenameHashes", true },			// 7
	{ "N_Common_FilenameHash80", true },			// 8
	{ "N_Common_ExactFilenameHash", true },			// 9
	{ "N_Score_of_Common_Filename", true },			// 10
	{ "N_Score_of_Common_Similar_Filename", true },		// 11
	{ "N_Score_of_Common_FilenameHash", true },		// 12
	{ "N_Score_of_Common_FilenameHash80", true },		// 13
	{ "N_Score_of_Common_ExactFilenameHash80", true },	// 14
	{ "N_Data_Common_Filenames", true },			// 15
	{ "N_Data_Common_Similar_Filenames", true },		// 16
	{ "N_Data_Common_FilenameHashes", true },		// 17
	{ "N_Data_Common_FilenameHash80", true },		// 18
	{ "N_Data_Common_ExactFilenameHash", true },		// 19
	{ "N_Data_Score_of_Common_Filename", true },		// 20
	{ "N_Data_Score_of_Common_Similar_Filename", true },	// 21
	{ "N_Data_Score_of_Common_FilenameHash", true },	// 22
	{ "N_Data_Score_of_Common_FilenameHash80", true },	// 23
	{ "N_Data_Score_of_Common_ExactFilenameHash80", true },	// 24
	{ "N_Common_Hash", false },				// 25
	{ "N_Common_ExactHash", true },				// 26
	{ "N_Common_DataExactHash", true },			// 27
	{ NULL, false }
};

static void CreateFeatures();

bool UseFeatureSelection = true;
FILE *outFd = stdout;
double maxWeight = 0.0;
std::set<std::string> featureExceptions;
bool approxFilename = true;
const char *distroString = "ubuntu-11.10";
OutputFormat_e outputFormat = CLONEWISE_OUTPUT_TEXT;
bool useSSDeep = true;
bool useExtensions = true;
int verbose = 2;
std::map<std::string, float> idf;
std::set<std::string> featureSet;
std::set<std::string> ignoreFalsePositives;
unsigned int numPackages = 0;
bool allPackages = false;
std::map<std::string, std::list<std::string> > packages;
std::map<std::string, std::map<std::string, std::set<std::string> > > packagesSignatures;
std::set<std::string> extensions;
bool reportError = false;
bool useRelativePathForSignature = true;
std::map<std::string, std::string> normalFeatures;
std::map<std::string, std::set<std::string> > embeddedList;

static void
errorLog(const char *fmt, ...)
{
	if (reportError) {
		va_list ap;

		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
}

unsigned int
LongestCommonSubsequenceLength(const char *s, unsigned int m, const char *t, unsigned int n)
{
       double d[m + 1][n + 1];
       unsigned int i, j;

       for (i = 0; i <= m; i++) {
               d[i][0] = 0.0;
       }
       for (j = 0; j <= n; j++) {
               d[0][j] = 0.0;
       }
       for (j = 1; j <= n; j++) {
               for (i = 1; i <= m; i++) {
                       if (s[i - 1] == t[j - 1]) {
                               d[i][j] = d[i-1][j-1] + 1;
                       } else {
                               d[i][j] = std::max(d[i][j - 1], d[i-1][j]);
                       }
               }
       }
       return d[m][n];
}

static unsigned int
SmithWatermanDistance(const char *s, unsigned int m, const char *t, unsigned int n)
{
       double d[m + 1][2];
       unsigned int i, j;
       double cost;
       double D;

       D = 0;
       for (i = 0; i <= m; i++) {
               d[i][0] = 0.0;
       }
       d[0][0] = 0.0;
       for (j = 1; j <= n; j++) {
               d[0][j % 2] = 0.0;
               for (i = 1; i <= m; i++) {
                       const static double G = 1.0; // gap cost

                       if (s[i - 1] == t[j - 1])
                               cost =  1.0;
                       else {
                               cost =  0.0;
                       }
                       d[i][j % 2] = std::max(
                               0.0,
// start over
                               std::max(d[i - 1][(j - 1) % 2] + cost,
// substitution
                               std::max(d[i - 1][j % 2] - G,
// insertion
                               d[i][(j - 1) % 2] - G)));
// deletion
                       if (d[i][j % 2] > D)
                               D = d[i][j % 2];
               }
       }
       return (int)std::max(m, n) - D;
}

static unsigned int
LevenshteinDistance(const char *s, unsigned int m, const char *t, unsigned int n)
{
       unsigned int d[m + 1][2];
       unsigned int i, j;
       unsigned int cost;

       for (i = 0; i <= m; i++) {
               d[i][0] = i;
       }
       d[0][0] = 0;
       for (j = 1; j <= n; j++) {
               d[0][j % 2] = j;
               for (i = 1; i <= m; i++) {
                       if (s[i - 1] == t[j - 1])
                               cost = 0;
                       else
                               cost = 1;
                       d[i][j % 2] = std::min(std::min(
                               d[i - 1][j % 2] + 1,            // insertion
                               d[i][(j - 1) % 2] + 1),         // deletion
                               d[i - 1][(j - 1) % 2] + cost);  // substitution
               }
       }
       return d[m][n % 2];
}

void
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

void
normalizeFeature(std::string &normalFeature, const std::string &feature)
{
	size_t lastDot;
	int i;

	lastDot = feature.find_last_of('.');
	i = 0;
	if (feature.size() >= 4 && feature[0] == 'l' && feature[1] == 'i' && feature[2] == 'b') {
		i = 3;
	}
	for (; i < feature.size(); i++) {
		if (isdigit(feature[i]) || feature[i] == '_' || feature[i] == '-' || (i != lastDot && feature[i] == '.'))
			continue;
		normalFeature += tolower(feature[i]);
	}
	if (normalFeature.size() > 4 && !strcmp(&normalFeature.c_str()[normalFeature.size() - 4], ".cxx")) {
		normalFeature = normalFeature.substr(0, normalFeature.size() - 2);
	} else if (normalFeature.size() > 4 && !strcmp(&normalFeature.c_str()[normalFeature.size() - 4], ".cpp")) {
		normalFeature = normalFeature.substr(0, normalFeature.size() - 2);
	} else if (normalFeature.size() > 3 && !strcmp(&normalFeature.c_str()[normalFeature.size() - 3], ".cc")) {
		normalFeature = normalFeature.substr(0, normalFeature.size() - 1);
	}
	if (normalFeature.size() == 0) {
		normalFeature = feature;
	}
}

static void
lineToFeature(const char *s, std::string &feature, std::string &hash)
{
	std::string str, str2, origFeature;
	size_t n, n1, n2;

	str = std::string(s);
	n2 = str.find_first_of(',');
	hash = str.substr(0, n2 - 1);
	n1 = str.find_first_of('"');
	n2 = str.find_last_of('"');
	str2 = str.substr(n1 + 1, n2 - n1 - 1);
	n = str2.find_last_of('/');
	origFeature = str2.substr(n + 1, str2.size() - n - 1);
	normalizeFeature(feature, origFeature);
}

static void
loadFeatureExceptions()
{
	std::ifstream stream;
	char s[1024];

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/features/feature-exceptions", distroString);
	stream.open(s);
	if (!stream) {
		fprintf(stderr, "Couldn't open feature-exceptions\n");
		exit(1);
	}
	while (!stream.eof()) {
		std::string str, feature;

		stream.getline(s, sizeof(s));
		if (s[0] == 0)
			break;
		str = std::string(s);
		normalizeFeature(str, feature);
		featureExceptions.insert(feature);
	}
	stream.close();
}

static void
BuildDatabase()
{
	if (getuid() != 0) {
		fprintf(stderr, "Need to be root to build database.\n");
		exit(1);
	}
	fprintf(stderr, "Building database. This could take a considerable amount of time..\n");
	system("Clonewise-BuildDatabase");
}

static void
BuildFeatures()
{
	if (getuid() != 0) {
		fprintf(stderr, "Need to be root to build database.\n");
		exit(1);
	}
	CreateFeatures();
}

bool
hasSameExtension(const std::string &x, const std::string &y)
{
	size_t n1, n2;

	n1 = x.find_last_of('.');
	n2 = y.find_last_of('.');
	if (n1 == std::string::npos && n2 == std::string::npos)
		return true;
	return x.substr(n1 + 1) == y.substr(n2 + 1);
}

bool
approxMatchFilenames(const std::string &x, const std::string &y)
{
	int xs, ys;
	unsigned int d;
	float s;
//	float s_min, s_max;
	int m_min, m_max;

	if (!hasSameExtension(x, y)) {
		return false;
	}
	xs = x.size();
	ys = y.size();
	m_max = std::max((double)xs, (double)ys);
	m_min = std::min((double)xs, (double)ys);
	//d = SmithWatermanDistance(x.c_str(), xs, y.c_str(), ys);
	d = LevenshteinDistance(x.c_str(), xs, y.c_str(), ys);
//	d = LongestCommonSubsequenceLength(x.c_str(), xs, y.c_str(), ys);
	s = 1.0 - (double)d/(double)m_max;
//	s_min = (double)d/(double)m_min;
//	s_max = (double)d/(double)m_max;
//	if (xs >= 5 && ys >= 5 && s_min >= 0.85 && s_max >= 0.65) {
	if (s >= 0.85) {
		return true;
	}
	return false;
/*
	xi = xs;
	yi = ys;
	while (xi > 0 && yi > 0 && x[xi - 1] == y[yi - 1]) {
		xi--;
		yi--;
	}
	if (xs >= 5 && ys >= 5) {
		return xi == 0 || yi == 0;
	} else {
		return xi == 0 && yi == 0;
	}
*/
}

std::map<std::string, std::set<std::string> >::const_iterator
findFilenameFromPackage(const std::string &filename, const std::map<std::string, std::set<std::string> > & package, float &weight)
{
	std::map<std::string, std::set<std::string> >::const_iterator pIter;

	if (!approxFilename) {
		pIter = package.find(filename);
		weight = idf[filename];
		return pIter;
	} else {
		for (	pIter  = package.begin();
			pIter != package.end();
			pIter++)
		{
			if (approxMatchFilenames(filename, pIter->first)) {
//				weight = min((double)idf[filename], (double)idf[pIter->first]);
				weight = (double)idf[pIter->first];
				return pIter;
			}
		}
	}
	return package.end();
}

bool
IsProgramFilename(const std::string &feature)
{
	std::string ext;
	size_t n;

	n = feature.find_last_of('.');
	if (n == 0 || n == std::string::npos)
		return false;
	ext = feature.substr(n + 1);
	for (size_t i = 0; i < ext.size(); i++) {
		ext[i] = tolower(ext[i]);
	}
	return extensions.find(ext) != extensions.end();
}

void
printMatch(const std::map<std::string, std::set<std::string> > &embedding, const std::map<std::string, std::set<std::string> > &package, std::map<std::string, std::pair<std::string, float> > &matches)
{
	std::map<std::string, std::pair<std::string, float> >::iterator matchesIter;
	if (verbose >= 2) {
		for (	matchesIter  = matches.begin();
			matchesIter != matches.end();
			matchesIter++)
		{
			if (IsProgramFilename(matchesIter->first) || IsProgramFilename(matchesIter->second.first)) {
				if (outputFormat == CLONEWISE_OUTPUT_XML) {
					fprintf(outFd, "\t\t<Match>\n");
					fprintf(outFd, "\t\t\t<Filename1>%s</Filename1>\n", matchesIter->first.c_str());
					fprintf(outFd, "\t\t\t<Filename2>%s</Filename2>\n", matchesIter->second.first.c_str());
					fprintf(outFd, "\t\t\t<Weight>%f</Weight>\n", matchesIter->second.second);
					fprintf(outFd, "\t\t</Match>\n");
				} else {
					fprintf(outFd, "\t\tMATCH %s/%s (%f)\n", matchesIter->first.c_str(), matchesIter->second.first.c_str(), matchesIter->second.second);
				}
			}
		}
	}
}

unsigned int
numberOfSources(const std::map<std::string, std::set<std::string> > &embedding)
{
	std::map<std::string, std::set<std::string> >::const_iterator eIter;
	unsigned int count;

	count = 0;
	for (	eIter  = embedding.begin();
		eIter != embedding.end();
		eIter++)
	{

		if (IsProgramFilename(eIter->first)) {
			count++;
		}
	}
	return count;
}

template <class T>
void
SolveGreedy(Matrix<T> &matrix)
{
	std::set<int> done;
	int min_j;
	T min;

	for (int i = 0; i < matrix.rows(); i++) {
		min_j = -1;
		for (int j = 0; j < matrix.columns(); j++) {
			if ((min_j == -1 || matrix(i, j) < min) && done.find(i) == done.end()) {
				min_j = j;
				min = matrix(i, j);
			}
			matrix(i, j) = -1;
		}
		if (min_j != -1) {
			done.insert(min_j);
			matrix(i, min_j) = 0;
		}
	}
}

bool
WriteCheckForClone(const std::string &name, std::ofstream &testStream, const std::map<std::string, std::set<std::string> > &embedding, const std::map<std::string, std::set<std::string> > &package, const std::string &cl, std::map<std::string, std::pair<std::string, float> > &matches)
{
	std::map<std::string, std::set<std::string> >::const_iterator eIter;
	int found, foundSimilar, foundFilenameHash, foundFilenameHash80, foundExactFilenameHash;
	int foundData, foundDataSimilar, foundDataFilenameHash, foundDataFilenameHash80, foundDataExactFilenameHash;
	int foundHash, foundExactHash, foundDataExactHash;
	float scoreFilename, scoreSimilar, scoreFilenameHash, scoreFilenameHash80, scoreExactFilenameHash;
	float scoreDataFilename, scoreDataSimilar, scoreDataFilenameHash, scoreDataFilenameHash80, scoreDataExactFilenameHash;
	float featureVector[NFEATURES];
	std::map<std::string, std::map<std::string, float> > possibleMatches;
	std::map<std::string, std::map<std::string, float> >::const_iterator possibleMatchesIter;
	std::set<std::string> possibleMatches2;

	foundDataExactHash = 0;
	foundExactHash = 0;
	foundHash = 0;

	found = 0;
	foundSimilar = 0;
	foundFilenameHash80 = 0;
	foundFilenameHash = 0;
	foundExactFilenameHash = 0;
	scoreFilenameHash = 0.0;
	scoreFilenameHash80 = 0.0;
	scoreFilename = 0.0;
	scoreSimilar = 0.0;
	scoreExactFilenameHash = 0.0;

	foundData = 0;
	foundDataSimilar = 0;
	foundDataFilenameHash80 = 0;
	foundDataFilenameHash = 0;
	foundDataExactFilenameHash = 0;
	scoreDataFilenameHash = 0.0;
	scoreDataFilenameHash80 = 0.0;
	scoreDataFilename = 0.0;
	scoreDataSimilar = 0.0;
	scoreDataExactFilenameHash = 0.0;

	for (	eIter  = embedding.begin();
		eIter != embedding.end();
		eIter++)
	{

		std::map<std::string, std::set<std::string> >::const_iterator pIter;
		float maxS;
		std::set<std::string>::const_iterator l1Iter;
		float weight;
		bool isData;

		isData = !IsProgramFilename(eIter->first);

#if 0
		for (	l1Iter  = eIter->second.begin();
			l1Iter != eIter->second.end();
			l1Iter++)
		{
			maxS = 0.0;
			for (	pIter  = package.begin();
				pIter != package.end();
				pIter++)
			{
				std::set<std::string>::const_iterator l2Iter;

				for (	l2Iter  = pIter->second.begin();
					l2Iter != pIter->second.end();
					l2Iter++)
				{
					float s;

					s = fuzzy_compare(l1Iter->c_str(), l2Iter->c_str());
					if (s > maxS) {
						maxS = s;
						goto skip1;
					}
				}
			}
skip1:
			if (maxS > 0.0) {
				foundHash++;
			}
		}
#endif

		for (	l1Iter  = eIter->second.begin();
			l1Iter != eIter->second.end();
			l1Iter++)
		{
			for (	pIter  = package.begin();
				pIter != package.end();
				pIter++)
			{
				if (pIter->second.find(*l1Iter) != pIter->second.end()) {
					if (isData) {
						foundDataExactHash++;
					} else {
						foundExactHash++;
					}
					goto skip3;
				}
			}
		}
skip3:
		pIter = findFilenameFromPackage(eIter->first, package, weight);
		if (pIter != package.end()) {
			possibleMatches[eIter->first][pIter->first] = weight;
			possibleMatches2.insert(pIter->first);
			maxS = 0.00;
			for (	l1Iter  = eIter->second.begin();
				l1Iter != eIter->second.end();
				l1Iter++)
			{
				std::set<std::string>::const_iterator l2Iter;

				if (pIter->second.find(*l1Iter) != pIter->second.end()) {
					maxS = 100.0;
					if (isData) {
						foundDataExactFilenameHash++;
						scoreDataExactFilenameHash += weight;
					} else {
						foundExactFilenameHash++;
						scoreExactFilenameHash += weight;
					}
					goto skip2;
				}
				for (	l2Iter  = pIter->second.begin();
					l2Iter != pIter->second.end();
					l2Iter++)
				{
					float s;

					s = fuzzy_compare(l1Iter->c_str(), l2Iter->c_str());
					if (s > maxS) {
						maxS = s;
					}
				}
			}
skip2:
			if (maxS > 0.0) {
				if (isData) {
					foundDataFilenameHash++;
					scoreDataFilenameHash += weight;
				} else {
					foundFilenameHash++;
					scoreFilenameHash += weight;
				}
			}
			if (maxS > 80.0) {
				if (isData) {
					foundDataFilenameHash80++;
					scoreDataFilenameHash80 += weight;
				} else {
					foundFilenameHash80++;
					scoreFilenameHash80 += weight;
				}
			}
		}
	}

	Matrix<double> matrix(possibleMatches.size(), possibleMatches2.size());
	int i;

	i = 0;
	for (	possibleMatchesIter  = possibleMatches.begin();
		possibleMatchesIter != possibleMatches.end();
		possibleMatchesIter++, i++)
	{
		std::set<std::string>::const_iterator possibleMatchesIter2;
		int j;

		j = 0;
		for (	possibleMatchesIter2  = possibleMatches2.begin();
			possibleMatchesIter2 != possibleMatches2.end();
			possibleMatchesIter2++, j++)
		{
			matrix(i, j) = maxWeight - possibleMatches[possibleMatchesIter->first][*possibleMatchesIter2];
		}
	}
	if (matrix.rows() > 500 || matrix.columns() > 500) {
		SolveGreedy(matrix);
	} else {
		Munkres m;
		m.solve(matrix);
	}

	i = 0;
	for (	possibleMatchesIter  = possibleMatches.begin();
		possibleMatchesIter != possibleMatches.end();
		possibleMatchesIter++, i++)
	{
		std::set<std::string>::const_iterator possibleMatchesIter2;
		int j;

		j = 0;
		for (	possibleMatchesIter2  = possibleMatches2.begin();
			possibleMatchesIter2 != possibleMatches2.end();
			possibleMatchesIter2++, j++)
		{
			if (matrix(i, j) == 0) {
				bool isData;
				float weight;

				weight = possibleMatches[possibleMatchesIter->first][*possibleMatchesIter2];
				isData = !IsProgramFilename(possibleMatchesIter->first);
				if (isData) {
					if (possibleMatchesIter->first == *possibleMatchesIter2) {
						foundData++;
						scoreDataFilename += weight;
					} else {
						foundDataSimilar++;
						scoreDataSimilar += weight;
					}
				} else {
					if (possibleMatchesIter->first == *possibleMatchesIter2) {
						found++;
						scoreFilename += weight;
					} else {
						foundSimilar++;
						scoreSimilar += weight;
					}
				}
				matches[possibleMatchesIter->first] = std::pair<std::string, float>(*possibleMatchesIter2, weight);	
				break;
			}
		}
	}
	featureVector[ 0] = embedding.size();
	featureVector[ 1] = numberOfSources(embedding);
	featureVector[ 2] = package.size();
	featureVector[ 3] = numberOfSources(package);

	featureVector[ 4] = found;
	featureVector[ 5] = foundSimilar;
	featureVector[ 6] = foundFilenameHash;
	featureVector[ 7] = foundFilenameHash80;
	featureVector[ 8] = foundExactFilenameHash;
	featureVector[ 9] = scoreFilename;
	featureVector[10] = scoreSimilar;
	featureVector[11] = scoreFilenameHash;
	featureVector[12] = scoreFilenameHash80;
	featureVector[13] = scoreExactFilenameHash;

	featureVector[14] = foundData;
	featureVector[15] = foundDataSimilar;
	featureVector[16] = foundDataFilenameHash;
	featureVector[17] = foundDataFilenameHash80;
	featureVector[18] = foundDataExactFilenameHash;
	featureVector[19] = scoreDataFilename;
	featureVector[20] = scoreDataSimilar;
	featureVector[21] = scoreDataFilenameHash;
	featureVector[22] = scoreDataFilenameHash80;
	featureVector[23] = scoreDataExactFilenameHash;

	featureVector[24] = foundHash;
	featureVector[25] = foundExactHash;
	featureVector[26] = foundDataExactHash;

#pragma omp critical
	{
		if (verbose >= 3) {
			char fs[1024];

			snprintf(fs, sizeof(fs), "%f", featureVector[0]);
			for (int i = 1; i < NFEATURES; i++) {
				char ts[1024];

				snprintf(ts, sizeof(ts), ",%f", featureVector[i]);
				strcat(fs, ts);
			}
			if (outputFormat == CLONEWISE_OUTPUT_XML) {
				fprintf(outFd, "\t<Comparison>\n");
				fprintf(outFd, "\t\t<Names>%s</Names>\n", name.c_str());
				fprintf(outFd, "\t\t<FeatureVector>%s</FeatureVector>\n", fs);
				fprintf(outFd, "\t</Comparison>\n");
			} else {
				fprintf(outFd, "# Comparison %s : %s\n", name.c_str(), fs);
			}
		}
	}


#pragma omp critical
	{
		for (int i = 0; i < NFEATURES; i++) {
			if (!UseFeatureSelection || Features[i].Use) {
				testStream << featureVector[i] << ",";
			}
		}
		testStream << cl << "\n";
	}
//printMatch(embedding, package, matches);

	if (featureVector[4] == 0)
		return false;

	return true;
}

void
LoadSignature(std::string name, std::map<std::string, std::set<std::string> > &signature)
{
	std::ifstream stream;

	stream.open(name.c_str());
	if (!stream) {
		errorLog("Couldn't open %s\n", name.c_str());
	}
	while (!stream.eof()) {
		std::string str, str2, hash, feature;
		char s[1024];

		stream.getline(s, sizeof(s));
		if (s[0] == 0)
			break;
		lineToFeature(s, feature, hash);
		if (featureExceptions.find(feature) == featureExceptions.end()) {
			signature[feature].insert(hash);
		}
	}
	stream.close();
}

void
printArffHeader(std::ofstream &testStream)
{
	testStream << "@RELATION Clones\n";

	for (int i = 0; i < NFEATURES; i++) {
		if (!UseFeatureSelection || Features[i].Use) {
			testStream << "@Attribute " << Features[i].Name << " NUMERIC\n";
		}
	}

	testStream << "@ATTRIBUTE CLASS {Y,N}\n";
	testStream << "@DATA\n";
}

int
GetScoreForNotEmbedded(std::ofstream &testStream)
{
        std::map<std::string, std::list<std::string> >::const_iterator pIter;
        std::string p1, p2;
        int n1, n2, i;
        bool breakit, doit;
	int skip;
	std::map<std::string, std::pair<std::string, float> > matches;

	skip = 0;
	do {
		std::string n;

		doit = false;
		do {
        	        n1 = rand() % packages.size();
			n2 = rand() % packages.size();
			breakit = true;
			for (   pIter  = packages.begin(), i = 0;
                        	pIter != packages.end() && i < n1;
                        	pIter++, i++);
			p1 = pIter->first;
			for (   pIter  = packages.begin(), i = 0;
				pIter != packages.end() && i < n2;
				pIter++, i++);
			p2 = pIter->first;
			if (embeddedList.find(p1) != embeddedList.end() && embeddedList[p1].find(p2) != embeddedList[p1].end())
				breakit = false;
			else if (embeddedList.find(p2) != embeddedList.end() && embeddedList[p2].find(p1) != embeddedList[p2].end())
				breakit = false;
		} while (!breakit);
		n = p1 + std::string("/") + p2;
		if (!WriteCheckForClone(n, testStream, packagesSignatures[p1], packagesSignatures[p2], "N", matches)) {
			doit = true;
			skip++;
		}
	} while (doit);
	return 1 + skip;
}

int
DoScoresForEmbedded(std::ofstream &testStream)
{
	int total;
	int fp;

	fp = 0;
	total = 0;
#pragma omp parallel
#pragma omp single
	{
		std::map<std::string, std::set<std::string> >::const_iterator iter1;

        	for (   iter1  = embeddedList.begin();
        	        iter1 != embeddedList.end();
        	        iter1++)
        	{
			std::set<std::string>::const_iterator iter2;
			std::map<std::string, std::set<std::string> > *sig1, *sig2;

			sig1 = &packagesSignatures[iter1->first];
			for (   iter2  = iter1->second.begin();
				iter2 != iter1->second.end();
				iter2++, total++)
			{
				std::map<std::string, std::pair<std::string, float> > matches;
				std::string n;

				n = iter1->first + std::string("/") + *iter2;
				sig2 = &packagesSignatures[*iter2];
#pragma omp task
				{
					if (!WriteCheckForClone(n, testStream, *sig1, *sig2, "Y", matches)) {
#pragma omp atomic
						fp++;
					}
				}
			}
		}
#pragma omp taskwait
	}
	return total - fp;
}

void
trainModel()
{
	std::ofstream testStream;
	char cmd[1024], s[1024], testFilename[L_tmpnam + 128];
	std::string rmString;
	int c, total;

	srand(0);
	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/embedded-code-copies", distroString);
	LoadEmbeddedCodeCopiesList(s);

	snprintf(testFilename, sizeof(testFilename), "/var/lib/Clonewise/clones/weka/training.arff");

	testStream.open(testFilename);
	if (!testStream) {
		fprintf(stderr, "Can't write test.arff\n");
		return;
	}
	printArffHeader(testStream);
	c = DoScoresForEmbedded(testStream);
	total = 0;
	c = 0;
#pragma omp parallel for
	for (int i = 0; i < 4000; i++) {
		int t;

		t = GetScoreForNotEmbedded(testStream);
#pragma omp atomic
		total += t;
#pragma omp atomic
		c++;
	}
	testStream.close();

#if 0
	// feature selection on training set
	snprintf(cmd, sizeof(cmd), "java -cp /usr/share/java/weka.jar weka.attributeSelection.WrapperSubsetEval -B weka.classifiers.trees.RandomForest -F 5 -T 0.01 -R 1 -- -I 10 -K 0 -S 1 -i /var/lib/Clonewise/clones/weka/training.arff");
#endif

	// generate new training set based on feature selection
	rmString = "3,4,7,9,11,17,19,21,23,24,25";
//	snprintf(cmd, sizeof(cmd), "java -cp /usr/share/java/weka.jar weka.filters.unsupervised.attribute.Remove -R %s -i /var/lib/Clonewise/clones/weka/training.arff -o /var/lib/Clonewise/clones/weka/training_featureselection.arff", rmString.c_str());
	snprintf(cmd, sizeof(cmd), "cp /var/lib/Clonewise/clones/weka/training.arff /var/lib/Clonewise/clones/weka/training_featureselection.arff");
	system(cmd);

	// train model
	snprintf(cmd, sizeof(cmd), "java -cp /usr/share/java/weka.jar weka.classifiers.trees.RandomForest -I 10 -K 0 -S 1 -d /var/lib/Clonewise/clones/weka/model -t /var/lib/Clonewise/clones/weka/training_featureselection.arff");
	system(cmd);
}

static void
checkPackage(std::map<std::string, std::set<std::string > > &embedding, const char *name)
{
	std::map<std::string, std::map<std::string, std::pair<std::string, float> > > matchesTable;
	std::map<std::string, std::list<std::string> >::const_iterator pIter;
	char cmd[1024], testFilename[L_tmpnam + 128], testFilename2[L_tmpnam + 128], t[L_tmpnam], str[1024];
	FILE *p;
	std::ofstream testStream;
	std::set<std::string> skip;
	std::string rmString;

	tmpnam(t);
	snprintf(testFilename, sizeof(testFilename), "%s.arff", t);
	tmpnam(t);
	snprintf(testFilename2, sizeof(testFilename2), "%s.arff", t);

	testStream.open(testFilename);
	if (!testStream) {
		fprintf(stderr, "Can't write test.arff\n");
		return;
	}
	printArffHeader(testStream);

#pragma parallel
#pragma single
	{
		for (	pIter  = packages.begin();
			pIter != packages.end();
			pIter++)
		{
#pragma task
			{
				const std::map<std::string, std::set<std::string> > *package;
				std::string fp;

				fp = std::string(name) + std::string("/") + pIter->first;
				if (ignoreFalsePositives.find(fp) != ignoreFalsePositives.end())
					continue;

				package = &packagesSignatures[pIter->first];
				if (strcmp(name, pIter->first.c_str()) != 0) {
					if (!WriteCheckForClone(fp, testStream, embedding, *package, "?", matchesTable[pIter->first]))
#pragma omp critical (skipMod)
						{
//							skip.insert(fp);
						}
				} else {
#pragma omp critical (skipMod)
					{
						skip.insert(fp);
					}
				}
			}
		}
#pragma omp taskwait
	}
	testStream.close();

	// generate new test set based on feature selection
	rmString = "3,4,7,9,11,17,19,21,23,24,25";
//	snprintf(cmd, sizeof(cmd), "java -cp /usr/share/java/weka.jar weka.filters.unsupervised.attribute.Remove -R %s -i %s -o %s", rmString.c_str(), testFilename, testFilename2);
	snprintf(cmd, sizeof(cmd), "cp %s %s", testFilename, testFilename2);

	system(cmd);
	snprintf(cmd, sizeof(cmd), "java -cp /usr/share/java/weka.jar weka.classifiers.trees.RandomForest -l /var/lib/Clonewise/clones/weka/model -T %s -p 0", testFilename2);
	p = popen(cmd, "r");
	if (p == NULL) {
		unlink(testFilename);
		fprintf(stderr, "Can't popen\n");
		return;
	}
	for (int i = 0; i < 5; i++) {
		fgets(str, sizeof(str), p);
	}
	for (
		pIter = packages.begin();
		!feof(p) && pIter != packages.end();
		pIter++)
	{
		const std::map<std::string, std::set<std::string> > *package;
		std::string fp;

		fp = std::string(name) + std::string("/") + pIter->first;
		if (ignoreFalsePositives.find(fp) != ignoreFalsePositives.end())
			continue;

		if (skip.find(fp) != skip.end())
			continue;

		package = &packagesSignatures[pIter->first];

		str[27] = 0;
		fgets(str, sizeof(str), p);
		if (str[27] == 'Y') {
			if (str[35] == '1' || (str[37] - '0') >= 8) {
				if (outputFormat == CLONEWISE_OUTPUT_XML) {
					fprintf(outFd, "\t<Clone>\n");
					fprintf(outFd, "\t\t<SourcePackage>%s</SourcePackage>\n", name);
					fprintf(outFd, "\t\t<ClonedSourcePackage>%s</ClonedSourcePackage>\n", pIter->first.c_str());
				} else {
					fprintf(outFd, "%s CLONED_IN_SOURCE %s\n", name, pIter->first.c_str());		
				}
				printMatch(embedding, *package, matchesTable[pIter->first]);
				if (verbose >= 1) {
					std::list<std::string>::const_iterator nIter;

					for (	nIter  = pIter->second.begin();
						nIter != pIter->second.end();
						nIter++)
					{
						if (outputFormat == CLONEWISE_OUTPUT_XML) {
							fprintf(outFd, "\t\t<ClonedPackage>%s</ClonedPackage>\n", nIter->c_str());
						} else {
							fprintf(outFd, "\t%s CLONED_IN_PACKAGE %s\n", name, nIter->c_str());
						}
					}
				}
			}
			if (outputFormat == CLONEWISE_OUTPUT_XML) {
				fprintf(outFd, "\t</Clone>\n");
			}
		}
	}
	pclose(p);
//	unlink(testFilename);
//	unlink(testFilename2);
}

void
ClonewiseInit()
{
	static char dString[1024];
	std::ifstream stream;
	char s[1024];

	snprintf(s, sizeof(s), "/var/lib/Clonewise/default-distribution");
	stream.open(s);
	if (!stream) {
		fprintf(stderr, "Couldn't open %s\n", s);
		exit(1);
	}
	stream.getline(dString, sizeof(dString));
	distroString = dString;
	stream.close();
}

void
ClonewiseCleanup()
{
}

void
LoadPackagesInfo()
{
	std::ifstream stream;
	char s[1024];

	if (numPackages != 0)
		return;

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/packages", distroString);
	stream.open(s);
	if (!stream) {
		fprintf(stderr, "Couldn't open packages\n");
		exit(1);
	}
	while (!stream.eof()) {
		std::string str, sigName, srcName;
		size_t n;
		char s[1024];

		stream.getline(s, sizeof(s));
		if (s[0] == 0)
			break;
		str = std::string(s);
		n = str.find_last_of('/');
		if (n == std::string::npos)
			return;
		sigName = str.substr(0, n);
		srcName = str.substr(n + 1);
		if (sigName.size() > 0 && srcName.size() > 0) {
			packages[srcName].push_back(sigName);
		}
	}
	stream.close();
	numPackages = packages.size();
}

static void
ReadIgnoreList(const char *filename)
{
	std::ifstream stream;

	stream.open(filename);
	if (stream) {
		while (!stream.eof()) {
			char s[1024];

			stream.getline(s, sizeof(s));
			if (s[0] == 0)
				break;
			ignoreFalsePositives.insert(s);
		}
		stream.close();
	}
}

void
LoadExtensions()
{
	std::ifstream stream;
	char s[1024];

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/extensions", distroString);
	stream.open(s);
	if (!stream) {
		fprintf(stderr, "Couldn't open %s\n", s);
		exit(1);
	}
	while (!stream.eof()) {
		std::string ext;

		stream.getline(s, sizeof(s));
		if (s[0] == 0)
			break;
		ext = std::string(s);
		for (size_t i = 0; i < ext.size(); i++) {
			ext[i] = tolower(ext[i]);
		}
		extensions.insert(ext);
	}
	stream.close();
}

void
Clonewise_build_database(int argc, char *argv[])
{
	ClonewiseInit();
	LoadEverything();
}

int
LoadEverything()
{
	std::string filename;
	std::ifstream stream;
	char s[1024];
	std::map<std::string, std::list<std::string> >::const_iterator pIter;
	std::map<std::string, float>::iterator idfIter;

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/.done", distroString);
	if (access(s, R_OK) != 0)
		BuildDatabase();

	LoadExtensions();
	LoadPackagesInfo();
	loadFeatureExceptions();

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/features/features", distroString);
	stream.open(s);
	if (!stream) {
		BuildFeatures();
		snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/features/features", distroString);
		stream.open(s);
		if (!stream) {
			fprintf(stderr, "Can't open features\n");
			exit(1);
		}
	}
	while (!stream.eof()) {
		std::string str, fname, freqStr, normalFeature;
		int freq;
		size_t n1, n2;

		stream.getline(s, sizeof(s));
		if (s[0] == 0)
			break;
		str = std::string(s);
		n1 = str.find_first_of('/');
		n2 = str.find_last_of('/');
		if (n1 == std::string::npos || n2 == std::string::npos)
			goto err;
		fname = str.substr(0, n1);
		normalizeFeature(normalFeature, fname);
		normalFeatures[fname] = normalFeature;
		freqStr = str.substr(n2 + 1);
		freq = strtol(freqStr.c_str(), NULL, 10);
		if (freq >= 1) {
			idf[normalFeature] += freq;
		}
	}
	stream.close();
	for (	idfIter  = idf.begin();
		idfIter != idf.end();
		idfIter++)
	{
		idfIter->second = log(numPackages/idfIter->second);
		if (idfIter->second > maxWeight) {
			maxWeight = idfIter->second;
		}
	}
	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/features/ignore-list-for-features", distroString);
	stream.open(s);
	if (!stream) {
		fprintf(stderr, "Couldn't open ignore-these-features\n");
		exit(1);
	}
	while (!stream.eof()) {
		stream.getline(s, sizeof(s));
		if (s[0] == 0)
			break;
		featureSet.insert(s);
	}
	stream.close();
	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/features/ignore-these-false-positives", distroString);
	ReadIgnoreList(s);
	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/features/ignore-these-fixed", distroString);
	ReadIgnoreList(s);
	for (	pIter  = packages.begin();
		pIter != packages.end();
		pIter++)
	{
		filename = std::string("/var/lib/Clonewise/clones/distros/") + distroString + std::string("/signatures/") + pIter->first;
		LoadSignature(filename, packagesSignatures[pIter->first]);
	}

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/weka/model");
	if (access(s, R_OK) != 0) {
		if (getuid() != 0) {
			fprintf(stderr, "Need to be root to build database.\n");
			exit(1);
		}
		trainModel();
	}

	return 0;
err:
	printf("error\n");
	return 1;
}

int
RunClonewise(int argc, char *argv[])
{
	std::string filename;
	std::map<std::string, std::set<std::string> > embedding;
	std::map<std::string, std::list<std::string> >::const_iterator pIter;

	if (outputFormat == CLONEWISE_OUTPUT_XML) {
		fprintf(outFd, "<Clonewise>\n");
	}
	if (allPackages == false) {
		for (int i = 0; i < argc; i++) {
			if (useRelativePathForSignature) {
				embedding = packagesSignatures[argv[i]];
			} else {
				filename = std::string(argv[i]);
				LoadSignature(filename, embedding);
			}
			checkPackage(embedding, argv[i]);
		}
	} else {
		for (	pIter  = packages.begin();
			pIter != packages.end();
			pIter++)
		{
			checkPackage(packagesSignatures[pIter->first], pIter->first.c_str());
		}
	}
	if (outputFormat == CLONEWISE_OUTPUT_XML) {
		fprintf(outFd, "</Clonewise>\n");
	}
	return 0;
}

static void
CreateFeatures()
{
	std::ofstream rawFeaturesFile, featuresFile, fileCountFile;
	std::ifstream stream;
	char s[1024];
	std::set<std::string> featuresKey;
	std::map<std::string, int> features, featuresDoc;
	std::map<std::string, int>::iterator iter;
	std::multimap<int, std::string> iFeatures;
	std::multimap<int, std::string>::reverse_iterator iIter;
	std::map<std::string, std::list<std::string> >::const_iterator pIter;

	LoadPackagesInfo();

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/features/raw-features", distroString);
	rawFeaturesFile.open(s);
	if (!rawFeaturesFile) {
		fprintf(stderr, "Couldn't open raw-features\n");
		exit(1);
	}
	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/features/files-count", distroString);
	fileCountFile.open(s);
	if (!rawFeaturesFile) {
		fprintf(stderr, "Couldn't open files-count\n");
		exit(1);
	}
	for (	pIter  = packages.begin();
		pIter != packages.end();
		pIter++)
	{
		std::string filename;
		int i;

		filename = std::string("/var/lib/Clonewise/clones/distros/") + distroString + std::string("/signatures/") + pIter->first;
		stream.open(filename.c_str());
		if (!stream) {
			errorLog("Couldn't open %s\n", filename.c_str());
		}
		i = 0;
		while (!stream.eof()) {
			std::string str, str2, hash, feature;
			char s[1024];

			stream.getline(s, sizeof(s));
			if (s[0] == 0)
				break;
			lineToFeature(s, feature, hash);
			rawFeaturesFile << feature.c_str() << "\n";
			i++;
		}
		rawFeaturesFile << "/\n";
		fileCountFile << i << "\n";
		stream.close();
	}
	rawFeaturesFile.close();
	fileCountFile.close();

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/features/features", distroString);
	featuresFile.open(s);
	if (!featuresFile) {
		fprintf(stderr, "Couldn't open features\n");
		exit(1);
	}

	snprintf(s, sizeof(s), "/var/lib/Clonewise/clones/distros/%s/features/raw-features", distroString);
	stream.open(s);
	if (!stream) {
		fprintf(stderr, "Couldn't open raw-features\n");
		exit(1);
	}
	while (!stream.eof()) {
		stream.getline(s, sizeof(s));
		if (s[0] == 0)
			break;
		if (s[0] == '/') {
			std::set<std::string>::iterator iter2;

			for (	iter2  = featuresKey.begin();
				iter2 != featuresKey.end();
				iter2++)
			{
				featuresDoc[*iter2]++;
			}
			featuresKey = std::set<std::string>();
		} else {
			featuresKey.insert(s);
			features[s]++;
		}
	}
	for (	iter  = features.begin();
		iter != features.end();
		iter++)
	{
		iFeatures.insert(std::pair<int, std::string>(iter->second, iter->first));
	}
	for (	iIter  = iFeatures.rbegin();
		iIter != iFeatures.rend();
		iIter++)
	{
		featuresFile << iIter->second.c_str() << "/" << iIter->first << "/" << featuresDoc[iIter->second] << "\n";
	}
	featuresFile.close();
}
